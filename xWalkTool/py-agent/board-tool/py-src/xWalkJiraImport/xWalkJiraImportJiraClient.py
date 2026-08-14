"""Discover Jira metadata and guard historical xWalk item creation."""

from __future__ import annotations

import re
from datetime import date, datetime
from typing import Any, Iterable, Mapping
from zoneinfo import ZoneInfo

import requests

from .xWalkJiraImportGitHubClient import ApiError, request_json
from .xWalkJiraImportModels import (
    ChangedFile,
    CommitAnalysis,
    CommitRecord,
    EffortEstimate,
    HistoricalDates,
    JiraField,
    JiraMetadata,
)


IMPORT_MARKER_PREFIX = "xwalk-git-import:"
NOTICE = (
    "Historical record: This work item was reconstructed from an existing Git commit for thesis planning "
    "and traceability. Jira's system creation and resolution timestamps represent the import operation, "
    "while the historical development dates are recorded below."
)
STOCKHOLM = ZoneInfo("Europe/Stockholm")


class JiraSafetyError(Exception):
    """Reject every Jira mutation unless the client was explicitly write-enabled."""


def _text(value: str, *, bold: bool = False, link: str | None = None) -> dict[str, Any]:
    """Create one ADF text node with optional safe marks."""
    node: dict[str, Any] = {"type": "text", "text": value}
    marks: list[dict[str, Any]] = []
    if bold:
        marks.append({"type": "strong"})
    if link:
        marks.append({"type": "link", "attrs": {"href": link}})
    if marks:
        node["marks"] = marks
    return node


def _paragraph(*content: dict[str, Any]) -> dict[str, Any]:
    """Create one ADF paragraph."""
    return {"type": "paragraph", "content": list(content)}


def _heading(value: str, level: int = 2) -> dict[str, Any]:
    """Create one ADF heading node."""
    return {"type": "heading", "attrs": {"level": level}, "content": [_text(value)]}


def _bullet_list(values: Iterable[str]) -> dict[str, Any]:
    """Create one ADF bullet list from non-empty values."""
    items = [
        {"type": "listItem", "content": [_paragraph(_text(value))]}
        for value in values
        if value
    ]
    return {"type": "bulletList", "content": items}


def _changed_file_detail(file: ChangedFile) -> str:
    """Describe one changed file as completed human-readable work."""
    action = {
        "added": "Added",
        "modified": "Updated",
        "renamed": "Renamed",
        "removed": "Removed",
        "deleted": "Removed",
    }.get(file.status.casefold(), "Changed")
    path_detail = file.filename
    if file.status.casefold() == "renamed" and file.previous_filename:
        path_detail = f"{file.previous_filename} to {file.filename}"
    return f"{action} {path_detail} (+{file.additions}/-{file.deletions} lines)."


