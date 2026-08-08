"""Estimate human engineering effort from semantic changes and integration risk."""

from __future__ import annotations

from .xWalkJiraImportModels import CommitAnalysis, CommitRecord, EffortEstimate


LEVELS = (
    ("Trivial", "15-30 minutes", "30m", 30, 1),
    ("Small", "30 minutes-2 hours", "2h", 120, 2),
    ("Medium", "2-4 hours", "4h", 240, 3),
    ("Large", "4-8 hours", "1d", 480, 5),
    ("Very large", "1-3 working days", "2d", 960, 8),
)

SOURCE_SUFFIXES = (".c", ".cc", ".cpp", ".h", ".hpp", ".py", ".sh", ".cmake")
COMPLEX_COMPONENTS = {
    "YOLO or AI model",
    "Camera",
    "Computer vision",
    "Hardware abstraction",
    "Motor or steering control",
    "Raspberry Pi 5",
    "Sensors",
    "V2X",
}


def estimate_effort(commit: CommitRecord, analysis: CommitAnalysis) -> EffortEstimate:
    """Estimate a human developer's complete engineering effort for one commit."""
    files = analysis.meaningful_files
    semantic_lines = sum(item.changes for item in files)
    test_files = sum("test" in item.filename.casefold() for item in files)
    source_files = sum(item.filename.casefold().endswith(SOURCE_SUFFIXES) for item in files)
    implementation_files = sum(
        item.filename.casefold().endswith(SOURCE_SUFFIXES) and "test" not in item.filename.casefold()
        for item in files
    )
    new_implementation_files = sum(
        item.status == "added"
        and item.filename.casefold().endswith(SOURCE_SUFFIXES)
        and "test" not in item.filename.casefold()
        for item in files
    )
    affected_areas = {
        item.filename.split("/", maxsplit=1)[0].casefold()
        for item in files
        if item.filename
    }
    if analysis.manual_review or len(files) > 30 or semantic_lines > 2500:
        return EffortEstimate(
            "Too broad",
            "Requires manual review",
            None,
            None,
            13,
            "Low",
            "A human estimate cannot be defended from this commit alone because it spans too many semantic "
            "files, changed lines, or unrelated components. Manual review must separate investigation, design, "
            "implementation, integration, testing, and review work.",
            True,
        )

    if implementation_files == 0 and len(files) == 1 and semantic_lines <= 20:
        index = 0
    elif len(files) <= 2 and semantic_lines <= 80:
        index = 1
    elif len(files) <= 6 and semantic_lines <= 350:
        index = 2
    elif len(files) <= 15 and semantic_lines <= 1000:
        index = 3
    else:
        index = 4

    if implementation_files and index == 0:
        index = 1
    if analysis.issue_type == "Bug":
        index = max(index, 1)
    if analysis.issue_type == "Story":
        index = max(index, 2)
    if analysis.component in COMPLEX_COMPONENTS and implementation_files:
        index = max(index, 2)
    if test_files and implementation_files:
        index = min(index + 1, len(LEVELS) - 1)
    if new_implementation_files >= 3 or len(affected_areas) >= 3:
        index = min(index + 1, len(LEVELS) - 1)

    level, display, jira_time, minutes, points = LEVELS[index]
    confidence = "High"
    if analysis.unrelated_change_groups > 1 or any(item.patch is None for item in files):
        confidence = "Medium"
    if analysis.unrelated_change_groups >= 3:
        confidence = "Low"
    rationale = (
        "Human-development estimate includes repository and requirements review, investigation or design, "
        "implementation, integration, local verification, regression testing, and normal code-review rework. "
        f"Evidence includes {len(files)} semantic files, {semantic_lines} non-generated changed lines, "
        f"{source_files} source files, {implementation_files} implementation files, {test_files} test-related "
        f"files, and {len(affected_areas)} affected repository areas."
    )
    return EffortEstimate(level, display, jira_time, minutes, points, confidence, rationale)
