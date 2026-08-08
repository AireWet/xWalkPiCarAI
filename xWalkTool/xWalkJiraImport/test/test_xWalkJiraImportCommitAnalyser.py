#!/usr/bin/env python3
"""Test xWalk Jira import commit analysis and Jira type selection."""

from __future__ import annotations

import unittest
from datetime import datetime, timezone

from xWalkJiraImport.xWalkJiraImportCommitAnalyser import analyse_commit, is_generated_or_non_effort_file
from xWalkJiraImport.xWalkJiraImportModels import ChangedFile, CommitRecord


def make_commit(title: str, files: list[ChangedFile], body: str = "") -> CommitRecord:
    """Create one deterministic commit fixture."""
    timestamp = datetime(2026, 5, 4, 12, 0, tzinfo=timezone.utc)
    return CommitRecord(
        sha="a" * 40,
        short_sha="a" * 7,
        title=title,
        body=body,
        author_name="Thesis Author",
        author_email="author@example.invalid",
        committer_name="Thesis Author",
        committer_email="author@example.invalid",
        author_date=timestamp,
        commit_date=timestamp,
        url="https://github.com/jochuuu/xWalkPiCarAI/commit/" + "a" * 40,
        parents=("b" * 40,),
        files=tuple(files),
        additions=sum(item.additions for item in files),
        deletions=sum(item.deletions for item in files),
        total_changes=sum(item.changes for item in files),
    )


class CommitAnalyserTest(unittest.TestCase):
    """Verify meaningful changes receive stable Jira-facing classifications."""

    def test_classifies_yolo_story(self) -> None:
        """A complete detection capability is a YOLO Story request."""
        commit = make_commit(
            "feat: add vulnerable road-user detection pipeline",
            [ChangedFile("src/yolo/vru_detector.py", "added", 90, 0, 90, patch="+detect")],
        )
        analysis = analyse_commit(commit)
        self.assertTrue(analysis.accepted)
        self.assertEqual(analysis.component, "YOLO or AI model")
        self.assertEqual(analysis.issue_type, "Story")
        self.assertEqual(analysis.summary, "[YOLO] Add vulnerable road-user detection pipeline")
        self.assertIn("yolo", analysis.labels)

    def test_classifies_camera_failure_as_bug(self) -> None:
        """A reconnection failure correction becomes a Camera Bug."""
        commit = make_commit(
            "Correct USB camera reconnection failure",
            [ChangedFile("src/camera/usb_camera.cpp", "modified", 22, 9, 31, patch="+retry")],
        )
        analysis = analyse_commit(commit)
        self.assertEqual(analysis.component, "Camera")
        self.assertEqual(analysis.issue_type, "Bug")
        self.assertTrue(analysis.summary.startswith("[Camera] Fix "))

    def test_implementation_without_user_capability_is_task(self) -> None:
        """Internal CMake implementation remains a Task."""
        commit = make_commit(
            "Add Raspberry Pi dependency validation",
            [ChangedFile("cmake/Dependencies.cmake", "modified", 30, 2, 32, patch="+find_package")],
        )
        analysis = analyse_commit(commit)
        self.assertEqual(analysis.component, "Build or CMake")
        self.assertEqual(analysis.issue_type, "Task")

    def test_excludes_generated_models_and_lock_files(self) -> None:
        """Generated outputs, model weights, and locks never count as semantic effort."""
        excluded = (
            ChangedFile("build/generated.pb.cc", "added", 1000, 0, 1000),
            ChangedFile("models/vru.weights", "added", 0, 0, 0),
            ChangedFile("package-lock.json", "modified", 200, 200, 400),
            ChangedFile("datasets/raw/frame001.jpg", "added", 0, 0, 0),
        )
        self.assertTrue(all(is_generated_or_non_effort_file(item) for item in excluded))
        analysis = analyse_commit(make_commit("Add model artifacts", list(excluded)))
        self.assertFalse(analysis.accepted)
        self.assertIn("non-effort", analysis.reason)

    def test_ignores_formatting_unless_explicitly_included(self) -> None:
        """Formatting-only commits require the explicit inclusion option."""
        commit = make_commit(
            "Apply formatting",
            [ChangedFile("src/control.py", "modified", 10, 10, 20, patch=" whitespace")],
        )
        self.assertFalse(analyse_commit(commit).accepted)
        self.assertTrue(analyse_commit(commit, include_insignificant=True).accepted)

    def test_uses_diff_evidence_for_vague_title(self) -> None:
        """A vague commit title becomes a component-specific generated summary."""
        commit = make_commit(
            "update",
            [ChangedFile("test/test_motor_control.py", "modified", 15, 2, 17, patch="+assert")],
        )
        analysis = analyse_commit(commit)
        self.assertEqual(analysis.component, "Testing")
        self.assertEqual(analysis.summary, "[Testing] Update motor control test coverage")

    def test_uses_detailed_commit_body_for_vague_title(self) -> None:
        """A useful body statement takes priority over a generic commit title."""
        commit = make_commit(
            "changes",
            [ChangedFile("src/camera/reconnect.cpp", "modified", 15, 3, 18, patch="+retry")],
            body="Prevent stale camera handles after a USB reconnect.",
        )
        analysis = analyse_commit(commit)
        self.assertIn("Prevent stale camera handles", analysis.summary)

    def test_expands_short_summary_from_changed_file_scope(self) -> None:
        """A short title is expanded with concrete semantic file evidence."""
        commit = make_commit(
            "Add retry",
            [ChangedFile("src/camera/frame_recovery.cpp", "added", 20, 0, 20, patch="+recover")],
        )
        analysis = analyse_commit(commit)
        self.assertEqual(analysis.summary, "[Camera] Add frame recovery")


if __name__ == "__main__":
    unittest.main()
