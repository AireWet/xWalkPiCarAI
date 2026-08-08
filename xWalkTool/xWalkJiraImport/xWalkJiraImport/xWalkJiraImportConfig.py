"""Configure the xWalk Jira history importer."""

from __future__ import annotations

import argparse
import os
import re
from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import Mapping, Sequence
from urllib.parse import urlparse

from .xWalkJiraImportCredentials import (
    CredentialError,
    Credentials,
    resolve_credentials,
    resolve_netrc_path,
)


DEFAULT_JIRA_URL = "https://student-team-xwalk-rpi5.atlassian.net"
DEFAULT_JIRA_PROJECT_KEY = "TARS"
DEFAULT_GITHUB_REPOSITORY = "jochuuu/xWalkPiCarAI"
DEFAULT_GITHUB_BRANCH = "master"
DEFAULT_BOARD_ID = 3
DEFAULT_GITHUB_API_HOST = "api.github.com"


class ConfigurationError(Exception):
    """Report a safe configuration failure without exposing credential values."""


@dataclass(frozen=True)
class ImportConfig:
    """Contain validated runtime settings and secret-presence state."""

    jira_url: str
    jira_project_key: str
    jira_board_id: int
    github_repository: str
    github_branch: str
    credentials: Credentials
    netrc_file: Path
    credential_source: str
    credential_warnings: tuple[str, ...]
    apply: bool
    since: date | None
    until: date | None
    author: str | None
    max_commits: int | None
    include_merges: bool
    include_insignificant: bool
    apply_manual_review: bool
    output_report: Path
    verbose: bool
    update_existing_estimates: bool = False
    update_existing_planning_after: str | None = None
    update_existing_planning_start: date | None = None
    update_existing_sprints: bool = False
    update_existing_epic: str | None = None

    @property
    def dry_run(self) -> bool:
        """Return true unless explicit apply mode was selected."""
        return not self.apply

    @property
    def has_jira_credentials(self) -> bool:
        """Return whether both Jira Basic authentication values are present."""
        return self.credentials.has_jira_credentials

    @property
    def github_token(self) -> str | None:
        """Return the optional GitHub token without copying it to diagnostics."""
        return self.credentials.github_token

    @property
    def jira_email(self) -> str | None:
        """Return the Jira login loaded from the selected credential source."""
        return self.credentials.jira_email

    @property
    def jira_api_token(self) -> str | None:
        """Return the Jira API token loaded from the selected credential source."""
        return self.credentials.jira_api_token


