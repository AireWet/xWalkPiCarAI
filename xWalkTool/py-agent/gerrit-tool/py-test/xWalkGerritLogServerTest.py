#!/usr/bin/env python3
"""Test the server-rendered xWalk Gerrit CI module dashboard."""

from __future__ import annotations

import json
from pathlib import Path
import sys
import tempfile
import unittest
from urllib.request import urlopen

sys.path.insert(0, str(Path(__file__).parents[1] / "py-src"))

from xWalkGerritLogServer import XWalkGerritLogServer


class XWalkGerritLogServerTest(unittest.TestCase):
    """Verify structured state, rendering, escaping, and read-only routes."""

    def setUp(self) -> None:
        """Create one isolated retained-log directory."""

        self.temporary_directory = tempfile.TemporaryDirectory()
        self.log_directory = Path(self.temporary_directory.name)
        self.log_server = XWalkGerritLogServer(
            self.log_directory, "127.0.0.1", 0, "http://ci.example:8091",
        )

    def tearDown(self) -> None:
        """Stop any test server and remove its retained logs."""

        if self.log_server.server is not None:
            self.log_server.server.shutdown()
            self.log_server.server.server_close()
        self.temporary_directory.cleanup()

    def write_log(self, contents: str = "combined output\n") -> Path:
        """Write one valid retained log."""

        path = self.log_directory / "change-9-2-20260809T130000Z.log"
        path.write_text(contents, encoding="utf-8")
        return path

    def state(self, overrides: dict[str, str] | None = None) -> dict[str, object]:
        """Create a complete module graph with optional status overrides."""

        overrides = overrides or {}
        jobs = []
        for identifier, name in self.log_server.MODULES:
            status = overrides.get(identifier, "PASSED")
            checks = [] if identifier == "host-quality-gate" else [
                {
                    "id": "test", "name": f"{name} test", "status": status,
                    "duration_seconds": 1.25,
                }
            ]
            jobs.append({
                "id": identifier, "name": name, "needs": [],
                "log_link": f"jobs/{identifier}", "status": status,
                "duration_seconds": 2.5, "checks": checks,
            })
        return {"schema_version": 1, "name": "xWalk Host Quality", "jobs": jobs}

    def write_state(self, log_path: Path, state: dict[str, object]) -> None:
        """Write one structured state sidecar."""

        log_path.with_suffix(".json").write_text(json.dumps(state), encoding="utf-8")

    def render(self, state: dict[str, object], contents: str = "combined output\n") -> str:
        """Render one state and matching retained log."""

        log_path = self.write_log(contents)
        self.write_state(log_path, state)
        return self.log_server.render_dashboard(log_path, contents)

    def test_all_passed_graph_has_three_stages_and_clickable_nodes(self) -> None:
        """Render the complete passed Preparation-to-modules-to-gate graph."""

        page = self.render(self.state())
        self.assertIn('data-layout="preparation-modules-gate"', page)
        self.assertIn('aria-label="Preparation stage"', page)
        self.assertIn('aria-label="Module stage"', page)
        self.assertIn('aria-label="Quality gate stage"', page)
        self.assertIn('href="/changes/9/2/jobs/xwalk-hal"', page)

    def test_component_graph_renders_only_dynamic_selected_module(self) -> None:
        """Render an arbitrary component node without adding unrelated product modules."""

        state = self.state()
        state["jobs"] = [
            state["jobs"][0],
            {
                "id": "xwalk-controller", "name": "xWalkController",
                "needs": ["preparation"], "log_link": "jobs/xwalk-controller",
                "status": "PASSED", "duration_seconds": 7.0,
                "checks": [{
                    "id": "tests", "name": "Controller host tests",
                    "status": "PASSED", "duration_seconds": 0.2,
                }],
            },
            {
                **state["jobs"][-1],
                "name": "xWalk Component Quality Gate",
                "needs": ["xwalk-controller"],
            },
        ]
        page = self.render(state)

        self.assertIn('href="/changes/9/2/jobs/xwalk-controller"', page)
        self.assertIn("xWalkController", page)
        self.assertIn("xWalk Component Quality Gate", page)
        self.assertNotIn("xWalkAgent", page)
        self.assertEqual(
            self.log_server.job_url(9, 2, "xwalk-controller"),
            "http://ci.example:8091/changes/9/2/jobs/xwalk-controller",
        )

    def test_failed_module_and_gate_are_distinguishable(self) -> None:
        """Show failed text and styling without relying on colour alone."""

        page = self.render(self.state({"xwalk-hal": "FAILED", "host-quality-gate": "FAILED"}))
        self.assertIn('status-failed', page)
        self.assertIn('xWalkHal: FAILED', page)
        self.assertIn('Overall status: <strong class="failed">FAILED</strong>', page)

    def test_waiting_running_pending_skipped_and_cancelled_states_render(self) -> None:
        """Support every nonterminal state required by the dashboard."""

        page = self.render(self.state({
            "xwalk-agent": "RUNNING", "xwalk-controller": "WAITING",
            "xwalk-hal": "PENDING", "xwalk-iw": "SKIPPED", "xwalk-library": "CANCELLED",
            "host-quality-gate": "RUNNING",
        }))
        for status in ("WAITING", "RUNNING", "PENDING", "SKIPPED", "CANCELLED"):
            self.assertIn(status, page)
        self.assertIn('http-equiv="refresh"', page)

    def test_queued_state_renders_separately_from_pending(self) -> None:
        """Expose a queued module before its worker begins."""

        page = self.render(self.state({"xwalk-library": "QUEUED"}))
        self.assertIn('class="graph-node status-queued"', page)
        self.assertIn("QUEUED", page)

    def test_unavailable_code_health_is_visible_without_failing_the_final_gate(self) -> None:
        """Distinguish a non-blocking service outage from a completed analysis."""

        page = self.render(self.state({"codescene-code-health": "UNAVAILABLE"}))
        self.assertIn('class="graph-node status-unavailable"', page)
        self.assertIn("UNAVAILABLE", page)
        self.assertIn('Overall status: <strong class="passed">PASSED</strong>', page)

    def test_missing_duration_and_long_check_name_are_safe(self) -> None:
        """Keep optional durations and long names readable."""

        state = self.state()
        job = state["jobs"][1]
        job["duration_seconds"] = None
        job["checks"][0]["name"] = "A very long module check name that must wrap inside the result card"
        page = self.render(state)
        self.assertIn("Duration unavailable", page)
        self.assertIn("overflow-wrap:anywhere", page)
        self.assertIn("A very long module check name", page)

    def test_missing_optional_module_does_not_break_graph(self) -> None:
        """Render known available nodes when an optional sidecar entry is absent."""

        state = self.state()
        state["jobs"] = [job for job in state["jobs"] if job["id"] != "xwalk-vision"]
        page = self.render(state)
        self.assertNotIn('id="job-xwalk-vision"', page)
        self.assertIn('id="job-xwalk-streaming"', page)

    def test_log_and_check_content_are_html_escaped(self) -> None:
        """Prevent retained output and check names from injecting markup."""

        state = self.state()
        state["jobs"][1]["checks"][0]["name"] = '<img src=x onerror="unsafe()">'
        page = self.render(state, "<script>unsafe()</script>\n")
        self.assertNotIn("<script>unsafe()</script>", page)
        self.assertIn("&lt;script&gt;unsafe()&lt;/script&gt;", page)
        self.assertNotIn("<img src=x", page)

    def test_mobile_structure_and_visible_keyboard_focus_exist(self) -> None:
        """Retain a vertical narrow layout and accessible focus treatment."""

        page = self.render(self.state())
        self.assertIn("@media (max-width:760px)", page)
        self.assertIn("grid-template-columns:1fr", page)
        self.assertIn("a:focus-visible", page)

    def test_every_module_has_stable_anchor_and_log_route(self) -> None:
        """Generate stable anchors and module-log links for every known job."""

        page = self.render(self.state())
        for identifier, unused_name in self.log_server.MODULES:
            self.assertIn(f'id="job-{identifier}"', page)
            self.assertIn(f'/changes/9/2/jobs/{identifier}', page)

    def test_required_twelve_module_nodes_are_rendered(self) -> None:
        """Expose product, documentation, and Code Health nodes between preparation and gate."""

        page = self.render(self.state())
        module_identifiers = [identifier for identifier, unused in self.log_server.MODULES[1:-1]]
        self.assertEqual(len(module_identifiers), 12)
        for name in (
            "xWalkAgent", "xWalkHal", "xWalk Quality", "xWalk Deployment",
            "Developer Documentation", "MyPiCarX / Code Health",
        ):
            self.assertIn(name, page)

    def test_dependency_relationship_is_explained_semantically(self) -> None:
        """Keep the dependency graph understandable without CSS."""

        page = self.render(self.state())
        self.assertIn("Preparation must pass before each module", page)
        self.assertIn("every module must pass before the gate", page)

    def test_final_gate_controls_overall_status(self) -> None:
        """Derive overall Gerrit status from the structured final gate."""

        log_path = self.write_log()
        state = self.state({"host-quality-gate": "FAILED"})
        self.write_state(log_path, state)
        loaded = self.log_server.load_state(log_path)
        self.assertEqual(self.log_server.overall_status("", loaded), "FAILED")

    def test_unknown_status_fails_closed(self) -> None:
        """Handle malformed status input without displaying false success."""

        page = self.render(self.state({"host-quality-gate": "mystery"}))
        self.assertIn('Overall status: <strong class="failed">FAILED</strong>', page)

    def test_raw_log_link_and_complete_log_are_preserved(self) -> None:
        """Keep both existing full-log access paths."""

        page = self.render(self.state())
        self.assertIn("Open raw full log", page)
        self.assertIn("/logs/change-9-2-20260809T130000Z.log", page)
        self.assertIn("Complete log", page)

    def test_module_page_contains_checks_and_only_that_module_output(self) -> None:
        """Show one module's checks and matching retained check sections."""

        contents = (
            "------------------------ CHECK xwalk-hal/unit ------------------------\nHAL output\n"
            "[PASSED] xwalk-hal/unit\n"
            "------------------------ CHECK xwalk-agent/aggregate ------------------------\nAgent output\n"
            "[PASSED] xwalk-agent/aggregate\n"
        )
        log_path = self.write_log(contents)
        state = self.state()
        self.write_state(log_path, state)
        loaded = self.log_server.load_state(log_path)
        assert loaded is not None
        page = self.log_server.render_job_dashboard(log_path, contents, "xwalk-hal", loaded)
        self.assertIn("HAL output", page)
        self.assertNotIn("Agent output", page)
        self.assertIn("Back to module graph", page)
        self.assertEqual(page.count("<h1>xWalkHal</h1>"), 1)
        self.assertNotIn("<h3>xWalkHal</h3>", page)
        self.assertNotIn("Open module log", page)

    def test_cancelled_module_without_output_explains_missing_log(self) -> None:
        """Explain a cancelled module without claiming that it has not started."""

        log_path = self.write_log()
        state = self.state({"xwalk-library": "CANCELLED"})
        self.write_state(log_path, state)
        loaded = self.log_server.load_state(log_path)
        assert loaded is not None
        page = self.log_server.render_job_dashboard(log_path, "", "xwalk-library", loaded)
        self.assertIn("cancelled before retained log output was available", page)

    def test_gate_page_explains_dependency_calculation(self) -> None:
        """Describe the aggregate gate instead of presenting a missing command log."""

        log_path = self.write_log()
        state = self.state({"host-quality-gate": "FAILED"})
        self.write_state(log_path, state)
        loaded = self.log_server.load_state(log_path)
        assert loaded is not None
        page = self.log_server.render_job_dashboard(log_path, "", "host-quality-gate", loaded)
        self.assertIn("calculated from its module dependencies", page)
        self.assertIn("does not execute an independent test", page)

    def test_historical_text_log_keeps_complete_log_access(self) -> None:
        """Render retained pre-sidecar runs without fabricating module state."""

        log_path = self.write_log("QUALITY SUMMARY\n[PASSED] gerrit-host\n")
        page = self.log_server.render_dashboard(log_path, log_path.read_text(encoding="utf-8"))
        self.assertIn("Historical CI result", page)
        self.assertIn("Complete log", page)

    def test_latest_log_uses_exact_patch_set_and_timestamp(self) -> None:
        """Select only the newest valid log for the requested patch set."""

        old = self.log_directory / "change-9-2-20260809T120000Z.log"
        old.write_text("old", encoding="utf-8")
        newest = self.write_log("new")
        other = self.log_directory / "change-9-3-20260809T140000Z.log"
        other.write_text("other", encoding="utf-8")
        self.assertEqual(self.log_server.latest_log(9, 2), newest)

    def test_http_server_exposes_dashboard_module_and_raw_log(self) -> None:
        """Serve live structured results and raw text through read-only routes."""

        log_path = self.write_log()
        self.write_state(log_path, self.state())
        try:
            self.log_server.start()
        except PermissionError:
            self.skipTest("The validation sandbox does not permit loopback sockets")
        assert self.log_server.server is not None
        base_url = f"http://127.0.0.1:{self.log_server.server.server_port}"
        with urlopen(f"{base_url}/changes/9/2", timeout=2.0) as response:
            self.assertIn(b"Module dependency graph", response.read())
        with urlopen(f"{base_url}/changes/9/2/jobs/xwalk-hal", timeout=2.0) as response:
            self.assertIn(b"xWalkHal", response.read())
        with urlopen(f"{base_url}/logs/{log_path.name}", timeout=2.0) as response:
            self.assertEqual(response.read(), b"combined output\n")


if __name__ == "__main__":
    unittest.main()
