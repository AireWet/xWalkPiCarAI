#!/usr/bin/env python3
"""Append sanitized Gerrit operation records to locked CSV and Markdown logs."""

from __future__ import annotations

import argparse
import csv
from datetime import datetime, timezone
import fcntl
import os
from pathlib import Path
import re
import sys
import uuid


COLUMNS = (
    "timestamp_utc", "operation_id", "operation", "category", "repository", "target",
    "previous_value", "new_value", "change_summary", "change_explanation", "requested_by",
    "executed_by", "mode", "status", "verification", "error_summary", "related_change",
    "commit_before", "commit_after",
)
STATUSES = {"Planned", "Skipped", "Applied", "Verified", "Failed", "RolledBack"}
CATEGORIES = {
    "Provisioning", "ACL", "migration", "submodule", "CI", "uplift", "GitHub",
    "validation", "rollback",
}
SECRET_PATTERN = re.compile(
    r"(?i)(password|token|cookie|authorization|private[-_ ]?key|secret)\s*[:=]\s*\S+"
)
AUTHENTICATED_URL = re.compile(r"([a-z][a-z0-9+.-]*://)[^/@\s]+@", re.IGNORECASE)


def clean(value: object) -> str:
    """Return one single-line value with credentials and control text removed."""

    text = " ".join(str(value or "").replace("\0", "").splitlines()).strip()
    text = SECRET_PATTERN.sub(lambda match: f"{match.group(1)}=[REDACTED]", text)
    return AUTHENTICATED_URL.sub(r"\1[IDENTITY]@", text)


def operation_id(operation: str) -> str:
    """Generate a readable unique operation identifier."""

    prefix = re.sub(r"[^a-z0-9]+", "-", operation.casefold()).strip("-")[:24] or "operation"
    instant = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    return f"{prefix}-{instant}-{uuid.uuid4().hex[:8]}"


def runtime_directory() -> Path:
    """Return the protected configured or per-user runtime log directory."""

    configured = os.environ.get("XWALK_CHANGE_LOG_DIR")
    return Path(configured).expanduser() if configured else Path.home() / ".local/state/xwalk-gerrit"


def markdown_value(value: str) -> str:
    """Escape one sanitized Markdown table cell."""

    return value.replace("\\", "\\\\").replace("|", "\\|").replace("`", "\\`")


class XWalkChangeLog:
    """Own append-only, process-locked CSV and Markdown operation reports."""

    def __init__(self, directory: Path | None = None) -> None:
        """Select the runtime directory without creating it until append time."""

        self.directory = (directory or runtime_directory()).resolve()
        self.csv_path = self.directory / "change-log.csv"
        self.markdown_path = self.directory / "change-log.md"
        self.lock_path = self.directory / ".change-log.lock"

    def append(self, record: dict[str, object]) -> dict[str, str]:
        """Validate, sanitize, lock, and append one operation to both reports."""

        row = {column: clean(record.get(column, "")) for column in COLUMNS}
        row["timestamp_utc"] = row["timestamp_utc"] or datetime.now(timezone.utc).isoformat().replace(
            "+00:00", "Z"
        )
        row["operation_id"] = row["operation_id"] or operation_id(row["operation"])
        if row["status"] not in STATUSES:
            raise ValueError(f"Unsupported change-log status: {row['status']}")
        if row["category"] not in CATEGORIES:
            raise ValueError(f"Unsupported change-log category: {row['category']}")
        if row["mode"] not in {"dry-run", "apply"}:
            raise ValueError(f"Unsupported change-log mode: {row['mode']}")
        self.directory.mkdir(parents=True, exist_ok=True, mode=0o700)
        self.directory.chmod(0o700)
        with self.lock_path.open("a+", encoding="utf-8") as lock:
            os.fchmod(lock.fileno(), 0o600)
            fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
            self._append_csv(row)
            self._append_markdown(row)
            fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
        return row

    def _append_csv(self, row: dict[str, str]) -> None:
        """Append one RFC-compatible CSV record and create its header once."""

        new_file = not self.csv_path.exists() or self.csv_path.stat().st_size == 0
        with self.csv_path.open("a", encoding="utf-8", newline="") as output:
            os.fchmod(output.fileno(), 0o600)
            writer = csv.DictWriter(output, fieldnames=COLUMNS)
            if new_file:
                writer.writeheader()
            writer.writerow(row)

    def _append_markdown(self, row: dict[str, str]) -> None:
        """Append one human-readable operation row and create its heading once."""

        new_file = not self.markdown_path.exists() or self.markdown_path.stat().st_size == 0
        with self.markdown_path.open("a", encoding="utf-8") as output:
            os.fchmod(output.fileno(), 0o600)
            if new_file:
                output.write(
                    "# Gerrit operation change log\n\n"
                    "| Time | ID | Operation | Repository | Change | Explanation | Mode | Status | "
                    "Verification |\n"
                    "|---|---|---|---|---|---|---|---|---|\n"
                )
            values = (
                row["timestamp_utc"], row["operation_id"], row["operation"], row["repository"],
                f"{row['previous_value']} → {row['new_value']}", row["change_explanation"],
                row["mode"], row["status"], row["verification"] or row["error_summary"],
            )
            output.write("| " + " | ".join(markdown_value(value) for value in values) + " |\n")


def parser() -> argparse.ArgumentParser:
    """Build the command-line logger used by shell and Python operations."""

    argument_parser = argparse.ArgumentParser(description=__doc__)
    for column in COLUMNS:
        required = column in {"operation", "category", "mode", "status"}
        argument_parser.add_argument(f"--{column.replace('_', '-')}", required=required)
    return argument_parser


def main() -> int:
    """Append one mandatory operation row or fail nonzero."""

    try:
        XWalkChangeLog().append(vars(parser().parse_args()))
    except (OSError, ValueError) as error:
        print(f"XWALK_CHANGE_LOG: FAILED - {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