def build_description_adf(
    commit: CommitRecord,
    analysis: CommitAnalysis,
    estimate: EffortEstimate,
    dates: HistoricalDates,
) -> dict[str, Any]:
    """Build a detailed, sectioned Jira API v3 ADF historical description."""
    start_text = dates.start.isoformat() if dates.start is not None else "Requires manual review"
    completion_text = dates.completion.isoformat()
    original_message = commit.title
    if commit.body:
        original_message += f"\n\n{commit.body}"
    important_files = [
        _changed_file_detail(item)
        for item in sorted(analysis.meaningful_files, key=lambda value: value.changes, reverse=True)[:12]
    ]
    testing = list(analysis.testing_evidence) or ["No explicit testing evidence was found in this commit."]
    reason_lines = [line.strip().lstrip("- ") for line in commit.body.splitlines() if line.strip()]
    change_reason = reason_lines[0] if reason_lines else f"Recorded by the commit title: {commit.title}"
    semantic_additions = sum(item.additions for item in analysis.meaningful_files)
    semantic_deletions = sum(item.deletions for item in analysis.meaningful_files)
    excluded_files = len(commit.files) - len(analysis.meaningful_files)
    parent_text = ", ".join(parent[:12] for parent in commit.parents) or "No parent commit reported"
    human_activities = [
        "Review the repository context and understand the intended behavior.",
        "Investigate the problem or design the implementation approach.",
        "Implement and integrate the semantic source, configuration, test, or documentation changes.",
        "Run relevant local verification and regression checks.",
        "Allow for normal human code review, corrections, and documentation updates.",
    ]
    content: list[dict[str, Any]] = [
        _heading("Overview"),
        _paragraph(_text(analysis.summary)),
        _paragraph(
            _text(
                "This work item records completed development reconstructed from the semantic changes in the "
                "linked Git commit. The description is based on repository evidence and does not claim work "
                "that is not visible in the commit."
            )
        ),
        _heading("Engineering context"),
        _paragraph(_text(change_reason)),
        _bullet_list(
            [
                f"Affected component: {analysis.component}",
                f"Work-item classification: {analysis.issue_type}",
                f"Analysis result: {analysis.reason}",
                f"Semantic scope: {len(analysis.meaningful_files)} meaningful files, "
                f"+{semantic_additions}/-{semantic_deletions} lines.",
                f"Excluded from effort calculation: {excluded_files} generated or non-effort files.",
            ]
        ),
        _heading("Completed work"),
        _paragraph(
            _text(
                "The following important files provide the strongest evidence of the implementation. "
                "Generated files, model weights, datasets, lock files, build output, and third-party content "
                "are omitted from this list."
            )
        ),
        _bullet_list(important_files),
        _heading("Human development estimate"),
        _paragraph(
            _text(
                "This estimate represents the expected effort for a human developer. It is not based on AI "
                "generation speed, automated code-editing time, or changed-line count alone."
            )
        ),
        _bullet_list(
            [
                f"Estimated effort: {estimate.level} ({estimate.display_time})",
                f"Jira original estimate: {estimate.jira_time or 'Requires manual review'}",
                f"Story-point estimate: {estimate.story_points}",
                f"Effort confidence: {estimate.confidence}",
            ]
        ),
        _paragraph(_text(estimate.rationale)),
        _paragraph(_text("Human activities included in the estimate:", bold=True)),
        _bullet_list(human_activities),
        _heading("Validation and testing"),
        _paragraph(
            _text(
                "Testing evidence is reported conservatively from changed paths and the original commit "
                "message. Absence of evidence does not mean that no testing occurred."
            )
        ),
        _bullet_list(testing),
        _heading("Historical timeline"),
        _bullet_list(
            [
                f"Historical start: {start_text}",
                f"Historical completion: {completion_text}",
                f"Timeline confidence: {dates.confidence}",
            ]
        ),
        _paragraph(_text(dates.note)),
        _heading("Git traceability"),
        _bullet_list(
            [
                f"Commit author: {commit.author_name}",
                f"Commit SHA: {commit.sha}",
                f"Parent commits: {parent_text}",
                f"Complete commit statistics: +{commit.additions}/-{commit.deletions} lines, "
                f"{commit.total_changes} total changes.",
            ]
        ),
        _paragraph(_text("GitHub commit: ", bold=True), _text(commit.url, link=commit.url)),
        _heading("Original commit message", level=3),
        {"type": "codeBlock", "attrs": {"language": "text"}, "content": [_text(original_message)]},
        _heading("Import provenance"),
        {"type": "panel", "attrs": {"panelType": "info"}, "content": [_paragraph(_text(NOTICE))]},
        _paragraph(_text(f"{IMPORT_MARKER_PREFIX}{commit.sha}")),
    ]
    return {"type": "doc", "version": 1, "content": content}


def flatten_adf_text(value: object) -> str:
    """Flatten nested ADF nodes for exact duplicate-marker verification."""
    if isinstance(value, dict):
        text = value.get("text")
        pieces = [str(text)] if isinstance(text, str) else []
        pieces.extend(flatten_adf_text(item) for item in value.get("content", []))
        return " ".join(piece for piece in pieces if piece)
    if isinstance(value, list):
        return " ".join(flatten_adf_text(item) for item in value)
    return ""


