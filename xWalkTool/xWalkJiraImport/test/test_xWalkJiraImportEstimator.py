#!/usr/bin/env python3
"""Test xWalk Jira effort estimation from semantic source changes."""

from __future__ import annotations

import unittest

from test_xWalkJiraImportCommitAnalyser import make_commit
from xWalkJiraImport.xWalkJiraImportCommitAnalyser import analyse_commit
from xWalkJiraImport.xWalkJiraImportEstimator import estimate_effort
from xWalkJiraImport.xWalkJiraImportModels import ChangedFile


class EffortEstimatorTest(unittest.TestCase):
    """Verify the baseline levels and conservative manual-review boundary."""

    def test_control_correction_includes_human_hardware_validation(self) -> None:
        """A small steering correction includes human investigation and hardware validation."""
        commit = make_commit(
            "Correct steering bound",
            [ChangedFile("src/steering.py", "modified", 3, 2, 5, patch="+clamp")],
        )
        estimate = estimate_effort(commit, analyse_commit(commit))
        self.assertEqual(estimate.level, "Medium")
        self.assertEqual(estimate.jira_time, "4h")
        self.assertEqual(estimate.story_points, 3)
        self.assertIn("Human-development estimate", estimate.rationale)
        self.assertIn("investigation or design", estimate.rationale)

    def test_trivial_documentation_change_retains_baseline(self) -> None:
        """A genuinely tiny non-code change can still use the trivial baseline."""
        commit = make_commit(
            "Clarify camera setup note",
            [ChangedFile("README.md", "modified", 3, 1, 4, patch="+note")],
        )
        estimate = estimate_effort(commit, analyse_commit(commit))
        self.assertEqual(estimate.level, "Trivial")
        self.assertEqual(estimate.jira_time, "30m")

    def test_hardware_integration_increases_semantic_level(self) -> None:
        """Multi-file hardware integration receives a complexity adjustment."""
        files = [
            ChangedFile("src/hal/i2c.cpp", "modified", 25, 5, 30, patch="+transaction"),
            ChangedFile("src/hal/gpio.cpp", "modified", 20, 4, 24, patch="+line"),
        ]
        commit = make_commit("Add hardware transaction validation", files)
        estimate = estimate_effort(commit, analyse_commit(commit))
        self.assertEqual(estimate.level, "Medium")
        self.assertEqual(estimate.story_points, 3)

    def test_test_and_source_change_is_not_underestimated(self) -> None:
        """A tested implementation is at least Medium even with few lines."""
        files = [
            ChangedFile("src/deploy.py", "modified", 10, 2, 12, patch="+validate"),
            ChangedFile("test/test_deploy.py", "added", 15, 0, 15, patch="+test"),
        ]
        commit = make_commit("Add deployment validation tests", files)
        estimate = estimate_effort(commit, analyse_commit(commit))
        self.assertGreaterEqual(estimate.story_points, 3)

    def test_generated_volume_does_not_inflate_estimate(self) -> None:
        """A large generated file is removed before estimating semantic effort."""
        files = [
            ChangedFile("src/camera.py", "modified", 4, 2, 6, patch="+retry"),
            ChangedFile("build/generated.cpp", "added", 5000, 0, 5000),
        ]
        commit = make_commit("Update camera timeout", files)
        analysis = analyse_commit(commit)
        estimate = estimate_effort(commit, analysis)
        self.assertEqual(len(analysis.meaningful_files), 1)
        self.assertEqual(estimate.level, "Medium")

    def test_unusually_broad_commit_requires_manual_review(self) -> None:
        """A broad semantic commit receives no invented Jira time estimate."""
        files = [
            ChangedFile(f"src/module_{index}.py", "modified", 50, 20, 70, patch="+change")
            for index in range(31)
        ]
        commit = make_commit("Restructure application", files)
        estimate = estimate_effort(commit, analyse_commit(commit))
        self.assertTrue(estimate.manual_review)
        self.assertIsNone(estimate.jira_time)
        self.assertEqual(estimate.story_points, 13)
        self.assertEqual(estimate.confidence, "Low")


if __name__ == "__main__":
    unittest.main()
