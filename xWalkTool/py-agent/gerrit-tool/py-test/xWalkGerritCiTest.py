#!/usr/bin/env python3
"""Test Gerrit event selection and guarded xWalk-rpi5 synchronization."""

from __future__ import annotations

import io
import json
import pathlib
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(pathlib.Path(__file__).parents[1] / "py-src"))

from xWalkGerritCi import XWalkGerritCi
from xWalkGerritLogServer import XWalkGerritLogServer
from xWalkGerritQuality import GATE, MODULES, PREPARATION, XWalkGerritQuality


class XWalkGerritCiTest(unittest.TestCase):
    """Verify Gerrit event selection and GitHub destination selection."""

    def setUp(self) -> None:
        """Create a verifier without initializing host service resources."""

        self.ci = XWalkGerritCi.__new__(XWalkGerritCi)
        self.ci.project = "xWalk-rpi5"
        self.ci.branch = "main"
        self.ci.verification_targets = {
            ("xWalk-rpi5", "main"), ("xWalkPiCarAI", "master"),
        }
        self.ci.github_remote = "git@github.com:example/xWalk-rpi5.git"
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
            "number": 41,
            "project": "xWalk-rpi5",
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

    def test_legacy_monorepository_patch_set_triggers_verification(self) -> None:
        """Keep active legacy reviews verified during repository migration."""

        event = self.verification_event("patchset-created", False)
        event["change"] = {
            "project": "xWalkPiCarAI", "branch": "master", "wip": False,
        }
        self.assertTrue(self.ci.matching_verification_event(event))

    def test_legacy_monorepository_wrong_branch_is_ignored(self) -> None:
        """Accept only the explicit legacy project and master branch pair."""

        event = self.verification_event("patchset-created", False)
        event["change"] = {
            "project": "xWalkPiCarAI", "branch": "main", "wip": False,
        }
        self.assertFalse(self.ci.matching_verification_event(event))

    def test_verification_targets_include_primary_project(self) -> None:
        """Always retain the primary project when extra targets are configured."""

        targets = self.ci.parse_verification_targets(
            "xWalkPiCarAI:master", ("xWalk-rpi5", "main")
        )
        self.assertEqual(
            targets, {("xWalk-rpi5", "main"), ("xWalkPiCarAI", "master")}
        )

    def test_invalid_verification_target_is_rejected(self) -> None:
        """Reject ambiguous target entries before consuming Gerrit events."""

        with self.assertRaisesRegex(SystemExit, "project:branch"):
            self.ci.parse_verification_targets("xWalkPiCarAI", ("xWalk-rpi5", "main"))

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

    def test_exact_xwalk_rpi5_destination_is_allowed(self) -> None:
        """Allow only the configured xWalk-rpi5 main synchronization."""

        self.assertTrue(self.ci.validate_github_destination())

    def test_component_github_destination_is_rejected(self) -> None:
        """Never synchronize an individual component repository to GitHub."""

        self.ci.github_remote = "git@github.com:example/xWalkHal.git"
        self.assertFalse(self.ci.validate_github_destination())

    def test_xwalk_tool_is_not_a_ci_product_repository(self) -> None:
        """Keep administration tooling out of component and integration CI selection."""

        self.assertNotIn("xWalkTool", self.ci.repositories)

    def test_disabled_github_push_is_rejected(self) -> None:
        """Require the explicit GitHub synchronization enable switch."""

        self.ci.github_push_enabled = False
        self.assertFalse(self.ci.validate_github_destination())

    def test_non_main_integration_branch_is_rejected(self) -> None:
        """Reject any source branch other than xWalk-rpi5 main."""

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
        """Keep xWalk-rpi5 submission on the guarded GitHub path."""

        self.ci.uplift_enabled = True
        event = {
            "type": "change-merged",
            "change": {"project": "xWalk-rpi5", "branch": "main", "number": 153},
            "newRev": "1" * 40,
        }
        self.assertFalse(self.ci.matching_uplift_event(event))

    def test_module_checkout_uses_private_integration_baseline(self) -> None:
        """Overlay a module patch set onto exact xWalk-rpi5 submodules."""

        self.ci.project = "xWalkHal"
        self.ci.user = "ci"
        self.ci.host = "gerrit.example"
        self.ci.port = "29418"
        self.ci.private_key = pathlib.Path("/key")
        self.ci.state_directory = pathlib.Path("/state")
        commands = [command for command, unused_environment in self.ci.checkout_commands("refs/changes/1")]
        self.assertIn("/xWalk-rpi5", commands[1][-1])
        self.assertIn(["git", "submodule", "update", "--init", "--recursive"], commands)
        self.assertIn(
            ["git", "-C", "xWalkHal", "fetch", "origin", "refs/changes/1"], commands
        )

    def test_legacy_monorepository_checkout_fetches_its_patch_set_directly(self) -> None:
        """Run the full graph from the reviewed legacy monorepository revision."""

        self.ci.user = "ci"
        self.ci.host = "gerrit.example"
        self.ci.port = "29418"
        self.ci.private_key = pathlib.Path("/key")
        self.ci.state_directory = pathlib.Path("/state")
        commands = [
            command for command, unused_environment in self.ci.checkout_commands(
                "refs/changes/62/62/2", "xWalkPiCarAI"
            )
        ]
        self.assertIn("/xWalkPiCarAI", commands[1][-1])
        self.assertEqual(
            commands[2], ["git", "fetch", "origin", "refs/changes/62/62/2"]
        )
        self.assertNotIn(
            ["git", "submodule", "update", "--init", "--recursive"], commands
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

    def test_each_module_posts_a_separate_uniquely_tagged_change_log_entry(self) -> None:
        """Keep module results separate while reserving the gate for the Verified vote."""

        with tempfile.TemporaryDirectory() as directory:
            log_path = pathlib.Path(directory) / "change-9-2-20260814T160000Z.log"
            with log_path.open("w", encoding="utf-8") as log:
                XWalkGerritQuality(pathlib.Path(directory), log)
            self.ci.log_server = XWalkGerritLogServer(
                pathlib.Path(directory), "127.0.0.1", 0, "https://ci.example/ci",
            )
            with mock.patch.object(self.ci, "post_message", return_value=True) as post:
                reported = self.ci.report_module_results(
                    9, 2, log_path, "https://ci.example/ci/changes/9/2"
                )

        self.assertTrue(reported)
        self.assertEqual(post.call_count, 1 + len(MODULES))
        tags = [call.args[3] for call in post.call_args_list]
        self.assertEqual(len(tags), len(set(tags)))
        self.assertIn("autogenerated:xwalk-ci:xwalk-hal", tags)
        self.assertNotIn("autogenerated:xwalk-ci:host-quality-gate", tags)
        messages = [call.args[2] for call in post.call_args_list]
        self.assertTrue(any("xWalkHal\nStatus: WAITING" in message for message in messages))
        self.assertTrue(any("Module tests and log:" in message for message in messages))

    def test_code_health_uses_exact_gerrit_patchset_metadata(self) -> None:
        """Analyse the reviewed revision and its parent with non-secret correlation IDs."""

        event = self.verification_event("patchset-created", False)
        event["patchSet"]["revision"] = "a" * 40
        environment = self.ci.code_health_environment(event)
        self.assertEqual(environment["GERRIT_CHANGE_NUMBER"], "41")
        self.assertEqual(environment["GERRIT_PATCHSET_NUMBER"], "3")
        self.assertEqual(environment["GERRIT_PATCHSET_REVISION"], "a" * 40)
        self.assertEqual(environment["GERRIT_REFSPEC"], "refs/changes/41/41/3")
        self.assertEqual(environment["XWALK_CODESCENE_REVISION"], "a" * 40)
        self.assertEqual(environment["XWALK_CODESCENE_BASE_REVISION"], f"{'a' * 40}^")

    def test_code_health_module_is_a_separate_gerrit_result(self) -> None:
        """Expose CodeScene details without adding a new Gerrit vote label."""

        module = next(
            item for item in MODULES if item.identifier == "codescene-code-health"
        )
        self.assertEqual(module.name, "MyPiCarX / Code Health")
        self.assertEqual(module.checks[0].arguments, ("codescene",))

    def test_component_patchset_does_not_analyse_unrelated_integration_head(self) -> None:
        """Mark component delta unavailable until native integration or uplift."""

        event = self.verification_event("patchset-created", False)
        event["change"]["project"] = "xWalkHal"
        environment = self.ci.code_health_environment(event)
        self.assertIn("integrated MyPiCarX", environment["XWALK_CODESCENE_UNAVAILABLE_REASON"])


class XWalkGerritQualityTest(unittest.TestCase):
    """Verify the structured, resource-safe Gerrit module graph."""

    def test_module_graph_has_stable_dependencies(self) -> None:
        """Use Preparation and every module as hard final-gate prerequisites."""

        self.assertEqual(PREPARATION.needs, ())
        self.assertTrue(all(module.needs == ("preparation",) for module in MODULES))
        self.assertEqual(GATE.needs, tuple(module.identifier for module in MODULES))

    def test_quality_preserves_every_compiler_variant_and_specialist_check(self) -> None:
        """Keep compiler, sanitizer, analysis, coverage, fuzz, stress, and Valgrind checks."""

        quality = next(module for module in MODULES if module.identifier == "xwalk-quality")
        identifiers = {check.identifier for check in quality.checks}
        expected = {
            "gcc-debug", "gcc-release", "clang-debug", "clang-release", "asan-ubsan",
            "leak-sanitizer", "thread-sanitizer", "stress-tests", "fuzz-smoke",
            "static-analysis", "coverage", "valgrind", "clang-static-analyzer",
        }
        self.assertEqual(identifiers, expected)

    def test_result_model_contains_module_log_links(self) -> None:
        """Publish a stable module log link in every structured result."""

        quality = XWalkGerritQuality(pathlib.Path("/workspace"), io.StringIO())
        for module in quality.state["jobs"]:
            self.assertEqual(module["log_link"], f"jobs/{module['id']}")

    def test_output_redaction_removes_credentials(self) -> None:
        """Redact secret assignments, authenticated URLs, and private keys."""

        output = (
            "ACCESS_TOKEN=secret-value\nhttps://user:password@example.test/repo\n"
            "-----BEGIN OPENSSH PRIVATE KEY-----\nsecret\n-----END OPENSSH PRIVATE KEY-----\n"
        )
        redacted = XWalkGerritQuality.redact(output)
        self.assertNotIn("secret-value", redacted)
        self.assertNotIn("user:password", redacted)
        self.assertNotIn("OPENSSH PRIVATE KEY-----\nsecret", redacted)

    def test_unavailable_code_health_is_visible_but_non_blocking_in_rollout(self) -> None:
        """Retain an unavailable state while allowing the boolean dependency policy."""

        with tempfile.TemporaryDirectory() as directory:
            workspace = pathlib.Path(directory)
            report = workspace / "build-host/codescene"
            report.mkdir(parents=True)
            (report / "summary.json").write_text(
                json.dumps({"status": "UNAVAILABLE"}), encoding="utf-8"
            )
            quality = XWalkGerritQuality(workspace, io.StringIO())
            module = next(
                item for item in MODULES if item.identifier == "codescene-code-health"
            )
            completed = mock.Mock(returncode=0)
            with mock.patch("xWalkGerritQuality.subprocess.run", return_value=completed):
                passed = quality.run_module(module)
        self.assertTrue(passed)
        self.assertEqual(quality.job_state(module.identifier)["status"], "UNAVAILABLE")

    def test_degraded_code_health_is_visible_but_non_blocking_in_rollout(self) -> None:
        """Show a failed advisory delta without converting it into strict enforcement."""

        with tempfile.TemporaryDirectory() as directory:
            workspace = pathlib.Path(directory)
            report = workspace / "build-host/codescene"
            report.mkdir(parents=True)
            (report / "summary.json").write_text(
                json.dumps({"status": "FAILED"}), encoding="utf-8"
            )
            quality = XWalkGerritQuality(workspace, io.StringIO())
            module = next(
                item for item in MODULES if item.identifier == "codescene-code-health"
            )
            completed = mock.Mock(returncode=0)
            with mock.patch("xWalkGerritQuality.subprocess.run", return_value=completed):
                passed = quality.run_module(module)
        self.assertTrue(passed)
        self.assertEqual(quality.job_state(module.identifier)["status"], "FAILED")

    def test_failed_preparation_skips_modules_and_fails_gate(self) -> None:
        """Do not run dependent modules after Preparation fails."""

        quality = XWalkGerritQuality(pathlib.Path("/workspace"), io.StringIO())
        with mock.patch.object(quality, "run_module", return_value=False):
            results = quality.run_all()
        self.assertFalse(results["preparation"])
        self.assertFalse(results["host-quality-gate"])
        for module in MODULES:
            self.assertEqual(quality.job_state(module.identifier)["status"], "SKIPPED")

if __name__ == "__main__":
    unittest.main()
