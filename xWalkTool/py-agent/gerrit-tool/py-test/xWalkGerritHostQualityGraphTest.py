#!/usr/bin/env python3
"""Validate alignment of GitHub and Gerrit xWalk Host Quality graphs."""

from __future__ import annotations

from pathlib import Path
import sys
import unittest

import yaml

sys.path.insert(0, str(Path(__file__).parents[1] / "py-src"))

from xWalkGerritQuality import GATE, MODULES, PREPARATION


class XWalkGerritHostQualityGraphTest(unittest.TestCase):
    """Keep both CI interfaces aligned to one acyclic module graph."""

    @classmethod
    def setUpClass(cls) -> None:
        """Load the repository workflow once for structural checks."""

        cls.repository = Path(__file__).parents[4]
        workflow_path = cls.repository / ".github/workflows/host-quality.yml"
        if not workflow_path.is_file():
            raise unittest.SkipTest(
                "GitHub integration workflow is not present in the standalone xWalkTool repository"
            )
        cls.workflow = yaml.safe_load(workflow_path.read_text(encoding="utf-8"))
        cls.jobs = cls.workflow["jobs"]

    @staticmethod
    def needs(job: dict[str, object]) -> list[str]:
        """Normalize one GitHub needs declaration."""

        value = job.get("needs", [])
        return [value] if isinstance(value, str) else list(value)

    def test_visible_job_identifiers_match_gerrit_plan(self) -> None:
        """Expose the same Preparation, modules, and gate in both systems."""

        expected = [
            PREPARATION.identifier, *(module.identifier for module in MODULES), GATE.identifier,
        ]
        self.assertEqual(list(self.jobs), expected)

    def test_module_dependencies_and_final_gate_match(self) -> None:
        """Require Preparation before modules and every module before the gate."""

        self.assertEqual(self.needs(self.jobs[PREPARATION.identifier]), [])
        for module in MODULES:
            self.assertEqual(self.needs(self.jobs[module.identifier]), [PREPARATION.identifier])
        self.assertEqual(self.needs(self.jobs[GATE.identifier]), list(GATE.needs))

    def test_dependency_graph_has_no_cycles(self) -> None:
        """Reject cyclic or unknown GitHub job dependencies."""

        visiting: set[str] = set()
        visited: set[str] = set()

        def visit(identifier: str) -> None:
            self.assertNotIn(identifier, visiting, f"dependency cycle at {identifier}")
            if identifier in visited:
                return
            self.assertIn(identifier, self.jobs)
            visiting.add(identifier)
            for dependency in self.needs(self.jobs[identifier]):
                self.assertIn(dependency, self.jobs)
                visit(dependency)
            visiting.remove(identifier)
            visited.add(identifier)

        for identifier in self.jobs:
            visit(identifier)

    def test_quality_job_retains_four_compiler_configurations(self) -> None:
        """Keep GCC/Clang and Debug/Release without a visible matrix fan-out."""

        quality = self.jobs["xwalk-quality"]
        self.assertNotIn("strategy", quality)
        commands = "\n".join(
            str(step.get("run", "")) for step in quality["steps"] if isinstance(step, dict)
        )
        for compiler, build_type in (
            ("gcc", "Debug"), ("gcc", "Release"),
            ("clang", "Debug"), ("clang", "Release"),
        ):
            self.assertIn(f"build-and-test {compiler} {build_type}", commands)

    def test_gate_checks_every_needs_result(self) -> None:
        """Fail the native GitHub gate for non-success module results."""

        gate_commands = "\n".join(
            str(step.get("run", ""))
            for step in self.jobs[GATE.identifier]["steps"]
            if isinstance(step, dict)
        )
        self.assertIn('all(.[]; .result == "success")', gate_commands)
        self.assertEqual(self.jobs[GATE.identifier].get("if"), "always()")

    def test_code_health_has_full_history_and_no_secret_reference(self) -> None:
        """Support exact deltas while keeping forked pull requests secret-free."""

        job = self.jobs["codescene-code-health"]
        checkout = next(step for step in job["steps"] if step.get("uses") == "actions/checkout@v6")
        self.assertEqual(checkout["with"]["fetch-depth"], 0)
        serialized = str(job)
        self.assertNotIn("secrets.", serialized)
        self.assertIn("xWalkCodeHealth analyze", serialized)


if __name__ == "__main__":
    unittest.main()