def _normalize_field_name(value: str) -> str:
    """Normalize Jira field names for case- and punctuation-independent discovery."""
    return re.sub(r"[^a-z0-9]+", " ", value.casefold()).strip()


def _find_field(fields: Mapping[str, JiraField], names: set[str]) -> JiraField | None:
    """Find the first field whose normalized name matches one requested name."""
    normalized_names = {_normalize_field_name(value) for value in names}
    candidates = [field for field in fields.values() if _normalize_field_name(field.name) in normalized_names]
    return sorted(candidates, key=lambda value: value.field_id)[0] if candidates else None


def _date_field_value(field: JiraField, value: datetime) -> str:
    """Format custom date or datetime values from their discovered schema."""
    if field.schema_type == "date" or "date" in field.custom_type and "time" not in field.custom_type:
        return value.date().isoformat()
    return value.isoformat()


def build_issue_payload(
    project_key: str,
    issue_type_name: str,
    issue_type_id: str,
    create_fields: Mapping[str, JiraField],
    commit: CommitRecord,
    analysis: CommitAnalysis,
    estimate: EffortEstimate,
    dates: HistoricalDates,
) -> dict[str, Any]:
    """Build only fields supported by the selected Jira create screen."""
    fields: dict[str, Any] = {
        "project": {"key": project_key},
        "issuetype": {"id": issue_type_id},
        "summary": analysis.summary,
        "description": build_description_adf(commit, analysis, estimate, dates),
    }
    if "labels" in create_fields:
        fields["labels"] = list(analysis.labels)
    if "timetracking" in create_fields and estimate.jira_time:
        fields["timetracking"] = {"originalEstimate": estimate.jira_time}
    if "duedate" in create_fields:
        fields["duedate"] = dates.completion.date().isoformat()

    story_points = _find_field(create_fields, {"Story point estimate", "Story points"})
    if story_points is not None and story_points.schema_type in {"number", "integer"}:
        fields[story_points.field_id] = estimate.story_points
    start_date = _find_field(create_fields, {"Start date"})
    if start_date is not None and dates.start is not None:
        fields[start_date.field_id] = _date_field_value(start_date, dates.start)
    historical_created = _find_field(create_fields, {"Historical created date"})
    if historical_created is not None and dates.start is not None:
        fields[historical_created.field_id] = _date_field_value(historical_created, dates.start)
    historical_done = _find_field(create_fields, {"Historical done date"})
    if historical_done is not None:
        fields[historical_done.field_id] = _date_field_value(historical_done, dates.completion)

    return {
        "fields": fields,
        "properties": [{"key": "xwalk.git.import.sha", "value": commit.sha}],
        "historyMetadata": {
            "type": "xwalk-git-history-import",
            "description": f"Historical import for {commit.short_sha}",
            "activityDescription": f"Reconstructed {issue_type_name} from Git history",
            "generator": {"type": "xwalk-tool", "id": "jira-import"},
        },
    }


def select_done_transition(
    transitions: Iterable[Mapping[str, Any]],
    visited_status_ids: set[str] | None = None,
) -> Mapping[str, Any] | None:
    """Prefer a direct Done transition, then a forward non-visited workflow step."""
    values = list(transitions)
    visited = visited_status_ids or set()

    def destination(item: Mapping[str, Any]) -> Mapping[str, Any]:
        value = item.get("to", {})
        return value if isinstance(value, dict) else {}

    preferred_done_names = ("done", "completed", "resolved", "closed")
    for preferred_name in preferred_done_names:
        for item in values:
            target_name = str(destination(item).get("name", "")).casefold()
            if target_name == preferred_name:
                return item

    excluded_done_names = ("cancel", "reject", "declin", "won't", "wont")
    for item in values:
        target = destination(item)
        category = target.get("statusCategory", {})
        category_key = str(category.get("key", "")).casefold() if isinstance(category, dict) else ""
        target_name = str(target.get("name", "")).casefold()
        if category_key == "done" and not any(value in target_name for value in excluded_done_names):
            return item

    preference = ("in progress", "review", "testing", "ready", "selected", "to do")
    candidates = [item for item in values if str(destination(item).get("id", "")) not in visited]
    for name in preference:
        for item in candidates:
            if name in str(destination(item).get("name", "")).casefold():
                return item
    return candidates[0] if candidates else None


