"""Infer Stockholm working dates for historical xWalk changes."""

from __future__ import annotations

from datetime import date, datetime, time, timedelta
from zoneinfo import ZoneInfo

from .xWalkJiraImportModels import CommitRecord, EffortEstimate, HistoricalDates


STOCKHOLM = ZoneInfo("Europe/Stockholm")
WORK_START = time(9, 0)
WORK_END = time(17, 0)
WORKDAY_MINUTES = 480


def next_workday(value: date) -> date:
    """Return the first Monday-through-Friday date after one schedule date."""
    candidate = value + timedelta(days=1)
    while candidate.weekday() >= 5:
        candidate += timedelta(days=1)
    return candidate


def calculate_planned_due_date(start: date, effort_minutes: int) -> date:
    """Round effort up to whole Jira work dates and return the inclusive due date."""
    if effort_minutes <= 0:
        raise ValueError("Planned effort minutes must be greater than zero.")
    workdays = max((effort_minutes + WORKDAY_MINUTES - 1) // WORKDAY_MINUTES, 1)
    due = start
    remaining = workdays - 1
    while remaining > 0:
        due = next_workday(due)
        remaining -= 1
    return due


def completion_timestamp(commit: CommitRecord) -> datetime:
    """Use a valid author date, falling back to the committer date."""
    value = commit.author_date or commit.commit_date
    if value is None or value.tzinfo is None:
        raise ValueError(f"Commit {commit.short_sha} has no valid completion timestamp.")
    return value.astimezone(STOCKHOLM)


def _previous_workday(value: datetime) -> datetime:
    """Return the end of the previous Monday-through-Friday workday."""
    candidate = value - timedelta(days=1)
    while candidate.weekday() >= 5:
        candidate -= timedelta(days=1)
    return candidate.replace(hour=WORK_END.hour, minute=WORK_END.minute, second=0, microsecond=0)


def _latest_work_instant(value: datetime) -> datetime:
    """Clamp a timestamp backward into the nearest valid working period."""
    local = value.astimezone(STOCKHOLM)
    if local.weekday() >= 5:
        return _previous_workday(local)
    if local.timetz().replace(tzinfo=None) > WORK_END:
        return local.replace(hour=WORK_END.hour, minute=0, second=0, microsecond=0)
    if local.timetz().replace(tzinfo=None) < WORK_START:
        return _previous_workday(local)
    return local


def subtract_work_minutes(completion: datetime, minutes: int) -> datetime:
    """Subtract positive effort through eight-hour weekday working periods."""
    if minutes < 0:
        raise ValueError("Effort minutes must not be negative.")
    current = _latest_work_instant(completion)
    remaining = minutes
    while remaining > 0:
        start = current.replace(hour=WORK_START.hour, minute=0, second=0, microsecond=0)
        available = max(int((current - start).total_seconds() // 60), 0)
        if remaining <= available:
            return current - timedelta(minutes=remaining)
        remaining -= available
        current = _previous_workday(current)
    return current


def calculate_historical_dates(
    commit: CommitRecord,
    estimate: EffortEstimate,
    previous_relevant: datetime | None = None,
) -> HistoricalDates:
    """Infer start/completion dates and disclose clamping or ambiguity."""
    completion = completion_timestamp(commit)
    if estimate.effort_minutes is None:
        return HistoricalDates(
            None,
            completion,
            "Low",
            "Start date omitted because the commit requires manual effort review.",
        )
    start = subtract_work_minutes(completion, estimate.effort_minutes)
    note = "Start date was inferred by subtracting estimated effort across 09:00-17:00 weekdays."
    confidence = estimate.confidence
    if previous_relevant is not None:
        lower_bound = previous_relevant.astimezone(STOCKHOLM)
        if start < lower_bound <= completion:
            start = lower_bound
            note += " It was clamped to the previous relevant commit and remains estimated."
            confidence = "Low"
    if start > completion:
        raise ValueError("Historical start date cannot be later than completion.")
    return HistoricalDates(start, completion, confidence, note)
