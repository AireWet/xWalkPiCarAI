#!/usr/bin/env python3
"""Serve live xWalk Gerrit verification results and complete logs."""

from __future__ import annotations

from html import escape
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import re
import threading
from urllib.parse import unquote, urlparse


PAGE_STYLE = """
body { background:#0d1117; color:#e6edf3; font-family:Arial,sans-serif; margin:0; }
main { margin:auto; max-width:1200px; padding:24px; }
a { color:#58a6ff; }
.summary { background:#161b22; border:1px solid #30363d; border-radius:8px; padding:20px; }
table { border-collapse:collapse; margin:20px 0; width:100%; }
td { border-bottom:1px solid #30363d; padding:9px; }
td:last-child { font-weight:bold; text-align:right; }
.passed { color:#3fb950; } .failed { color:#f85149; }
.running { color:#d29922; } .waiting { color:#8b949e; }
pre { background:#010409; border:1px solid #30363d; border-radius:8px; overflow:auto; padding:16px; }
"""


def html_page(title: str, refresh: str, summary: str, heading: str, output: str) -> str:
    """Render the shared escaped CI dashboard document shell."""

    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
{refresh}<title>{title}</title><style>{PAGE_STYLE}</style></head>
<body><main><section class="summary">{summary}</section>
<h2>{heading}</h2><pre>{escape(output)}</pre></main></body></html>"""


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
        self.public_path = urlparse(self.public_url).path.rstrip("/")
        self.server: ThreadingHTTPServer | None = None
        self.thread: threading.Thread | None = None

    def public_route(self, path: str) -> str:
        """Prefix one browser route with the configured public URL path."""

        return f"{self.public_path}{path}"

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

    def dashboard_rows(self, change: int, patch_set: int, contents: str) -> str:
        """Render linked status rows for every aggregate quality job."""

        rows = []
        for job, slug in self.JOB_SLUGS.items():
            status = self.job_status(contents, job)
            path = self.public_route(f"/changes/{change}/{patch_set}/jobs/{slug}")
            rows.append(
                f'<tr><td><a href="{path}">{escape(job)}</a></td>'
                f'<td class="{status.lower()}">{status}</td></tr>'
            )
        return "".join(rows)

    def render_dashboard(self, log_path: Path, contents: str) -> str:
        """Render one complete escaped log and its aggregate job overview."""

        match = self.LOG_NAME.fullmatch(log_path.name)
        assert match is not None
        change = int(match.group("change"))
        patch_set = int(match.group("patch"))
        status = self.overall_status(contents)
        refresh = '<meta http-equiv="refresh" content="10">' if status == "RUNNING" else ""
        raw_url = self.public_route(f"/logs/{escape(log_path.name)}")
        summary = (
            f"<h1>xWalk CI</h1><p>Gerrit change {change}, patch set {patch_set}</p>"
            f'<p>Status: <strong class="{status.lower()}">{status}</strong></p>'
            f'<p><a href="{raw_url}">Open raw full log</a></p>'
            f"<table><tbody>{self.dashboard_rows(change, patch_set, contents)}</tbody></table>"
        )
        title = f"xWalk CI change {change}, patch set {patch_set}"
        return html_page(title, refresh, summary, "Complete log", contents)

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
        dashboard_path = self.public_route(f"/changes/{change}/{patch_set}")
        job_output = self.job_contents(contents, job)
        displayed_output = job_output if job_output else "Job has not started.\n"
        summary = (
            f"<h1>{escape(job)}</h1><p>Gerrit change {change}, patch set {patch_set}</p>"
            f'<p>Status: <strong class="{status.lower()}">{status}</strong></p>'
            f'<p><a href="{dashboard_path}">Back to all jobs and the complete log</a></p>'
        )
        title = f"xWalk CI {escape(job)} - change {change}, patch set {patch_set}"
        return html_page(title, refresh, summary, "Job log", displayed_output)

    def job_response(self, parts: list[str]) -> tuple[int, str, bytes]:
        """Return one validated aggregate-job dashboard response."""

        job = self.job_name(parts[4])
        log_path = self.latest_log(int(parts[1]), int(parts[2]))
        if job is None or log_path is None:
            return 404, "text/plain; charset=utf-8", b"Job log not found\n"
        contents = log_path.read_text(encoding="utf-8", errors="replace")
        body = self.render_job_dashboard(log_path, contents, job).encode("utf-8")
        return 200, "text/html; charset=utf-8", body

    def dashboard_response(self, parts: list[str]) -> tuple[int, str, bytes]:
        """Return one validated patch-set dashboard response."""

        log_path = self.latest_log(int(parts[1]), int(parts[2]))
        if log_path is None:
            return 404, "text/plain; charset=utf-8", b"Log not found\n"
        contents = log_path.read_text(encoding="utf-8", errors="replace")
        body = self.render_dashboard(log_path, contents).encode("utf-8")
        return 200, "text/html; charset=utf-8", body

    def raw_log_response(self, name: str) -> tuple[int, str, bytes] | None:
        """Return one validated retained raw log or no route match."""

        if self.LOG_NAME.fullmatch(name) is None:
            return None
        log_path = self.log_directory / name
        if not log_path.is_file():
            return None
        return 200, "text/plain; charset=utf-8", log_path.read_bytes()

    def response(self, request_path: str) -> tuple[int, str, bytes]:
        """Route one read-only health, dashboard, job, or raw-log request."""

        if request_path == "/health":
            return 200, "text/plain; charset=utf-8", b"ok\n"
        parts = request_path.strip("/").split("/")
        digits = len(parts) >= 3 and all(value.isdigit() for value in parts[1:3])
        if len(parts) == 5 and parts[0] == "changes" and parts[3] == "jobs" and digits:
            return self.job_response(parts)
        if len(parts) == 3 and parts[0] == "changes" and digits:
            return self.dashboard_response(parts)
        if len(parts) == 2 and parts[0] == "logs":
            raw_response = self.raw_log_response(parts[1])
            if raw_response is not None:
                return raw_response
        return 404, "text/plain; charset=utf-8", b"Not found\n"

    def start(self) -> None:
        """Start the read-only HTTP server on one daemon thread."""

        self.server = ThreadingHTTPServer((self.host, self.port), XWalkLogRequestHandler)
        self.server.log_server = self  # type: ignore[attr-defined]
        self.thread = threading.Thread(
            target=self.server.serve_forever,
            name="xwalk-gerrit-log-server",
            daemon=True,
        )
        self.thread.start()


class XWalkLogRequestHandler(BaseHTTPRequestHandler):
    """Serve bounded read-only responses from the owning log server."""

    def send_content(self, status: int, content_type: str, body: bytes) -> None:
        """Write one bounded HTTP response with restrictive headers."""

        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self) -> None:
        """Serve one health, dashboard, job, or raw-log GET request."""

        log_server = self.server.log_server  # type: ignore[attr-defined]
        response = log_server.response(unquote(urlparse(self.path).path))
        self.send_content(*response)

    def log_message(self, format_text: str, *arguments: object) -> None:
        """Suppress routine access output from the Gerrit event log."""

        unused = (format_text, arguments)
        del unused
