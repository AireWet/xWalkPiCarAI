#!/usr/bin/env python3
"""Run the installable xWalk historical GitHub-to-Jira importer."""

from __future__ import annotations

import sys
from datetime import date, datetime
from typing import Sequence

from .xWalkJiraImportCommitAnalyser import analyse_commit
from .xWalkJiraImportConfig import ConfigurationError, ImportConfig, load_config
from .xWalkJiraImportCredentials import mask_email
from .xWalkJiraImportDateCalculator import (
    calculate_historical_dates,
    calculate_planned_due_date,
    completion_timestamp,
    next_workday,
)
from .xWalkJiraImportEstimator import estimate_effort
from .xWalkJiraImportGitHubClient import ApiError, GitHubClient
from .xWalkJiraImportJiraClient import JiraClient, build_issue_payload
from .xWalkJiraImportModels import ImportRecord, ImportSummary, JiraMetadata
from .xWalkJiraImportReport import write_reports


def _record_for_commit(
    commit_sha: str,
    completion: datetime | None,
    component: str,
    summary: str,
    issue_type: str,
    estimated_time: str,
    story_points: int,
    start: datetime | None,
    confidence: str,
) -> ImportRecord:
    """Create one report row with normalized ISO timestamps."""
    return ImportRecord(
        commit_sha=commit_sha,
        commit_date=completion.isoformat() if completion is not None else "",
        component=component,
        generated_summary=summary,
        issue_type=issue_type,
        estimated_time=estimated_time,
        story_points=story_points,
        historical_start_date=start.isoformat() if start is not None else "",
        historical_completion_date=completion.isoformat() if completion is not None else "",
        confidence=confidence,
    )


def _discover_jira(
    config: ImportConfig,
    summary: ImportSummary,
    supplied_client: JiraClient | None,
) -> tuple[JiraClient | None, JiraMetadata | None, str | None]:
    """Perform read-only Jira discovery when credentials are available."""
    if supplied_client is None and not config.has_jira_credentials:
        summary.limitations.append(
            "Jira credentials are unavailable; metadata, duplicate, workflow, and board discovery were skipped."
        )
        return None, None, None
    client = supplied_client or JiraClient(
        config.jira_url,
        config.jira_email or "",
        config.jira_api_token or "",
        config.jira_project_key,
        config.jira_board_id,
        allow_writes=config.apply,
    )
    metadata = client.discover_metadata()
    relevant_names = {
        "Historical created date",
        "Historical done date",
        "Original estimate",
        "Start date",
        "Story point estimate",
        "Story points",
        "Time tracking",
    }
    summary.fields_discovered = sorted(
        field.name
        for field in metadata.all_fields.values()
        if field.name in relevant_names or field.field_id == "timetracking"
    )
    board_jql: str | None = None
    try:
        board_jql = client.get_board_filter_jql()
    except ApiError as error:
        summary.limitations.append(str(error))
    return client, metadata, board_jql