class JiraClient:
    """Provide read-only discovery and guarded writes to one Jira Cloud project."""

    def __init__(
        self,
        base_url: str,
        email: str,
        api_token: str,
        project_key: str,
        board_id: int,
        *,
        allow_writes: bool = False,
        session: requests.Session | None = None,
    ) -> None:
        """Configure basic authentication without exposing either credential."""
        self.base_url = base_url.rstrip("/")
        self.project_key = project_key
        self.board_id = board_id
        self.allow_writes = allow_writes
        self.session = session or requests.Session()
        self.session.trust_env = False
        self.session.auth = (email, api_token)
        self.session.headers.update(
            {
                "Accept": "application/json",
                "Content-Type": "application/json",
                "User-Agent": "xwalk-thesis-jira-importer",
            }
        )

    def _url(self, path: str) -> str:
        """Join one trusted Jira API path to the configured site."""
        return f"{self.base_url}{path}"

    def discover_metadata(self) -> JiraMetadata:
        """Discover creatable issue types and their supported project fields."""
        raw_fields = request_json(self.session, "GET", self._url("/rest/api/3/field"))
        all_fields: dict[str, JiraField] = {}
        if not isinstance(raw_fields, list):
            raise ApiError("Jira field discovery returned an unexpected response.")
        for item in raw_fields:
            if not isinstance(item, dict) or not item.get("id"):
                continue
            field = self._decode_field(item)
            all_fields[field.field_id] = field

        issue_types: dict[str, str] = {}
        start_at = 0
        while True:
            payload = request_json(
                self.session,
                "GET",
                self._url(f"/rest/api/3/issue/createmeta/{self.project_key}/issuetypes"),
                params={"startAt": start_at, "maxResults": 50},
            )
            if not isinstance(payload, dict):
                raise ApiError("Jira issue-type discovery returned an unexpected response.")
            values = payload.get("issueTypes", payload.get("values", []))
            if not isinstance(values, list):
                raise ApiError("Jira issue-type discovery omitted its values.")
            for item in values:
                if isinstance(item, dict) and item.get("id") and item.get("name") and not item.get("subtask"):
                    issue_types[str(item["name"])] = str(item["id"])
            start_at += len(values)
            if start_at >= int(payload.get("total", start_at)) or not values:
                break

        fields_by_type: dict[str, dict[str, JiraField]] = {}
        for issue_type_id in issue_types.values():
            fields_by_type[issue_type_id] = self._discover_create_fields(issue_type_id)
        return JiraMetadata(issue_types, fields_by_type, all_fields)

    @staticmethod
    def _decode_field(item: Mapping[str, Any]) -> JiraField:
        """Decode one field from global or create metadata."""
        schema = item.get("schema", {})
        schema = schema if isinstance(schema, dict) else {}
        field_id = str(item.get("fieldId", item.get("key", item.get("id", ""))))
        return JiraField(
            field_id=field_id,
            name=str(item.get("name", field_id)),
            schema_type=str(schema.get("type", "")),
            custom_type=str(schema.get("custom", "")),
            required=bool(item.get("required", False)),
        )

    def _discover_create_fields(self, issue_type_id: str) -> dict[str, JiraField]:
        """Retrieve every create-field metadata page for one issue type."""
        results: dict[str, JiraField] = {}
        start_at = 0
        path = f"/rest/api/3/issue/createmeta/{self.project_key}/issuetypes/{issue_type_id}"
        while True:
            payload = request_json(
                self.session,
                "GET",
                self._url(path),
                params={"startAt": start_at, "maxResults": 100},
            )
            if not isinstance(payload, dict):
                raise ApiError("Jira create-field discovery returned an unexpected response.")
            values = payload.get("fields", payload.get("values", []))
            if isinstance(values, dict):
                values = [dict(value, fieldId=key) for key, value in values.items() if isinstance(value, dict)]
            if not isinstance(values, list):
                raise ApiError("Jira create-field discovery omitted its values.")
            for item in values:
                if isinstance(item, dict):
                    field = self._decode_field(item)
                    if field.field_id:
                        results[field.field_id] = field
            start_at += len(values)
            if start_at >= int(payload.get("total", start_at)) or not values:
                break
        return results

    def find_existing_import(self, sha: str) -> tuple[str, str] | None:
        """Search by full SHA and verify the exact machine-readable marker."""
        marker = f"{IMPORT_MARKER_PREFIX}{sha}"
        jql = f'project = "{self.project_key}" AND text ~ "{sha}"'
        payload = request_json(
            self.session,
            "POST",
            self._url("/rest/api/3/search/jql"),
            json={"jql": jql, "fields": ["description", "status"], "maxResults": 100},
        )
        if not isinstance(payload, dict) or not isinstance(payload.get("issues", []), list):
            raise ApiError("Jira duplicate search returned an unexpected response.")
        for issue in payload.get("issues", []):
            if not isinstance(issue, dict):
                continue
            fields = issue.get("fields", {})
            fields = fields if isinstance(fields, dict) else {}
            if marker in flatten_adf_text(fields.get("description")):
                status = fields.get("status", {})
                status_name = str(status.get("name", "")) if isinstance(status, dict) else ""
                return str(issue.get("key", "")), status_name
        return None

    def create_issue(self, payload: Mapping[str, Any]) -> tuple[str, str]:
        """Create one issue only after explicit write authorization."""
        self._require_writes()
        result = request_json(
            self.session,
            "POST",
            self._url("/rest/api/3/issue"),
            json=dict(payload),
            expected=(201,),
        )
        if not isinstance(result, dict) or not result.get("key"):
            raise ApiError("Jira create issue response omitted the issue key.")
        key = str(result["key"])
        return key, f"{self.base_url}/browse/{key}"

    def update_original_estimate(self, issue_key: str, jira_time: str) -> None:
        """Update one exact existing issue's human-effort Original estimate."""
        self._require_writes()
        if not issue_key.strip() or not jira_time.strip():
            raise ValueError("The Jira issue key and Original estimate must not be empty.")
        request_json(
            self.session,
            "PUT",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            json={"fields": {"timetracking": {"originalEstimate": jira_time}}},
            expected=(204,),
        )

    def get_due_date(self, issue_key: str) -> date:
        """Return the configured Due date for one existing planning anchor."""
        result = request_json(
            self.session,
            "GET",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            params={"fields": "duedate"},
        )
        fields = result.get("fields", {}) if isinstance(result, dict) else {}
        value = fields.get("duedate") if isinstance(fields, dict) else None
        if not isinstance(value, str):
            raise ValueError(f"Jira issue {issue_key} has no Due date for sequential planning.")
        try:
            return date.fromisoformat(value)
        except ValueError as error:
            raise ValueError(f"Jira issue {issue_key} returned an invalid Due date.") from error

    def update_planning(
        self,
        issue_key: str,
        summary: str,
        start_field_id: str,
        start: date,
        due: date,
    ) -> None:
        """Update one exact existing import's summary and sequential planning dates."""
        self._require_writes()
        if not issue_key.strip() or not summary.strip() or not start_field_id.strip():
            raise ValueError("The issue key, summary, and Start date field must not be empty.")
        if due < start:
            raise ValueError("The planned Due date must not precede the Start date.")
        request_json(
            self.session,
            "PUT",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            json={
                "fields": {
                    "summary": summary,
                    start_field_id: start.isoformat(),
                    "duedate": due.isoformat(),
                }
            },
            expected=(204,),
        )

    @staticmethod
    def _sprint_boundary(value: object) -> date | None:
        """Convert one Jira sprint timestamp to its Stockholm calendar date."""
        if not isinstance(value, str):
            return None
        try:
            parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
        except ValueError:
            return None
        if parsed.tzinfo is None:
            return None
        return parsed.astimezone(STOCKHOLM).date()

    def find_sprint_for_date(self, value: date) -> tuple[int, str]:
        """Select the board sprint whose half-open date range contains one Due date."""
        start_at = 0
        candidates: list[tuple[date, date, int, str]] = []
        while True:
            result = request_json(
                self.session,
                "GET",
                self._url(f"/rest/agile/1.0/board/{self.board_id}/sprint"),
                params={"state": "active,closed,future", "startAt": start_at, "maxResults": 50},
            )
            values = result.get("values", []) if isinstance(result, dict) else []
            if not isinstance(values, list):
                raise ApiError("Jira sprint discovery returned an unexpected response.")
            for item in values:
                if not isinstance(item, dict):
                    continue
                start = self._sprint_boundary(item.get("startDate"))
                end = self._sprint_boundary(item.get("endDate"))
                sprint_id = item.get("id")
                name = item.get("name")
                if start is not None and end is not None and sprint_id is not None and isinstance(name, str):
                    candidates.append((start, end, int(sprint_id), name))
            start_at += len(values)
            if not values or bool(result.get("isLast", start_at >= int(result.get("total", start_at)))):
                break
        for start, end, sprint_id, name in sorted(candidates):
            if start <= value < end:
                return sprint_id, name
        raise ValueError(f"No board sprint contains Due date {value.isoformat()}.")

    def assign_issue_to_sprint(self, issue_key: str, sprint_id: int) -> None:
        """Move one exact existing issue into the selected Jira Software sprint."""
        self._require_writes()
        if not issue_key.strip() or sprint_id <= 0:
            raise ValueError("The issue key and sprint ID must be valid.")
        request_json(
            self.session,
            "POST",
            self._url(f"/rest/agile/1.0/sprint/{sprint_id}/issue"),
            json={"issues": [issue_key]},
            expected=(204,),
        )

    def issue_matches_sprint(self, issue_key: str, sprint_id: int) -> bool:
        """Confirm one issue belongs to the selected sprint."""
        result = request_json(
            self.session,
            "GET",
            self._url(f"/rest/agile/1.0/sprint/{sprint_id}/issue"),
            params={"jql": f'key = "{issue_key}"', "fields": "key", "maxResults": 1},
        )
        issues = result.get("issues", []) if isinstance(result, dict) else []
        return bool(issues)

    def require_epic(self, issue_key: str) -> None:
        """Reject a requested parent unless Jira reports that it is an Epic."""
        result = request_json(
            self.session,
            "GET",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            params={"fields": "issuetype"},
        )
        fields = result.get("fields", {}) if isinstance(result, dict) else {}
        issue_type = fields.get("issuetype", {}) if isinstance(fields, dict) else {}
        name = str(issue_type.get("name", "")) if isinstance(issue_type, dict) else ""
        if name.casefold() != "epic":
            raise ValueError(f"Jira issue {issue_key} is not an Epic.")

    def update_parent(self, issue_key: str, parent_key: str) -> None:
        """Attach one exact existing import to a validated Epic parent."""
        self._require_writes()
        if not issue_key.strip() or not parent_key.strip():
            raise ValueError("The issue key and Epic parent key must not be empty.")
        request_json(
            self.session,
            "PUT",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            json={"fields": {"parent": {"key": parent_key}}},
            expected=(204,),
        )

    def issue_has_parent(self, issue_key: str, parent_key: str) -> bool:
        """Confirm one issue has the requested Epic parent."""
        result = request_json(
            self.session,
            "GET",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            params={"fields": "parent"},
        )
        fields = result.get("fields", {}) if isinstance(result, dict) else {}
        parent = fields.get("parent", {}) if isinstance(fields, dict) else {}
        return isinstance(parent, dict) and str(parent.get("key", "")) == parent_key

    def get_status(self, issue_key: str) -> tuple[str, str, str]:
        """Return status ID, name, and category for one issue."""
        result = request_json(
            self.session,
            "GET",
            self._url(f"/rest/api/3/issue/{issue_key}"),
            params={"fields": "status"},
        )
        fields = result.get("fields", {}) if isinstance(result, dict) else {}
        status = fields.get("status", {}) if isinstance(fields, dict) else {}
        category = status.get("statusCategory", {}) if isinstance(status, dict) else {}
        return (
            str(status.get("id", "")) if isinstance(status, dict) else "",
            str(status.get("name", "")) if isinstance(status, dict) else "",
            str(category.get("key", "")) if isinstance(category, dict) else "",
        )

    def transition_to_done(self, issue_key: str, maximum_steps: int = 10) -> tuple[str, bool]:
        """Follow available forward transitions until Jira reports a Done category."""
        self._require_writes()
        visited: set[str] = set()
        for _ in range(maximum_steps):
            status_id, status_name, category = self.get_status(issue_key)
            if category.casefold() == "done":
                return status_name, True
            visited.add(status_id)
            payload = request_json(
                self.session,
                "GET",
                self._url(f"/rest/api/3/issue/{issue_key}/transitions"),
                params={"expand": "transitions.fields"},
            )
            transitions = payload.get("transitions", []) if isinstance(payload, dict) else []
            if not isinstance(transitions, list):
                raise ApiError("Jira transition discovery returned an unexpected response.")
            selected = select_done_transition(
                [item for item in transitions if isinstance(item, dict)],
                visited,
            )
            if selected is None or not selected.get("id"):
                return status_name, False
            request_json(
                self.session,
                "POST",
                self._url(f"/rest/api/3/issue/{issue_key}/transitions"),
                json={"transition": {"id": str(selected["id"])}},
                expected=(204,),
            )
        _, status_name, category = self.get_status(issue_key)
        return status_name, category.casefold() == "done"

    def get_board_filter_jql(self) -> str:
        """Read board configuration and its saved filter without changing either."""
        configuration = request_json(
            self.session,
            "GET",
            self._url(f"/rest/agile/1.0/board/{self.board_id}/configuration"),
        )
        location = configuration.get("location", {}) if isinstance(configuration, dict) else {}
        filter_data = location.get("filter", {}) if isinstance(location, dict) else {}
        filter_id = filter_data.get("id") if isinstance(filter_data, dict) else None
        if filter_id is None and isinstance(configuration, dict):
            direct_filter = configuration.get("filter", {})
            filter_id = direct_filter.get("id") if isinstance(direct_filter, dict) else None
        if filter_id is None:
            raise ApiError("Jira board configuration omitted its saved filter.")
        result = request_json(
            self.session,
            "GET",
            self._url(f"/rest/api/3/filter/{filter_id}"),
            params={"expand": "jql"},
        )
        jql = result.get("jql") if isinstance(result, dict) else None
        if not isinstance(jql, str) or not jql.strip():
            raise ApiError("Jira board filter did not expose readable JQL.")
        return jql

    def issue_matches_board(self, issue_key: str) -> bool:
        """Confirm one created issue satisfies the board's existing saved filter."""
        payload = request_json(
            self.session,
            "GET",
            self._url(f"/rest/agile/1.0/board/{self.board_id}/issue"),
            params={"jql": f'key = "{issue_key}"', "fields": "key", "maxResults": 1},
        )
        issues = payload.get("issues", []) if isinstance(payload, dict) else []
        return bool(issues)

    def issue_matches_backlog(self, issue_key: str) -> bool:
        """Confirm one created issue appears in the configured board backlog."""
        payload = request_json(
            self.session,
            "GET",
            self._url(f"/rest/agile/1.0/board/{self.board_id}/backlog"),
            params={"jql": f'key = "{issue_key}"', "fields": "key", "maxResults": 1},
        )
        issues = payload.get("issues", []) if isinstance(payload, dict) else []
        return bool(issues)

    def _require_writes(self) -> None:
        """Enforce the single apply-mode mutation boundary."""
        if not self.allow_writes:
            raise JiraSafetyError("Jira mutation blocked because --apply was not supplied.")
