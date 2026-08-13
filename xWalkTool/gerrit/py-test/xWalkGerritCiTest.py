#!/usr/bin/env python3
"""Test owner-aware GitHub destination selection for Gerrit submissions."""

from __future__ import annotations

import io
import pathlib
import sys
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).parents[1] / "py-src"))

from xWalkGerritCi import XWalkGerritCi
from xWalkGerritQuality import XWalkGerritQuality


class XWalkGerritCiTest(unittest.TestCase):
    """Verify Gerrit event selection and GitHub destination selection."""

    def setUp(self) -> None:
        """Create a verifier without initializing host service resources."""

        self.ci = XWalkGerritCi.__new__(XWalkGerritCi)
        self.ci.project = "xWalkPiCarAI"
        self.ci.branch = "master"

    @staticmethod
    def verification_event(
        event_type: str, wip: bool | None
    ) -> dict[str, object]:
        """Return one target-project event with a current patch-set payload."""

        change: dict[str, object] = {
            "project": "xWalkPiCarAI",
            "branch": "master",
        }
        if wip is not None:
            change["wip"] = wip
        return {
            "type": event_type,
            "change": change,
            "patchSet": {
                "number": 3,
                "revision": "revision",
                "ref": "refs/changes/41/41/3",
            },
        }

    def test_active_patch_set_triggers_verification(self) -> None:
        """Run CI automatically when an active patch set is uploaded."""

        event = self.verification_event("patchset-created", False)
        self.assertTrue(self.ci.matching_verification_event(event))

    def test_wip_patch_set_waits_for_activation(self) -> None:
        """Keep a WIP patch-set upload idle until its change becomes active."""

        event = self.verification_event("patchset-created", True)
        self.assertFalse(self.ci.matching_verification_event(event))

    def test_mark_as_active_triggers_verification(self) -> None:
        """Run CI when activation omits Gerrit's false WIP field."""

        event = self.verification_event("wip-state-changed", None)
        self.assertTrue(self.ci.matching_verification_event(event))

    def test_mark_as_wip_does_not_trigger_verification(self) -> None:
        """Do not run CI when an active change is moved into WIP state."""

        event = self.verification_event("wip-state-changed", True)
        self.assertFalse(self.ci.matching_verification_event(event))

    def test_unrelated_event_does_not_trigger_verification(self) -> None:
        """Ignore other event types even when they carry a target patch set."""

        event = self.verification_event("comment-added", False)
        self.assertFalse(self.ci.matching_verification_event(event))

    def test_joxy_change_targets_master(self) -> None:
        """Allow Joxy when GitHub is exactly at the submitted change parent."""

        branch = XWalkGerritCi.select_github_branch(
            "joxjoh24@student.hh.se",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "master")

    def test_joxy_email_comparison_is_case_insensitive(self) -> None:
        """Treat email casing as insignificant for the configured owner."""

        branch = XWalkGerritCi.select_github_branch(
            "JOXJOH24@STUDENT.HH.SE",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "master")

    def test_other_owner_targets_review_branch(self) -> None:
        """Route every non-Joxy owner through a GitHub pull request."""

        branch = XWalkGerritCi.select_github_branch(
            "partner@example.com",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "gerrit-submitted")

    def test_missing_owner_targets_review_branch(self) -> None:
        """Fail safely when Gerrit does not expose an owner email."""

        branch = XWalkGerritCi.select_github_branch(
            "",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "parent",
            "parent",
        )
        self.assertEqual(branch, "gerrit-submitted")

    def test_stacked_change_targets_review_branch(self) -> None:
        """Prevent a later Joxy change from bypassing an earlier pending PR."""

        branch = XWalkGerritCi.select_github_branch(
            "joxjoh24@student.hh.se",
            "joxjoh24@student.hh.se",
            "master",
            "gerrit-submitted",
            "gerrit-parent",
            "github-tip",
        )
        self.assertEqual(branch, "gerrit-submitted")


class XWalkGerritQualityTest(unittest.TestCase):
    """Verify resource-safe Gerrit quality-job scheduling."""

    def test_tsan_runs_without_parallel_build_jobs(self) -> None:
        """Run TSan separately so build contention cannot time out its probe."""

        jobs = [("gcc Debug", lambda: True), ("thread-sanitizer", lambda: True)]
        isolated, parallel = XWalkGerritQuality.partition_jobs(jobs)
        self.assertEqual([name for name, unused_job in isolated], ["thread-sanitizer"])
        self.assertEqual([name for name, unused_job in parallel], ["gcc Debug"])

    def test_tsan_uses_non_root_namespace_with_loopback(self) -> None:
        """Give TSan stable mappings and retain its socket-test coverage."""

        quality = XWalkGerritQuality(pathlib.Path("/workspace"), io.StringIO())
        with mock.patch.object(quality, "run_steps", return_value=True) as run_steps:
            self.assertTrue(quality.thread_sanitizer_job())
        command = run_steps.call_args.args[1][0][0]
        self.assertEqual(command[0], "unshare")
        self.assertIn("--map-root-user", command)
        self.assertIn("--net", command)
        self.assertIn("ip link set lo up", command[-1])

if __name__ == "__main__":
    unittest.main()