def run_import(
    config: ImportConfig,
    github_client: GitHubClient | None = None,
    jira_client: JiraClient | None = None,
) -> tuple[list[ImportRecord], ImportSummary]:
    """Execute one import with all mutations guarded by config.apply."""
    github = github_client or GitHubClient(config.github_repository, config.github_token)
    commits = github.collect_commits(
        config.github_branch,
        since=config.since,
        until=config.until,
        author=config.author,
        max_commits=config.max_commits,
        include_merges=config.include_merges,
    )
    summary = ImportSummary(
        commits_inspected=len(commits),
        limitations=list(config.credential_warnings),
    )
    try:
        jira, metadata, board_jql = _discover_jira(config, summary, jira_client)
    except ApiError as error:
        if config.apply:
            raise
        summary.limitations.append(f"Jira read-only discovery failed: {error}")
        jira, metadata, board_jql = None, None, None
    if config.apply and (jira is None or metadata is None):
        raise ConfigurationError("--apply requires successful Jira metadata discovery.")
    if config.apply and board_jql is None:
        raise ConfigurationError("--apply requires readable board-filter metadata before issue creation.")

    analysed = [(commit, analyse_commit(commit, config.include_insignificant)) for commit in commits]
    records: list[ImportRecord] = []
    previous_relevant: datetime | None = None
    planned_start: date | None = None
    start_field_id = ""
    if config.update_existing_planning_after is not None:
        anchor_due = jira.get_due_date(config.update_existing_planning_after)
        planned_start = next_workday(anchor_due)
    elif config.update_existing_planning_start is not None:
        planned_start = config.update_existing_planning_start
    if planned_start is not None:
        start_fields = [
            field.field_id
            for field in metadata.all_fields.values()
            if field.name.casefold() == "start date"
        ]
        if not start_fields:
            raise ConfigurationError("Sequential planning requires a Jira Start date field.")
        start_field_id = sorted(start_fields)[0]
    if config.update_existing_epic is not None:
        jira.require_epic(config.update_existing_epic)

    for commit, analysis in analysed:
        try:
            completion = completion_timestamp(commit)
        except ValueError as error:
            summary.failures += 1
            row = _record_for_commit(commit.sha, None, "Other", commit.title, "Task", "", 0, None, "Low")
            row.result = "failed"
            row.error_details = str(error)
            records.append(row)
            continue
        if not analysis.accepted:
            summary.commits_ignored += 1
            ignored_summary = f"[Ignored] {commit.title}" if commit.title else "[Ignored] Empty commit message"
            row = _record_for_commit(
                commit.sha,
                completion,
                "Other",
                ignored_summary,
                "Task",
                "",
                0,
                None,
                "Low",
            )
            row.result = "skipped"
            row.error_details = analysis.reason
            records.append(row)
            continue

        summary.commits_accepted += 1
        estimate = estimate_effort(commit, analysis)
        dates = calculate_historical_dates(commit, estimate, previous_relevant)
        previous_relevant = completion
        planned_due: date | None = None
        current_planned_start = planned_start
        if current_planned_start is not None:
            if estimate.effort_minutes is None:
                raise ConfigurationError("Sequential planning requires an effort estimate for every commit.")
            planned_due = calculate_planned_due_date(current_planned_start, estimate.effort_minutes)
            planned_start = next_workday(planned_due)
        issue_type_name = analysis.issue_type
        issue_type_id = ""
        create_fields = {}
        if metadata is not None:
            issue_type_name, issue_type_id = metadata.choose_issue_type(analysis.issue_type)
            create_fields = metadata.fields_by_issue_type.get(issue_type_id, {})

        row = _record_for_commit(
            commit.sha,
            dates.completion,
            analysis.component,
            analysis.summary,
            issue_type_name,
            estimate.display_time,
            estimate.story_points,
            dates.start,
            dates.confidence,
        )
        records.append(row)
        if current_planned_start is not None and planned_due is not None:
            row.planned_start_date = current_planned_start.isoformat()
            row.planned_due_date = planned_due.isoformat()

        requires_manual_review = estimate.manual_review or analysis.manual_review
        if requires_manual_review:
            summary.manual_review += 1
            if not (config.apply and config.apply_manual_review):
                row.result = "manual review"
                row.error_details = estimate.rationale if estimate.manual_review else analysis.reason
                continue

        if jira is not None:
            try:
                existing = jira.find_existing_import(commit.sha)
            except ApiError as error:
                summary.failures += 1
                row.result = "failed"
                row.error_details = str(error)
                continue
            if existing is not None:
                row.jira_issue_key, row.final_jira_status = existing
                row.jira_issue_url = f"{config.jira_url}/browse/{row.jira_issue_key}"
                if config.update_existing_estimates:
                    try:
                        jira.update_original_estimate(row.jira_issue_key, estimate.jira_time or "")
                    except (ApiError, ValueError) as error:
                        summary.failures += 1
                        row.result = "failed"
                        row.error_details = str(error)
                        continue
                    summary.estimates_updated += 1
                if current_planned_start is not None and planned_due is not None:
                    try:
                        jira.update_planning(
                            row.jira_issue_key,
                            analysis.summary,
                            start_field_id,
                            current_planned_start,
                            planned_due,
                        )
                    except (ApiError, ValueError) as error:
                        summary.failures += 1
                        row.result = "failed"
                        row.error_details = str(error)
                        continue
                    summary.planning_updated += 1
                if config.update_existing_sprints:
                    sprint_due = planned_due
                    if sprint_due is None:
                        try:
                            sprint_due = jira.get_due_date(row.jira_issue_key)
                        except (ApiError, ValueError) as error:
                            summary.failures += 1
                            row.result = "failed"
                            row.error_details = str(error)
                            continue
                    try:
                        sprint_id, sprint_name = jira.find_sprint_for_date(sprint_due)
                        jira.assign_issue_to_sprint(row.jira_issue_key, sprint_id)
                        if not jira.issue_matches_sprint(row.jira_issue_key, sprint_id):
                            raise ApiError("Updated issue does not match the selected sprint.")
                        if not jira.issue_matches_board(row.jira_issue_key):
                            raise ApiError("Updated issue does not match the configured Jira board.")
                    except (ApiError, ValueError) as error:
                        summary.failures += 1
                        row.result = "failed"
                        row.error_details = str(error)
                        continue
                    row.sprint_name = sprint_name
                    summary.sprints_updated += 1
                if config.update_existing_epic is not None:
                    try:
                        jira.update_parent(row.jira_issue_key, config.update_existing_epic)
                        if not jira.issue_has_parent(row.jira_issue_key, config.update_existing_epic):
                            raise ApiError("Updated issue does not match the requested Epic parent.")
                    except (ApiError, ValueError) as error:
                        summary.failures += 1
                        row.result = "failed"
                        row.error_details = str(error)
                        continue
                    row.parent_issue_key = config.update_existing_epic
                    summary.epic_links_updated += 1
                existing_updated = (
                    config.update_existing_estimates
                    or current_planned_start is not None
                    or config.update_existing_sprints
                    or config.update_existing_epic is not None
                )
                if existing_updated:
                    row.result = "updated"
                    updates = []
                    if config.update_existing_estimates:
                        updates.append("Original estimate")
                    if current_planned_start is not None:
                        updates.append("summary and planning dates")
                    if config.update_existing_sprints:
                        updates.append("Sprint")
                    if config.update_existing_epic is not None:
                        updates.append("Epic parent")
                    row.error_details = f"Updated {'; '.join(updates)} on existing import"
                else:
                    summary.existing_skipped += 1
                    row.result = "skipped"
                    row.error_details = "already imported"
                continue

        if config.dry_run:
            row.result = "skipped"
            row.final_jira_status = "Dry run"
            row.error_details = "dry-run preview; no Jira mutation attempted"
            if config.verbose:
                print(f"PREVIEW {commit.short_sha}: {analysis.summary}")
            continue

        try:
            payload = build_issue_payload(
                config.jira_project_key,
                issue_type_name,
                issue_type_id,
                create_fields,
                commit,
                analysis,
                estimate,
                dates,
            )
            issue_key, issue_url = jira.create_issue(payload)
            summary.jira_created += 1
            row.jira_issue_key = issue_key
            row.jira_issue_url = issue_url
            _, status_name, status_category = jira.get_status(issue_key)
            row.final_jira_status = status_name
            if status_name.casefold() != "to do" or status_category.casefold() != "new":
                raise ApiError("Created issue did not enter the To Do status.")
            if board_jql is None:
                raise ApiError("Created issue could not be checked against the board filter.")
            if not jira.issue_matches_board(issue_key):
                raise ApiError("Created issue does not match the existing board filter.")
            if not jira.issue_matches_backlog(issue_key):
                raise ApiError("Created issue does not appear in the configured Jira backlog.")
            summary.created_todo += 1
            row.result = "created"
        except (ApiError, ValueError) as error:
            summary.failures += 1
            row.result = "failed"
            row.error_details = str(error)
    return records, summary


