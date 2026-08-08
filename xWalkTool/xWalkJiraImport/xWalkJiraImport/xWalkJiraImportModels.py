"""Define records shared by the xWalk Jira import workflow."""

from __future__ import annotations

from dataclasses import asdict, dataclass, field
from datetime import datetime
from typing import Any


@dataclass(frozen=True)
class ChangedFile:
    """Describe one path reported by the GitHub commit API."""

    filename: str
    status: str
    additions: int
    deletions: int
    changes: int
    previous_filename: str | None = None
    patch: str | None = None


@dataclass(frozen=True)
class CommitRecord:
    """Contain complete immutable evidence for one Git commit."""

    sha: str
    short_sha: str
    title: str
    body: str
    author_name: str
    author_email: str
    committer_name: str
    committer_email: str
    author_date: datetime | None
    commit_date: datetime | None
    url: str
    parents: tuple[str, ...]
    files: tuple[ChangedFile, ...]
    additions: int
    deletions: int
    total_changes: int


@dataclass(frozen=True)
class CommitAnalysis:
    """Record the semantic acceptance and classification of one commit."""

    accepted: bool
    component: str
    issue_type: str
    summary: str
    labels: tuple[str, ...]
    meaningful_files: tuple[ChangedFile, ...]
    testing_evidence: tuple[str, ...]
    reason: str = ""
    manual_review: bool = False
    unrelated_change_groups: int = 1


@dataclass(frozen=True)
class EffortEstimate:
    """Describe one evidence-based effort estimate."""

    level: str
    display_time: str
    jira_time: str | None
    effort_minutes: int | None
    story_points: int
    confidence: str
    rationale: str
    manual_review: bool = False


@dataclass(frozen=True)
class HistoricalDates:
    """Contain inferred historical timestamps and their limitations."""

    start: datetime | None
    completion: datetime
    confidence: str
    note: str


@dataclass(frozen=True)
class JiraField:
    """Describe one Jira field discovered from create metadata."""

    field_id: str
    name: str
    schema_type: str
    custom_type: str
    required: bool


@dataclass(frozen=True)
class JiraMetadata:
    """Contain issue types and supported fields discovered for a project."""

    issue_types: dict[str, str]
    fields_by_issue_type: dict[str, dict[str, JiraField]]
    all_fields: dict[str, JiraField]

    def choose_issue_type(self, requested: str) -> tuple[str, str]:
        """Choose a supported type, falling back to Task as required."""
        normalized = {name.casefold(): (name, value) for name, value in self.issue_types.items()}
        selected = normalized.get(requested.casefold())
        if selected is not None:
            return selected
        task = normalized.get("task")
        if task is not None:
            return task
        if not self.issue_types:
            raise ValueError("Jira returned no creatable issue types for the project.")
        first_name = sorted(self.issue_types, key=str.casefold)[0]
        return first_name, self.issue_types[first_name]


@dataclass
class ImportRecord:
    """Represent one row written to both JSON and CSV reports."""

    commit_sha: str
    commit_date: str
    component: str
    generated_summary: str
    issue_type: str
    estimated_time: str
    story_points: int
    historical_start_date: str
    historical_completion_date: str
    confidence: str
    jira_issue_key: str = ""
    jira_issue_url: str = ""
    final_jira_status: str = ""
    result: str = "skipped"
    error_details: str = ""

    def as_dict(self) -> dict[str, Any]:
        """Return a stable serializable representation."""
        return asdict(self)


@dataclass
class ImportSummary:
    """Accumulate final counters without conflating ignored and duplicate commits."""

    commits_inspected: int = 0
    commits_accepted: int = 0
    commits_ignored: int = 0
    manual_review: int = 0
    existing_skipped: int = 0
    jira_created: int = 0
    transitioned_done: int = 0
    failures: int = 0
    fields_discovered: list[str] = field(default_factory=list)
    limitations: list[str] = field(default_factory=list)

    def as_dict(self) -> dict[str, Any]:
        """Return counters and diagnostics for the JSON report."""
        return asdict(self)
