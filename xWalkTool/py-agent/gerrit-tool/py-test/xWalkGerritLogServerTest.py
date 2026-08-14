#!/usr/bin/env python3
"""Test the read-only xWalk Gerrit CI log dashboard."""

from __future__ import annotations

from pathlib import Path
import sys
import tempfile
import unittest
from urllib.request import urlopen

sys.path.insert(0, str(Path(__file__).parents[1] / "py-src"))

from xWalkGerritLogServer import XWalkGerritLogServer


class XWalkGerritLogServerTest(unittest.TestCase):
    """Verify log selection, rendering, status, and HTTP access."""

    def setUp(self) -> None:
        """Create one isolated retained-log directory."""

        self.temporary_directory = tempfile.TemporaryDirectory()
        self.log_directory = Path(self.temporary_directory.name)
        self.log_server = XWalkGerritLogServer(
            self.log_directory,
            "127.0.0.1",
            0,
            "http://ci.example:8091",
        )

    def tearDown(self) -> None:
        """Stop any test server and remove its retained logs."""

        if self.log_server.server is not None:
            self.log_server.server.shutdown()
            self.log_server.server.server_close()
        self.temporary_directory.cleanup()

    def write_log(self, name: str, contents: str) -> Path:
        """Write one test-owned retained log and return its path."""

        path = self.log_directory / name
        path.write_text(contents, encoding="utf-8")
        return path

    def test_latest_log_uses_exact_patch_set_and_timestamp(self) -> None:
        """Select only the newest valid log for the requested patch set."""

        self.write_log("change-9-2-20260809T120000Z.log", "old")
        newest = self.write_log("change-9-2-20260809T130000Z.log", "new")
        self.write_log("change-9-3-20260809T140000Z.log", "other patch")
        self.write_log("change-9-2-invalid.log", "invalid")
        self.assertEqual(self.log_server.latest_log(9, 2), newest)
        self.assertIsNone(self.log_server.latest_log(10, 1))

    def test_status_tracks_running_and_aggregate_results(self) -> None:
        """Derive status without treating an incomplete log as successful."""

        self.assertEqual(self.log_server.overall_status("[PASSED] gerrit-host\n"), "RUNNING")
        self.assertEqual(
            self.log_server.overall_status(
                "QUALITY SUMMARY\n[PASSED] gerrit-host\n[PASSED] gcc Debug\n"
            ),
            "PASSED",
        )
        self.assertEqual(
            self.log_server.overall_status(
                "QUALITY SUMMARY\n[PASSED] gerrit-host\n[FAILED] thread-sanitizer\n"
            ),
            "FAILED",
        )

    def test_job_urls_are_stable_and_distinct(self) -> None:
        """Create one stable URL for each configured aggregate job."""

        links = self.log_server.job_links(9, 2)
        self.assertEqual(len(links), 12)
        self.assertEqual(
            self.log_server.job_url(9, 2, "gcc Debug"),
            "http://ci.example:8091/changes/9/2/jobs/gcc-debug",
        )
        self.assertEqual(self.log_server.job_name("gcc-debug"), "gcc Debug")
        self.assertIsNone(self.log_server.job_name("unknown"))

    def test_public_path_prefixes_generated_browser_links(self) -> None:
        """Keep dashboard navigation below the HTTPS proxy prefix."""

        self.log_server.public_url = "https://review.example:18443/ci"
        self.log_server.public_path = "/ci"
        log_path = self.write_log("change-9-2-20260809T130000Z.log", "running\n")
        page = self.log_server.render_dashboard(log_path, "running\n")
        self.assertIn('href="/ci/changes/9/2/jobs/gerrit-host"', page)
        self.assertIn('href="/ci/logs/change-9-2-20260809T130000Z.log"', page)

    def test_job_status_and_contents_follow_job_boundaries(self) -> None:
        """Isolate one job and track its waiting, running, and final states."""

        waiting_log = "checkout output\n"
        running_log = (
            "======================== gcc Debug ========================\n"
            "$ cmake --build\ncompiler output\n"
        )
        complete_log = (
            f"{running_log}[PASSED] gcc Debug\n"
            "======================== gcc Release ========================\n"
            "release-only output\n[FAILED] gcc Release\n"
        )
        self.assertEqual(self.log_server.job_status(waiting_log, "gcc Debug"), "WAITING")
        self.assertEqual(self.log_server.job_status(running_log, "gcc Debug"), "RUNNING")
        self.assertEqual(self.log_server.job_status(complete_log, "gcc Debug"), "PASSED")
        self.assertEqual(self.log_server.job_status(complete_log, "gcc Release"), "FAILED")
        debug_output = self.log_server.job_contents(complete_log, "gcc Debug")
        self.assertIn("compiler output", debug_output)
        self.assertNotIn("release-only output", debug_output)

    def test_dashboard_contains_jobs_and_escaped_full_log(self) -> None:
        """Render the overview and full log without allowing HTML injection."""

        log_path = self.write_log(
            "change-9-2-20260809T130000Z.log",
            "[PASSED] gerrit-host\n<script>unsafe()</script>\n",
        )
        page = self.log_server.render_dashboard(
            log_path,
            log_path.read_text(encoding="utf-8"),
        )
        self.assertIn("gerrit-host", page)
        self.assertIn("thread-sanitizer", page)
        self.assertIn("&lt;script&gt;unsafe()&lt;/script&gt;", page)
        self.assertNotIn("<script>unsafe()</script>", page)
        self.assertIn("/logs/change-9-2-20260809T130000Z.log", page)
        self.assertIn('/changes/9/2/jobs/gcc-debug', page)

    def test_job_dashboard_contains_only_requested_job_output(self) -> None:
        """Render one linked job page without including another job's output."""

        log_path = self.write_log(
            "change-9-2-20260809T130000Z.log",
            "======================== gcc Debug ========================\n"
            "debug-only output\n[PASSED] gcc Debug\n"
            "======================== gcc Release ========================\n"
            "release-only output\n",
        )
        page = self.log_server.render_job_dashboard(
            log_path,
            log_path.read_text(encoding="utf-8"),
            "gcc Debug",
        )
        self.assertIn("debug-only output", page)
        self.assertNotIn("release-only output", page)
        self.assertIn("Back to all jobs", page)

    def test_http_server_exposes_health_dashboard_and_raw_log(self) -> None:
        """Serve live results and raw text through read-only GET routes."""

        log_name = "change-9-2-20260809T130000Z.log"
        self.write_log(log_name, "[PASSED] gerrit-host\n")
        self.log_server.start()
        assert self.log_server.server is not None
        base_url = f"http://127.0.0.1:{self.log_server.server.server_port}"
        with urlopen(f"{base_url}/health", timeout=2.0) as response:
            self.assertEqual(response.read(), b"ok\n")
        with urlopen(f"{base_url}/changes/9/2", timeout=2.0) as response:
            self.assertIn(b"Complete log", response.read())
        with urlopen(f"{base_url}/changes/9/2/jobs/gerrit-host", timeout=2.0) as response:
            self.assertIn(b"gerrit-host", response.read())
        with urlopen(f"{base_url}/logs/{log_name}", timeout=2.0) as response:
            self.assertEqual(response.read(), b"[PASSED] gerrit-host\n")


if __name__ == "__main__":
    unittest.main()
