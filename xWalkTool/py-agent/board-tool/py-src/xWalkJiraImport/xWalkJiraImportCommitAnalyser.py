"""Classify meaningful xWalk changes and generate Jira summaries."""

from __future__ import annotations

import re
from pathlib import PurePosixPath

from .xWalkJiraImportModels import ChangedFile, CommitAnalysis, CommitRecord


COMPONENT_RULES: tuple[tuple[str, tuple[str, ...]], ...] = (
    ("Security", ("security", "credential", "token", "secret", "license")),
    ("CI/CD", (".github/workflows", "github-actions", "pipeline", "ci/")),
    ("Testing", ("/test", "test_", "tests/", "unittest", "gtest", "coverage")),
    ("Deployment", ("deploy", "systemd", "udev", "install", "package", "provision")),
    ("Build or CMake", ("cmake", "cmakelists", "toolchain", "compiler", "build")),
    ("Configuration", ("config", ".yaml", ".yml", ".json", ".xml", ".conf")),
    ("Documentation", ("readme", "developer", "doc/", ".md", ".rst", "thesis")),
    ("Refactoring", ("refactor", "reorgan", "rename", "cleanup")),
    ("YOLO or AI model", ("yolo", "onnx", "model", "inference", "neural", "detect")),
    ("Dataset or preprocessing", ("dataset", "preprocess", "augment", "annotation", "labelmap")),
    ("Computer vision", ("opencv", "vision", "tracking", "vru", "image")),
    ("Camera", ("camera", "v4l2", "libcamera", "capture", "webcam")),
    ("Raspberry Pi 5", ("/rpi5/", "rpi5/", "raspberry pi", "raspberry-pi", "aarch64", "bookworm")),
    ("Motor or steering control", ("motor", "steering", "servo", "pwm", "drive")),
    ("Sensors", ("sensor", "ultrasonic", "grayscale", "adc", "imu", "adxl")),
    ("V2X", ("v2x", "vehicle-to", "grpc", "protobuf", "mqtt", "spi")),
    ("Hardware abstraction", ("hal", "gpio", "i2c", "robot-hat", "hardware")),
)

DISPLAY_COMPONENTS = {
    "YOLO or AI model": "YOLO",
    "Dataset or preprocessing": "Dataset",
    "Build or CMake": "Build",
    "CI/CD": "CD/CI",
    "Configuration": "Config",
    "Hardware abstraction": "HW",
    "Raspberry Pi 5": "Raspberry Pi 5",
    "V2X": "V2X",
}

EXCLUDED_DIRECTORY_PARTS = {
    ".git",
    "auto-gen",
    "build",
    "cmakefiles",
    "dist",
    "generated",
    "node_modules",
    "third_party",
    "third-party",
    "vendor",
}
EXCLUDED_SUFFIXES = {
    ".7z",
    ".bin",
    ".bmp",
    ".class",
    ".csv",
    ".deb",
    ".dll",
    ".gif",
    ".gz",
    ".jpeg",
    ".jpg",
    ".lock",
    ".mp3",
    ".mp4",
    ".npy",
    ".onnx",
    ".pdf",
    ".png",
    ".pt",
    ".pth",
    ".rpm",
    ".so",
    ".tar",
    ".tflite",
    ".wav",
    ".weights",
    ".zip",
}
LOCK_NAMES = {"package-lock.json", "pnpm-lock.yaml", "poetry.lock", "cargo.lock", "pipfile.lock"}


def is_generated_or_non_effort_file(file: ChangedFile) -> bool:
    """Identify generated, binary, dataset, lock, build, and third-party content."""
    path = PurePosixPath(file.filename)
    lowered_parts = {part.casefold() for part in path.parts}
    lowered = file.filename.casefold()
    if lowered_parts & EXCLUDED_DIRECTORY_PARTS:
        return True
    if path.name.casefold() in LOCK_NAMES or path.suffix.casefold() in EXCLUDED_SUFFIXES:
        return True
    if any(token in lowered for token in ("/datasets/", "/data/raw/", "/models/weights/")):
        return True
    if any(token in lowered for token in (".generated.", "_pb2.py", ".pb.cc", ".pb.h")):
        return True
    return False


