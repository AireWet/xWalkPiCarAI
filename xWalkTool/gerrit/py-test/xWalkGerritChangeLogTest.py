#!/usr/bin/env python3
"""Test locked, sanitized Gerrit operation change logs."""

from __future__ import annotations

import csv
import pathlib
import sys
import tempfile
import threading
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).parents[1] / "py-src"))

from xWalkGerritChangeLog import COLUMNS, XWalkChangeLog


def record(operation: str, status: str = "Applied") -> dict[str, object]:
    """Return one complete test operation record."""

    return {
        "operation": operation,
        "category": "validation",
        "repository": "xWalkHal",
        "target": "refs/heads/main",
        "previous_value": "old",
        "new_value": "new",
        "change_summary": "Validated one operation",
        "change_explanation": "The operation requires an independent audit row.",
        "requested_by": "user",
        "executed_by": "automation",
        "mode": "apply",
        "status": status,
        "verification": "line one\nline two | complete",
        "error_summary": "",
    }


class XWalkGerritChangeLogTest(unittest.TestCase):
    """Verify schema, escaping, sanitization, and concurrent append behavior."""

    def test_append_creates_both_reports(self) -> None:
        """Create matching machine-readable and human-readable entries."""

        with tempfile.TemporaryDirectory() as directory:
            change_log = XWalkChangeLog(pathlib.Path(directory))
            appended = change_log.append(record("validate-acl"))
            with change_log.csv_path.open(encoding="utf-8", newline="") as source:
                rows = list(csv.DictReader(source))
            markdown = change_log.markdown_path.read_text(encoding="utf-8")
        self.assertEqual(tuple(rows[0]), COLUMNS)
        self.assertEqual(rows[0]["operation_id"], appended["operation_id"])
        self.assertIn("line one line two \\| complete", markdown)

    def test_sensitive_values_are_redacted(self) -> None:
        """Remove credentials and authenticated URL identities before writing."""

        with tempfile.TemporaryDirectory() as directory:
            change_log = XWalkChangeLog(pathlib.Path(directory))
            sensitive = record("sanitize")
            sensitive["new_value"] = "token=do-not-write ssh://person@example.test/project"
            change_log.append(sensitive)
            content = change_log.csv_path.read_text(encoding="utf-8")
        self.assertNotIn("do-not-write", content)
        self.assertNotIn("person@", content)
        self.assertIn("[REDACTED]", content)

    def test_concurrent_entries_keep_unique_rows(self) -> None:
        """Serialize simultaneous writers without losing or merging records."""

        with tempfile.TemporaryDirectory() as directory:
            change_log = XWalkChangeLog(pathlib.Path(directory))
            threads = [
                threading.Thread(target=change_log.append, args=(record(f"operation-{index}"),))
                for index in range(8)
            ]
            for thread in threads:
                thread.start()
            for thread in threads:
                thread.join()
            with change_log.csv_path.open(encoding="utf-8", newline="") as source:
                rows = list(csv.DictReader(source))
        self.assertEqual(len(rows), 8)
        self.assertEqual(len({row["operation_id"] for row in rows}), 8)

    def test_invalid_status_is_rejected(self) -> None:
        """Fail closed when a caller uses a status outside the schema."""

        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(ValueError):
                XWalkChangeLog(pathlib.Path(directory)).append(record("invalid", "Complete"))


if __name__ == "__main__":
    unittest.main()
