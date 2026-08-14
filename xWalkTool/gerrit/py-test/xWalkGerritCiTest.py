#!/usr/bin/env python3
"""Test Gerrit event selection and guarded MyPiCarX synchronization."""

from __future__ import annotations

import io
import json
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
        self.ci.project = "MyPiCarX"
        self.ci.branch = "main"
        self.ci.github_remote = "git@github.com:example/MyPiCarX.git"
        self.ci.github_branch = "main"
        self.ci.github_push_enabled = True
        self.ci.github_direct_push_owner_email = "owner@example.test"
        self.ci.uplift_enabled = False

    @staticmethod
    def verification_event(
        event_type: str, wip: bool | None
    ) -> dict[str, object]:
        """Return one target-project event with a current patch-set payload."""

        change: dict[str, object] = {
            "project": "MyPiCarX",
            "branch": "main",
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

    def test_exact_mypicarx_destination_is_allowed(self) -> None:
        """Allow only the configured MyPiCarX main synchronization."""

        self.assertTrue(self.ci.validate_github_destination())

    def test_component_github_destination_is_rejected(self) -> None:
        """Never synchronize an individual component repository to GitHub."""

        self.ci.github_remote = "git@github.com:example/xWalkHal.git"
        self.assertFalse(self.ci.validate_github_destination())

    def test_disabled_github_push_is_rejected(self) -> None:
        """Require the explicit GitHub synchronization enable switch."""

        self.ci.github_push_enabled = False
        self.assertFalse(self.ci.validate_github_destination())

    def test_non_main_integration_branch_is_rejected(self) -> None:
        """Reject any source branch other than MyPiCarX main."""

        self.ci.branch = "review"
        self.assertFalse(self.ci.validate_github_destination())

    def test_non_main_github_branch_is_rejected(self) -> None:
        """Reject any GitHub target branch other than main."""

        self.ci.github_branch = "review"
        self.assertFalse(self.ci.validate_github_destination())

    def test_submitted_component_triggers_enabled_uplift(self) -> None:
        """Select a merged module revision for automatic integration review."""

        self.ci.project = "xWalkHal"
        self.ci.uplift_enabled = True
        event = {
            "type": "change-merged",
            "change": {"project": "xWalkHal", "branch": "main", "number": 152},
            "newRev": "0" * 40,
        }
        self.assertTrue(self.ci.matching_uplift_event(event))

    def test_integration_project_never_uplifts_itself(self) -> None:
        """Keep MyPiCarX submission on the guarded GitHub path."""

        self.ci.uplift_enabled = True
        event = {
            "type": "change-merged",
            "change": {"project": "MyPiCarX", "branch": "main", "number": 153},
            "newRev": "1" * 40,
        }
        self.assertFalse(self.ci.matching_uplift_event(event))

    def test_module_checkout_uses_private_integration_baseline(self) -> None:
        """Overlay a module patch set onto exact MyPiCarX submodules."""

        self.ci.project = "xWalkHal"
        self.ci.user = "ci"
        self.ci.host = "gerrit.example"
        self.ci.port = "29418"
        self.ci.private_key = pathlib.Path("/key")
        self.ci.state_directory = pathlib.Path("/state")
        commands = [command for command, unused_environment in self.ci.checkout_commands("refs/changes/1")]
        self.assertIn("/MyPiCarX", commands[1][-1])
        self.assertIn(["git", "submodule", "update", "--init", "--recursive"], commands)
        self.assertIn(
            ["git", "-C", "xWalkHal", "fetch", "origin", "refs/changes/1"], commands
        )

    def test_code_module_has_standalone_build_and_test(self) -> None:
        """Run the required standalone CMake workflow before integration CI."""

        self.ci.project = "xWalkHal"
        commands = self.ci.standalone_commands(pathlib.Path("/workspace"))
        self.assertEqual(commands[0][0:2], ["cmake", "--fresh"])
        self.assertIn("-DXWALK_STANDALONE_BUILD=ON", commands[0])
        self.assertEqual(commands[-1][0], "ctest")

    def test_github_sync_requires_ci_verified_revision(self) -> None:
        """Match the submitted commit and CI account's Verified +1 approval."""

        self.ci.user = "xwalk-ci"
        self.ci.host = "gerrit.example"
        self.ci.port = "29418"
        self.ci.private_key = pathlib.Path("/key")
        self.ci.state_directory = pathlib.Path("/state")
        query = {
            "currentPatchSet": {
                "revision": "a" * 40,
                "approvals": [
                    {"type": "Verified", "value": "1", "by": {"username": "xwalk-ci"}}
                ],
            }
        }
        result = mock.Mock(returncode=0, stdout=f"{json.dumps(query)}\n")
        with mock.patch("xWalkGerritCi.subprocess.run", return_value=result):
            self.assertTrue(self.ci.submitted_revision_verified(12, "a" * 40))

    def test_github_sync_rejects_another_accounts_vote(self) -> None:
        """Do not accept a Verified vote attributed to another account."""

        self.ci.user = "xwalk-ci"
        self.ci.host = "gerrit.example"
        self.ci.port = "29418"
        self.ci.private_key = pathlib.Path("/key")
        self.ci.state_directory = pathlib.Path("/state")
        query = {
            "currentPatchSet": {
                "revision": "b" * 40,
                "approvals": [
                    {"type": "Verified", "value": "1", "by": {"username": "other-ci"}}
                ],
            }
        }
        result = mock.Mock(returncode=0, stdout=f"{json.dumps(query)}\n")
        with mock.patch("xWalkGerritCi.subprocess.run", return_value=result):
            self.assertFalse(self.ci.submitted_revision_verified(13, "b" * 40))


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
