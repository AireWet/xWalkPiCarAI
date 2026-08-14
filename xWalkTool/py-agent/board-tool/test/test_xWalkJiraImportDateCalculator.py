#!/usr/bin/env python3
"""Test xWalk Jira historical dates across Stockholm work periods."""

from __future__ import annotations

import unittest
from datetime import datetime, timezone
from zoneinfo import ZoneInfo

from test_xWalkJiraImportCommitAnalyser import make_commit
from xWalkJiraImport.xWalkJiraImportDateCalculator import (
    calculate_historical_dates,
    calculate_planned_due_date,
    next_workday,
    subtract_work_minutes,
)
from xWalkJiraImport.xWalkJiraImportModels import ChangedFile, EffortEstimate


STOCKHOLM = ZoneInfo("Europe/Stockholm")


def estimate(minutes: int | None, confidence: str = "High") -> EffortEstimate:
    """Create one date-focused estimate fixture."""
    return EffortEstimate(
        "Medium" if minutes is not None else "Too broad",
        "2-4 hours" if minutes is not None else "Requires manual review",
        "4h" if minutes is not None else None,
        minutes,
        3 if minutes is not None else 13,
        confidence,
        "fixture",
        minutes is None,
    )


class HistoricalDateCalculatorTest(unittest.TestCase):
    """Verify eight-hour weekdays, timezone conversion, and honest clamping."""

    def test_subtracts_within_one_workday(self) -> None:
        """Four hours before 16:00 remains on the same weekday."""
        completion = datetime(2026, 8, 7, 16, 0, tzinfo=STOCKHOLM)
        self.assertEqual(subtract_work_minutes(completion, 240).hour, 12)

    def test_crosses_weekend(self) -> None:
        """Monday morning effort continues backward from Friday afternoon."""
        completion = datetime(2026, 8, 10, 10, 0, tzinfo=STOCKHOLM)
        start = subtract_work_minutes(completion, 120)
        self.assertEqual(start.weekday(), 4)
        self.assertEqual((start.hour, start.minute), (16, 0))

    def test_planned_schedule_rounds_to_whole_non_overlapping_workdays(self) -> None:
        """Date-only Jira planning consumes at least one day and skips weekends."""
        start = datetime(2026, 9, 4, tzinfo=STOCKHOLM).date()
        self.assertEqual(calculate_planned_due_date(start, 30), start)
        self.assertEqual(calculate_planned_due_date(start, 960).isoformat(), "2026-09-07")
        self.assertEqual(next_workday(datetime(2026, 9, 4).date()).isoformat(), "2026-09-07")

    def test_uses_author_date_and_stockholm_timezone(self) -> None:
        """The author timestamp is the preferred historical completion date."""
        commit = make_commit(
            "Add camera validation",
            [ChangedFile("camera.py", "modified", 20, 2, 22, patch="+check")],
        )
        dates = calculate_historical_dates(commit, estimate(120))
        self.assertEqual(dates.completion.tzinfo, STOCKHOLM)
        self.assertLessEqual(dates.start, dates.completion)

    def test_clamps_to_previous_relevant_commit_with_low_confidence(self) -> None:
        """An inferred start never silently predates the previous relevant commit."""
        commit = make_commit(
            "Add tests",
            [ChangedFile("test_camera.py", "added", 30, 0, 30, patch="+test")],
        )
        completion = commit.author_date.astimezone(STOCKHOLM)
        previous = completion.replace(hour=13)
        dates = calculate_historical_dates(commit, estimate(480), previous)
        self.assertEqual(dates.start, previous)
        self.assertEqual(dates.confidence, "Low")
        self.assertIn("clamped", dates.note)

    def test_omits_start_for_manual_review(self) -> None:
        """A too-broad commit receives no precise inferred start date."""
        commit = make_commit(
            "Large migration",
            [ChangedFile("migration.py", "modified", 500, 500, 1000, patch="+change")],
        )
        dates = calculate_historical_dates(commit, estimate(None))
        self.assertIsNone(dates.start)
        self.assertEqual(dates.confidence, "Low")


if __name__ == "__main__":
    unittest.main()