def _component_for_text(value: str) -> str:
    """Return the first strongly matching component for path or message text."""
    lowered = value.casefold()
    for component, terms in COMPONENT_RULES:
        if any(term in lowered for term in terms):
            return component
    return "Other"


def _clean_title(title: str) -> str:
    """Remove conventional prefixes and repository-specific bracket tags."""
    value = re.sub(r"^(?:\[[^]]+\]\s*)+", "", title.strip())
    value = re.sub(r"^(?:feat|fix|bugfix|docs|test|refactor|build|ci|chore)(?:\([^)]*\))?!?:\s*", "", value,
                   flags=re.IGNORECASE)
    value = re.sub(r"\s+", " ", value).strip(" .:-")
    return value


def _is_vague_title(value: str) -> bool:
    """Reject titles that cannot stand as a traceable work-item summary."""
    normalized = re.sub(r"[^a-z]+", " ", value.casefold()).strip()
    words = normalized.split()
    generic = normalized in {"change", "changes", "fix", "misc", "update", "updates", "work"}
    return generic or len(value) < 8 or len(words) < 3


def _issue_type(commit: CommitRecord, component: str) -> str:
    """Choose Bug, Task, or a clearly supported user-facing Story request."""
    message = f"{commit.title}\n{commit.body}".casefold()
    bug_terms = (
        "bug",
        "crash",
        "error",
        "failure",
        "fix",
        "incorrect",
        "regression",
        "security",
    )
    if component == "Security" or any(re.search(rf"\b{term}\w*\b", message) for term in bug_terms):
        return "Bug"
    user_capability = re.search(r"\b(add|implement|introduce|enable)\b", message)
    user_surface = re.search(r"\b(user|driver|detection|control|dashboard|application|camera)\b", message)
    return "Story" if user_capability and user_surface else "Task"


def _testing_evidence(commit: CommitRecord, meaningful: tuple[ChangedFile, ...]) -> tuple[str, ...]:
    """Extract conservative test evidence from paths and commit-message lines."""
    evidence: list[str] = []
    test_paths = [item.filename for item in meaningful if _component_for_text(item.filename) == "Testing"]
    if test_paths:
        evidence.append(f"Test-related files changed: {', '.join(test_paths[:5])}")
    for line in commit.body.splitlines():
        cleaned = line.strip().lstrip("- ")
        if cleaned and re.search(r"\b(test|ctest|pytest|coverage|verified|passes?)\b", cleaned, re.IGNORECASE):
            evidence.append(cleaned)
    return tuple(dict.fromkeys(evidence))


def _path_scope(filename: str) -> str:
    """Create a readable Jira scope from one meaningful changed path."""
    stem = PurePosixPath(filename).stem
    stem = re.sub(r"^(?:test_|xwalk_|xhal_rpi5car)", "", stem, flags=re.IGNORECASE)
    words = re.sub(r"[^A-Za-z0-9]+", " ", stem).strip().casefold()
    return words or "implementation"