def print_summary(summary: ImportSummary) -> None:
    """Print only non-secret final counters and limitations."""
    print(f"Commits inspected: {summary.commits_inspected}")
    print(f"Commits accepted: {summary.commits_accepted}")
    print(f"Commits ignored: {summary.commits_ignored}")
    print(f"Items requiring manual review: {summary.manual_review}")
    print(f"Existing Jira items skipped: {summary.existing_skipped}")
    print(f"Jira items created: {summary.jira_created}")
    print(f"Created items retained in To Do backlog: {summary.created_todo}")
    print(f"Original estimates updated: {summary.estimates_updated}")
    print(f"Summaries and planning dates updated: {summary.planning_updated}")
    print(f"Sprint assignments updated: {summary.sprints_updated}")
    print(f"Epic parent links updated: {summary.epic_links_updated}")
    print(f"Failures: {summary.failures}")
    if summary.fields_discovered:
        print(f"Relevant Jira fields discovered: {', '.join(summary.fields_discovered)}")
    for limitation in summary.limitations:
        print(f"Limitation: {limitation}")


def print_credential_status(config: ImportConfig) -> None:
    """Print authentication state without revealing any credential value."""
    print(f"Credential source: {config.credential_source}")
    github_state = "configured" if config.github_token else "not configured"
    jira_state = "configured" if config.has_jira_credentials else "not configured"
    print(f"GitHub authentication: {github_state}")
    print(f"Jira authentication: {jira_state}")
    if config.jira_email:
        print(f"Jira account: {mask_email(config.jira_email)}")
    for warning in config.credential_warnings:
        print(f"Warning: {warning}")


def main(arguments: Sequence[str] | None = None) -> int:
    """Run the CLI, always write reports after a completed collection, and return status."""
    try:
        config = load_config(arguments)
    except (ConfigurationError, ValueError) as error:
        print(f"Jira history import failed: {error}", file=sys.stderr)
        return 2
    print_credential_status(config)
    try:
        records, summary = run_import(config)
    except (ApiError, ConfigurationError, ValueError) as error:
        records = []
        summary = ImportSummary(
            failures=1,
            limitations=[*config.credential_warnings, str(error)],
        )
        print(f"Jira history import failed: {error}", file=sys.stderr)
        json_path, csv_path = write_reports(
            config.output_report,
            records,
            summary,
            "apply" if config.apply else "dry-run",
        )
        print_summary(summary)
        print(f"JSON report: {json_path}")
        print(f"CSV report: {csv_path}")
        return 2
    json_path, csv_path = write_reports(
        config.output_report,
        records,
        summary,
        "apply" if config.apply else "dry-run",
    )
    print_summary(summary)
    print(f"JSON report: {json_path}")
    print(f"CSV report: {csv_path}")
    return 1 if summary.failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
