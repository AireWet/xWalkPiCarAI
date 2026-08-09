#!/usr/bin/env python3
"""Serve live xWalk Gerrit verification results and complete logs."""

from __future__ import annotations

from html import escape
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import re
import threading
from urllib.parse import unquote, urlparse


class XWalkGerritLogServer:
    """Expose retained CI logs through a read-only Actions-style dashboard."""

    LOG_NAME = re.compile(
        r"^change-(?P<change>[0-9]+)-(?P<patch>[0-9]+)-"
        r"(?P<timestamp>[0-9]{8}T[0-9]{6}Z)\.log$"
    )
    RESULT_LINE = re.compile(r"^\[(PASSED|FAILED)\] (.+)$", re.MULTILINE)
    JOB_SLUGS = {
        "gerrit-host": "gerrit-host",
        "gcc Debug": "gcc-debug",
        "gcc Release": "gcc-release",
        "clang Debug": "clang-debug",
        "clang Release": "clang-release",
        "sanitizers": "sanitizers",
        "thread-sanitizer": "thread-sanitizer",
        "stress-tests": "stress-tests",
        "static-analysis": "static-analysis",
        "coverage": "coverage",
        "deployment-scripts": "deployment-scripts",
        "staged-install": "staged-install",
    }

    def __init__(
        self,
        log_directory: Path,
        host: str,
        port: int,
        public_url: str,
    ) -> None:
        """Store the retained-log directory and HTTP endpoint configuration."""

        self.log_directory = log_directory
        self.host = host
        self.port = port
        self.public_url = public_url.rstrip("/")
        self.server: ThreadingHTTPServer | None = None
        self.thread: threading.Thread | None = None

    def dashboard_url(self, change: int, patch_set: int) -> str:
        """Return the stable dashboard URL for one Gerrit patch set."""

        return f"{self.public_url}/changes/{change}/{patch_set}"

    def job_url(self, change: int, patch_set: int, job: str) -> str:
        """Return the stable log-page URL for one aggregate quality job."""

        return f"{self.dashboard_url(change, patch_set)}/jobs/{self.JOB_SLUGS[job]}"

    def job_links(self, change: int, patch_set: int) -> list[tuple[str, str]]:
        """Return every aggregate job and its stable patch-set log URL."""

        return [
            (job, self.job_url(change, patch_set, job))
            for job in self.JOB_SLUGS
        ]

    def latest_log(self, change: int, patch_set: int) -> Path | None:
        """Return the newest retained log matching one exact patch set."""

        prefix = f"change-{change}-{patch_set}-"
        matches = [
            path
            for path in self.log_directory.glob(f"{prefix}*.log")
            if self.LOG_NAME.fullmatch(path.name) is not None
        ]
        return max(matches, key=lambda path: path.name) if matches else None

    @classmethod
    def job_results(cls, contents: str) -> dict[str, str]:
        """Extract the latest result written for each aggregate job."""

        return {
            match.group(2): match.group(1)
            for match in cls.RESULT_LINE.finditer(contents)
        }

    @classmethod
    def overall_status(cls, contents: str) -> str:
        """Return RUNNING, PASSED, or FAILED for one retained log."""

        results = cls.job_results(contents)
        if "QUALITY SUMMARY" not in contents:
            return "RUNNING"
        return "FAILED" if "FAILED" in results.values() else "PASSED"

    @classmethod
    def job_name(cls, slug: str) -> str | None:
        """Resolve one validated URL slug to its aggregate quality-job name."""

        for job, job_slug in cls.JOB_SLUGS.items():
            if slug == job_slug:
                return job
        return None

    @classmethod
    def job_status(cls, contents: str, job: str) -> str:
        """Return WAITING, RUNNING, PASSED, or FAILED for one job."""

        results = cls.job_results(contents)
        if job in results:
            return results[job]
        marker = f"{'=' * 24} {job} {'=' * 24}"
        return "RUNNING" if marker in contents else "WAITING"

    @classmethod
    def job_contents(cls, contents: str, job: str) -> str:
        """Extract one complete aggregate job section from a retained log."""

        marker = f"{'=' * 24} {job} {'=' * 24}"
        start = contents.find(marker)
        if start < 0:
            return ""
        section_end = len(contents)
        following_markers = [
            contents.find(f"{'=' * 24} {other_job} {'=' * 24}", start + len(marker))
            for other_job in cls.JOB_SLUGS
            if other_job != job
        ]
        following_markers.append(
            contents.find(f"{'=' * 24} QUALITY SUMMARY {'=' * 24}", start + len(marker))
        )
        valid_ends = [position for position in following_markers if position >= 0]
        if valid_ends:
            section_end = min(valid_ends)
        return contents[start:section_end].rstrip() + "\n"

    def render_dashboard(self, log_path: Path, contents: str) -> str:
        """Render one complete escaped log and its aggregate job overview."""

        match = self.LOG_NAME.fullmatch(log_path.name)
        assert match is not None
        status = self.overall_status(contents)
        rows = []
        change = int(match.group("change"))
        patch_set = int(match.group("patch"))
        for job, slug in self.JOB_SLUGS.items():
            result = self.job_status(contents, job)
            job_path = f"/changes/{change}/{patch_set}/jobs/{slug}"
            rows.append(
                f'<tr><td><a href="{job_path}">{escape(job)}</a></td>'
                f'<td class="{result.lower()}">'
                f"{result}</td></tr>"
            )
        refresh = '<meta http-equiv="refresh" content="10">' if status == "RUNNING" else ""
        raw_url = f"/logs/{escape(log_path.name)}"
        return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
{refresh}
<title>xWalk CI change {match.group('change')}, patch set {match.group('patch')}</title>
<style>
body {{ background:#0d1117; color:#e6edf3; font-family:Arial,sans-serif; margin:0; }}
main {{ margin:auto; max-width:1200px; padding:24px; }}
a {{ color:#58a6ff; }}
.summary {{ background:#161b22; border:1px solid #30363d; border-radius:8px; padding:20px; }}
table {{ border-collapse:collapse; margin:20px 0; width:100%; }}
td {{ border-bottom:1px solid #30363d; padding:9px; }}
td:last-child {{ font-weight:bold; text-align:right; }}
.passed {{ color:#3fb950; }} .failed {{ color:#f85149; }}
.running {{ color:#d29922; }} .waiting {{ color:#8b949e; }}
pre {{ background:#010409; border:1px solid #30363d; border-radius:8px; overflow:auto; padding:16px; }}
</style>
</head>
<body><main>
<section class="summary">
<h1>xWalk CI</h1>
<p>Gerrit change {match.group('change')}, patch set {match.group('patch')}</p>
<p>Status: <strong class="{status.lower()}">{status}</strong></p>
<p><a href="{raw_url}">Open raw full log</a></p>
<table><tbody>{''.join(rows)}</tbody></table>
</section>
<h2>Complete log</h2>
<pre>{escape(contents)}</pre>
</main></body>
</html>"""

    def render_job_dashboard(self, log_path: Path, contents: str, job: str) -> str:
        """Render one aggregate job's status and complete escaped output."""

        match = self.LOG_NAME.fullmatch(log_path.name)
        assert match is not None
        status = self.job_status(contents, job)
        refresh = (
            '<meta http-equiv="refresh" content="10">'
            if status in ("WAITING", "RUNNING")
            else ""
        )
        change = match.group("change")
        patch_set = match.group("patch")
        dashboard_path = f"/changes/{change}/{patch_set}"
        job_output = self.job_contents(contents, job)
        displayed_output = job_output if job_output else "Job has not started.\n"
        return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
{refresh}
<title>xWalk CI {escape(job)} - change {change}, patch set {patch_set}</title>
<style>
body {{ background:#0d1117; color:#e6edf3; font-family:Arial,sans-serif; margin:0; }}
main {{ margin:auto; max-width:1200px; padding:24px; }}
a {{ color:#58a6ff; }}
.summary {{ background:#161b22; border:1px solid #30363d; border-radius:8px; padding:20px; }}
.passed {{ color:#3fb950; }} .failed {{ color:#f85149; }}
.running {{ color:#d29922; }} .waiting {{ color:#8b949e; }}
pre {{ background:#010409; border:1px solid #30363d; border-radius:8px; overflow:auto; padding:16px; }}
</style>
</head>
<body><main>
<section class="summary">
<h1>{escape(job)}</h1>
<p>Gerrit change {change}, patch set {patch_set}</p>
<p>Status: <strong class="{status.lower()}">{status}</strong></p>
<p><a href="{dashboard_path}">Back to all jobs and the complete log</a></p>
</section>
<h2>Job log</h2>
<pre>{escape(displayed_output)}</pre>
</main></body>
</html>"""

    def start(self) -> None:
        """Start the read-only HTTP server on one daemon thread."""

        log_server = self

        class RequestHandler(BaseHTTPRequestHandler):
            """Route dashboard and raw-log reads without accepting mutations."""

            def send_content(self, status: int, content_type: str, body: bytes) -> None:
                """Write one bounded HTTP response."""

                self.send_response(status)
                self.send_header("Content-Type", content_type)
                self.send_header("Content-Length", str(len(body)))
                self.send_header("X-Content-Type-Options", "nosniff")
                self.send_header("Cache-Control", "no-store")
                self.send_header(
                    "Content-Security-Policy",
                    "default-src 'none'; style-src 'unsafe-inline'",
                )
                self.end_headers()
                self.wfile.write(body)

            def do_GET(self) -> None:
                """Serve health, patch-set dashboard, and raw-log requests."""

                request_path = unquote(urlparse(self.path).path)
                parts = request_path.strip("/").split("/")
                if request_path == "/health":
                    self.send_content(200, "text/plain; charset=utf-8", b"ok\n")
                    return
                is_job_path = (
                    len(parts) == 5
                    and parts[0] == "changes"
                    and parts[3] == "jobs"
                    and all(value.isdigit() for value in parts[1:3])
                )
                if is_job_path:
                    job = log_server.job_name(parts[4])
                    log_path = log_server.latest_log(int(parts[1]), int(parts[2]))
                    if job is None or log_path is None:
                        self.send_content(404, "text/plain; charset=utf-8", b"Job log not found\n")
                        return
                    contents = log_path.read_text(encoding="utf-8", errors="replace")
                    page = log_server.render_job_dashboard(log_path, contents, job)
                    body = page.encode("utf-8")
                    self.send_content(200, "text/html; charset=utf-8", body)
                    return
                if len(parts) == 3 and parts[0] == "changes" and all(
                    value.isdigit() for value in parts[1:]
                ):
                    log_path = log_server.latest_log(int(parts[1]), int(parts[2]))
                    if log_path is None:
                        self.send_content(404, "text/plain; charset=utf-8", b"Log not found\n")
                        return
                    contents = log_path.read_text(encoding="utf-8", errors="replace")
                    body = log_server.render_dashboard(log_path, contents).encode("utf-8")
                    self.send_content(200, "text/html; charset=utf-8", body)
                    return
                if len(parts) == 2 and parts[0] == "logs":
                    log_name = parts[1]
                    if log_server.LOG_NAME.fullmatch(log_name) is not None:
                        log_path = log_server.log_directory / log_name
                        if log_path.is_file():
                            self.send_content(200, "text/plain; charset=utf-8", log_path.read_bytes())
                            return
                self.send_content(404, "text/plain; charset=utf-8", b"Not found\n")

            def log_message(self, format_text: str, *arguments: object) -> None:
                """Suppress routine access output from the Gerrit event log."""

                unused = (format_text, arguments)
                del unused

        self.server = ThreadingHTTPServer((self.host, self.port), RequestHandler)
        self.thread = threading.Thread(
            target=self.server.serve_forever,
            name="xwalk-gerrit-log-server",
            daemon=True,
        )
        self.thread.start()