def _generated_summary(
    commit: CommitRecord,
    component: str,
    issue_type: str,
    meaningful: tuple[ChangedFile, ...],
) -> str:
    """Generate a clear component-prefixed summary from message and diff evidence."""
    cleaned = _clean_title(commit.title)
    message = f"{commit.title} {commit.body}".casefold()
    if "controller" in message and "agent" in message and "architecture" in message:
        display_component = "HW"
    elif component == "Testing" and "host" in message:
        display_component = "Host"
    elif component == "Testing":
        display_component = "HOST-TEST"
    elif component == "Other" and "host" in message:
        display_component = "Host"
    elif component == "Other" and any(value in message for value in ("agent", "controller", "architecture")):
        display_component = "HW"
    else:
        display_component = DISPLAY_COMPONENTS.get(component, component)
    if _is_vague_title(cleaned):
        body_candidates = [_clean_title(line.lstrip("- ")) for line in commit.body.splitlines() if line.strip()]
        body_detail = next((value for value in body_candidates if not _is_vague_title(value)), "")
        statuses = {item.status for item in meaningful}
        if issue_type == "Bug":
            action = "Fix"
        elif statuses == {"added"}:
            action = "Add"
        elif statuses == {"removed"}:
            action = "Remove"
        else:
            action = "Update"
        scopes = tuple(
            dict.fromkeys(
                _path_scope(item.filename)
                for item in sorted(meaningful, key=lambda value: value.changes, reverse=True)
            )
        )
        detail = body_detail
        if not detail:
            detail = " and ".join(scopes[:2])
            if component == "Testing":
                detail += " test coverage"
            elif detail == "implementation":
                detail = f"{display_component.lower()} implementation"
        cleaned = f"{action} {detail}"
    elif issue_type == "Bug":
        if re.match(r"^(correct|resolve)\b", cleaned, re.IGNORECASE):
            cleaned = re.sub(r"^(correct|resolve)\b", "Fix", cleaned, flags=re.IGNORECASE)
        elif not re.match(r"^(fix|prevent)\b", cleaned, re.IGNORECASE):
            cleaned = f"Fix {cleaned[0].lower() + cleaned[1:]}"
    cleaned = cleaned[0].upper() + cleaned[1:] if cleaned else "Update implementation"
    return f"[{display_component}] {cleaned}"[:255]


def analyse_commit(commit: CommitRecord, include_insignificant: bool = False) -> CommitAnalysis:
    """Accept and classify one commit using message and semantic file evidence."""
    meaningful = tuple(item for item in commit.files if not is_generated_or_non_effort_file(item))
    message = f"{commit.title}\n{commit.body}".casefold()
    automated = any(term in message for term in ("dependabot", "renovate bot", "automated dependency"))
    automated = automated or "dependabot" in commit.author_name.casefold()
    if automated:
        return CommitAnalysis(False, "Other", "Task", "", (), meaningful, (), "automated dependency commit")
    if not commit.files or not meaningful:
        return CommitAnalysis(False, "Other", "Task", "", (), meaningful, (), "empty or non-effort-only commit")

    formatting = bool(re.search(r"\b(format|formatting|whitespace|style-only)\b", message))
    typo = bool(re.search(r"\b(typo|spelling)\b", message)) and sum(item.changes for item in meaningful) <= 10
    if not include_insignificant and (formatting or typo):
        reason = "formatting-only commit" if formatting else "insignificant typo-only commit"
        return CommitAnalysis(False, "Other", "Task", "", (), meaningful, (), reason)

    component_scores: dict[str, int] = {}
    for item in meaningful:
        component = _component_for_text(item.filename)
        component_scores[component] = component_scores.get(component, 0) + max(item.changes, 1)
    message_component = _component_for_text(f"{commit.title} {commit.body}")
    if message_component != "Other":
        component_scores[message_component] = component_scores.get(message_component, 0) + 5
    component = max(component_scores, key=component_scores.get) if component_scores else "Other"
    issue_type = _issue_type(commit, component)
    groups = len({value for value in component_scores if value != "Other"}) or 1
    manual_review = groups >= 4
    labels = {"github-history-import", "xwalk-picar-ai", "thesis", "completed-work"}
    label_mapping = {
        "YOLO or AI model": "yolo",
        "Raspberry Pi 5": "rpi5",
        "Camera": "camera",
        "Hardware abstraction": "hardware",
        "Motor or steering control": "hardware",
        "Sensors": "hardware",
        "Testing": "testing",
        "Deployment": "deployment",
        "Documentation": "documentation",
        "Security": "security",
    }
    if component in label_mapping:
        labels.add(label_mapping[component])
    return CommitAnalysis(
        accepted=True,
        component=component,
        issue_type=issue_type,
        summary=_generated_summary(commit, component, issue_type, meaningful),
        labels=tuple(sorted(labels)),
        meaningful_files=meaningful,
        testing_evidence=_testing_evidence(commit, meaningful),
        manual_review=manual_review,
        unrelated_change_groups=groups,
        reason=(
            "commit spans several unrelated component groups"
            if manual_review
            else "meaningful semantic change"
        ),
    )
