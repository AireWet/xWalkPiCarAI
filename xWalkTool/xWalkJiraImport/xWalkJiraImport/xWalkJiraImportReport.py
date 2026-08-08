"""Write deterministic xWalk Jira import reports."""

from __future__ import annotations

import csv
import io
import json
import os
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable

from .xWalkJiraImportModels import ImportRecord, ImportSummary


CSV_FIELDS = (
    "commit_sha",
    "commit_date",
    "component",
    "generated_summary",
    "issue_type",
    "estimated_time",
    "story_points",
    "historical_start_date",
    "historical_completion_date",
    "confidence",
    "jira_issue_key",
    "jira_issue_url",
    "final_jira_status",
    "result",
    "error_details",
    "planned_start_date",
    "planned_due_date",
    "sprint_name",
    "parent_issue_key",
)


def report_paths(base_path: Path) -> tuple[Path, Path]:
    """Return sibling JSON and CSV paths from one base or suffixed path."""
    base = base_path
    if base.suffix.casefold() in {".json", ".csv"}:
        base = base.with_suffix("")
    return base.with_suffix(".json"), base.with_suffix(".csv")


def _atomic_write(path: Path, content: str) -> None:
    """Atomically replace one UTF-8 report in its destination directory."""
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary_name: str | None = None
    try:
        descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="") as output_file:
            output_file.write(content)
            output_file.flush()
            os.fsync(output_file.fileno())
        os.replace(temporary_name, path)
        temporary_name = None
    finally:
        if temporary_name is not None:
            try:
                os.unlink(temporary_name)
            except FileNotFoundError:
                pass


def write_reports(
    base_path: Path,
    records: Iterable[ImportRecord],
    summary: ImportSummary,
    mode: str,
) -> tuple[Path, Path]:
    """Write equivalent report records in JSON and CSV formats."""
    rows = [record.as_dict() for record in records]
    json_path, csv_path = report_paths(base_path)
    document = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "mode": mode,
        "summary": summary.as_dict(),
        "records": rows,
    }
    _atomic_write(json_path, json.dumps(document, indent=2, sort_keys=True) + "\n")

    buffer = io.StringIO(newline="")
    writer = csv.DictWriter(buffer, fieldnames=CSV_FIELDS, extrasaction="ignore")
    writer.writeheader()
    writer.writerows(rows)
    _atomic_write(csv_path, buffer.getvalue())
    return json_path, csv_path