def parse_date(value: str) -> date:
    """Parse one strict ISO calendar date for argparse."""
    try:
        return date.fromisoformat(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("expected YYYY-MM-DD") from error


def create_argument_parser() -> argparse.ArgumentParser:
    """Create the dry-run-first command-line parser."""
    parser = argparse.ArgumentParser(
        prog="xWalkJiraImport",
        description="Reconstruct completed Jira work items from historical GitHub commits."
    )
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", action="store_true", help="preview only; this is the default")
    mode.add_argument("--apply", action="store_true", help="create and complete missing Jira items")
    parser.add_argument("--since", type=parse_date, metavar="YYYY-MM-DD")
    parser.add_argument("--until", type=parse_date, metavar="YYYY-MM-DD")
    parser.add_argument("--author", metavar="NAME_OR_EMAIL")
    parser.add_argument("--branch", metavar="BRANCH")
    parser.add_argument("--jira-url", metavar="URL")
    parser.add_argument("--jira-project-key", metavar="KEY")
    parser.add_argument("--github-repository", metavar="OWNER/REPOSITORY")
    parser.add_argument(
        "--netrc-file",
        type=Path,
        metavar="PATH",
        help="override the automatic per-user ~/.netrc location",
    )
    parser.add_argument("--max-commits", type=int, metavar="NUMBER")
    parser.add_argument("--include-merges", action="store_true")
    parser.add_argument(
        "--include-insignificant",
        action="store_true",
        help="include formatting-only and insignificant typo-only commits",
    )
    parser.add_argument(
        "--apply-manual-review",
        action="store_true",
        help="allow --apply to create broad commits while retaining low-confidence estimates",
    )
    parser.add_argument(
        "--update-existing-estimates",
        action="store_true",
        help="allow --apply to update Original estimate on exact-SHA existing imports",
    )
    parser.add_argument(
        "--update-existing-planning-after",
        metavar="ISSUE_KEY",
        help="update summary, Start date, and Due date sequentially after an existing Jira issue",
    )
    parser.add_argument(
        "--update-existing-planning-start",
        type=parse_date,
        metavar="YYYY-MM-DD",
        help="update summary, Start date, and Due date from an explicit first Start date",
    )
    parser.add_argument(
        "--update-existing-sprints",
        action="store_true",
        help="assign exact-SHA existing imports to board sprints selected by Due date",
    )
    parser.add_argument(
        "--update-existing-epic",
        metavar="ISSUE_KEY",
        help="attach exact-SHA existing imports to a validated Epic parent",
    )
    parser.add_argument(
        "--output-report",
        type=Path,
        default=Path("build/jira-import-preview"),
        metavar="PATH",
    )
    parser.add_argument("--verbose", action="store_true")
    return parser


def load_config(
    arguments: Sequence[str] | None = None,
    environment: Mapping[str, str] | None = None,
) -> ImportConfig:
    """Load non-secret settings and resolve netrc-first credentials."""
    parser = create_argument_parser()
    options = parser.parse_args(arguments)
    values = os.environ if environment is None else environment

    if options.max_commits is not None and options.max_commits <= 0:
        parser.error("--max-commits must be greater than zero")
    if options.since is not None and options.until is not None and options.since > options.until:
        parser.error("--since must not be later than --until")
    if options.apply_manual_review and not options.apply:
        parser.error("--apply-manual-review requires --apply")
    if options.update_existing_estimates and not options.apply:
        parser.error("--update-existing-estimates requires --apply")
    if options.update_existing_planning_after and not options.apply:
        parser.error("--update-existing-planning-after requires --apply")
    if options.update_existing_planning_start and not options.apply:
        parser.error("--update-existing-planning-start requires --apply")
    if options.update_existing_sprints and not options.apply:
        parser.error("--update-existing-sprints requires --apply")
    if options.update_existing_epic and not options.apply:
        parser.error("--update-existing-epic requires --apply")
    if options.update_existing_planning_after and options.update_existing_planning_start:
        parser.error("select only one existing-planning anchor")

    repository = (
        options.github_repository or values.get("GITHUB_REPOSITORY", DEFAULT_GITHUB_REPOSITORY)
    ).strip()
    if re.fullmatch(r"[^/\s]+/[^/\s]+", repository) is None:
        raise ConfigurationError("GITHUB_REPOSITORY must use OWNER/REPOSITORY format.")
    branch = (options.branch or values.get("GITHUB_BRANCH", DEFAULT_GITHUB_BRANCH)).strip()
    if not branch:
        raise ConfigurationError("The selected GitHub branch must not be empty.")

    jira_url = (options.jira_url or values.get("JIRA_URL", DEFAULT_JIRA_URL)).strip().rstrip("/")
    project_key = (
        options.jira_project_key or values.get("JIRA_PROJECT_KEY", DEFAULT_JIRA_PROJECT_KEY)
    ).strip().upper()
    parsed_jira_url = urlparse(jira_url)
    if parsed_jira_url.scheme != "https" or not parsed_jira_url.hostname:
        raise ConfigurationError("JIRA_URL must be an HTTPS URL with a hostname.")
    if parsed_jira_url.username or parsed_jira_url.password:
        raise ConfigurationError("JIRA_URL must not contain credentials.")
    if re.fullmatch(r"[A-Z][A-Z0-9_]*", project_key) is None:
        raise ConfigurationError("JIRA_PROJECT_KEY is invalid.")
    schedule_after = options.update_existing_planning_after
    if schedule_after is not None:
        schedule_after = schedule_after.strip().upper()
        if re.fullmatch(rf"{re.escape(project_key)}-[1-9][0-9]*", schedule_after) is None:
            raise ConfigurationError("The planning anchor must be an issue key in the selected Jira project.")
    epic_key = options.update_existing_epic
    if epic_key is not None:
        epic_key = epic_key.strip().upper()
        if re.fullmatch(rf"{re.escape(project_key)}-[1-9][0-9]*", epic_key) is None:
            raise ConfigurationError("The Epic parent must be an issue key in the selected Jira project.")

    netrc_file = resolve_netrc_path(options.netrc_file)
    try:
        credential_resolution = resolve_credentials(
            netrc_file,
            DEFAULT_GITHUB_API_HOST,
            parsed_jira_url.hostname,
            values,
            bool(options.apply),
        )
    except CredentialError as error:
        raise ConfigurationError(str(error)) from None

    return ImportConfig(
        jira_url=jira_url,
        jira_project_key=project_key,
        jira_board_id=DEFAULT_BOARD_ID,
        github_repository=repository,
        github_branch=branch,
        credentials=credential_resolution.credentials,
        netrc_file=netrc_file,
        credential_source=credential_resolution.source,
        credential_warnings=credential_resolution.warnings,
        apply=bool(options.apply),
        since=options.since,
        until=options.until,
        author=options.author,
        max_commits=options.max_commits,
        include_merges=bool(options.include_merges),
        include_insignificant=bool(options.include_insignificant),
        apply_manual_review=bool(options.apply_manual_review),
        output_report=options.output_report,
        verbose=bool(options.verbose),
        update_existing_estimates=bool(options.update_existing_estimates),
        update_existing_planning_after=schedule_after,
        update_existing_planning_start=options.update_existing_planning_start,
        update_existing_sprints=bool(options.update_existing_sprints),
        update_existing_epic=epic_key,
    )
