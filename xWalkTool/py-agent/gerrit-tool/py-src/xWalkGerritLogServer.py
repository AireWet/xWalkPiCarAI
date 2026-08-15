#!/usr/bin/env python3
"""Serve live, module-oriented xWalk Gerrit CI results and logs."""

from __future__ import annotations

from html import escape
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import re
import threading
from typing import Any
from urllib.parse import unquote, urlparse


PAGE_STYLE = """
:root { color-scheme:dark; }
body { background:#0d1117; color:#e6edf3; font-family:Arial,sans-serif; margin:0; }
main { margin:auto; max-width:1240px; padding:24px; }
a { color:#58a6ff; }
a:focus-visible { outline:3px solid #58a6ff; outline-offset:3px; }
.summary,.job-detail { background:#161b22; border:1px solid #30363d; border-radius:8px; padding:20px; }
.ci-graph { display:grid; gap:16px; grid-template-columns:minmax(170px,1fr) auto
  minmax(260px,2fr) auto minmax(190px,1fr); margin:24px 0; }
.graph-stage { align-content:center; display:grid; gap:10px; min-width:0; position:relative; }
.graph-stage h2 { font-size:1rem; margin:0; }
.graph-modules { border-left:2px solid #484f58; border-right:2px solid #484f58; padding:0 20px; }
.graph-node { background:#0d1117; border:2px solid #8b949e; border-radius:8px; color:#e6edf3;
  display:block; min-width:0; overflow-wrap:anywhere; padding:12px; text-decoration:none; }
.graph-node:hover { background:#21262d; }
.graph-node strong,.graph-node small { display:block; }
.status-passed { border-color:#3fb950; } .status-failed { border-color:#f85149; }
.status-running { border-color:#58a6ff; } .status-cancelled { border-color:#d29922; }
.status-unavailable { border-color:#d29922; }
.status-waiting,.status-pending,.status-queued,.status-skipped { border-color:#8b949e; }
.passed { color:#3fb950; } .failed { color:#f85149; } .running { color:#58a6ff; }
.cancelled,.unavailable { color:#d29922; } .waiting,.pending,.queued,.skipped { color:#8b949e; }
.connector { color:#8b949e; font-size:1.5rem; text-align:center; }
.details { display:grid; gap:16px; margin-top:24px; }
.job-detail { scroll-margin-top:16px; }
.job-detail h3 { margin-top:0; }
table { border-collapse:collapse; margin:16px 0; width:100%; }
th,td { border-bottom:1px solid #30363d; padding:9px; text-align:left; }
th:last-child,td:last-child { text-align:right; }
pre { background:#010409; border:1px solid #30363d; border-radius:8px; overflow:auto;
  padding:16px; white-space:pre-wrap; }
@media (max-width:760px) {
  main { padding:12px; }
  .ci-graph { grid-template-columns:1fr; }
  .graph-modules { border-bottom:2px solid #484f58; border-left:0; border-right:0;
    border-top:2px solid #484f58; padding:20px 0; }
  .connector::before { content:'↓'; }
  .connector span { display:none; }
}
"""

VALID_STATUSES = {
    "WAITING", "PENDING", "QUEUED", "RUNNING", "PASSED", "FAILED", "SKIPPED", "CANCELLED",
    "UNAVAILABLE",
}
STATUS_ICONS = {
    "WAITING": "○", "PENDING": "○", "QUEUED": "◷", "RUNNING": "●", "PASSED": "✓",
    "FAILED": "✕", "SKIPPED": "—", "CANCELLED": "!", "UNAVAILABLE": "?",
}


def html_page(title: str, refresh: str, body: str) -> str:
    """Render the shared server-side CI document shell."""

    return (
        "<!doctype html><html lang=\"en\"><head><meta charset=\"utf-8\">"
        '<meta name="viewport" content="width=device-width, initial-scale=1">'
        f"{refresh}<title>{escape(title)}</title><style>{PAGE_STYLE}</style></head>"
        f"<body><main>{body}</main></body></html>"
    )


class XWalkGerritLogServer:
    """Expose retained CI state and logs through a read-only dashboard."""

    LOG_NAME = re.compile(
        r"^change-(?P<change>[0-9]+)-(?P<patch>[0-9]+)-"
        r"(?P<timestamp>[0-9]{8}T[0-9]{6}Z)\.log$"
    )
    RESULT_LINE = re.compile(r"^\[(PASSED|FAILED)\] (.+)$", re.MULTILINE)
    JOB_IDENTIFIER = re.compile(r"^[a-z0-9][a-z0-9-]*$")
    MODULES = (
        ("preparation", "xWalk Preparation"),
        ("xwalk-agent", "xWalkAgent"),
        ("xwalk-controller", "xWalkController"),
        ("xwalk-hal", "xWalkHal"),
        ("xwalk-iw", "xWalkIW"),
        ("xwalk-library", "xWalkLibrary"),
        ("xwalk-trace", "xWalkTrace"),
        ("xwalk-vision", "xWalk Vision"),
        ("xwalk-streaming", "xWalk Streaming"),
        ("xwalk-quality", "xWalk Quality"),
        ("xwalk-deployment", "xWalk Deployment"),
        ("codescene-code-health", "MyPiCarX / Code Health"),
        ("host-quality-gate", "xWalk Host Quality Gate"),
    )
    MODULE_NAMES = dict(MODULES)

    def __init__(self, log_directory: Path, host: str, port: int, public_url: str) -> None:
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
        """Return the stable module-log URL for one known identifier."""

        identifier = job if self.JOB_IDENTIFIER.fullmatch(job) else self.identifier_for_name(job)
        if identifier is None:
            return self.dashboard_url(change, patch_set)
        return f"{self.dashboard_url(change, patch_set)}/jobs/{identifier}"

    def job_links(self, change: int, patch_set: int) -> list[tuple[str, str]]:
        """Return every visible module and its stable log URL."""

        return [
            (name, self.job_url(change, patch_set, identifier))
            for identifier, name in self.MODULES
        ]

    @classmethod
    def identifier_for_name(cls, name: str) -> str | None:
        """Resolve one display name to its stable identifier."""

        return next((identifier for identifier, display in cls.MODULES if display == name), None)

    def latest_log(self, change: int, patch_set: int) -> Path | None:
        """Return the newest retained log matching one exact patch set."""

        prefix = f"change-{change}-{patch_set}-"
        matches = [
            path for path in self.log_directory.glob(f"{prefix}*.log")
            if self.LOG_NAME.fullmatch(path.name) is not None
        ]
        return max(matches, key=lambda path: path.name) if matches else None

    @staticmethod
    def state_path(log_path: Path) -> Path:
        """Return the structured state sidecar for one retained log."""

        return log_path.with_suffix(".json")

    @classmethod
    def normalize_status(cls, value: object) -> str:
        """Map unknown or malformed status input to a safe failure state."""

        status = str(value).upper()
        return status if status in VALID_STATUSES else "FAILED"

    def load_state(self, log_path: Path) -> dict[str, Any] | None:
        """Load and validate the optional structured state sidecar."""

        path = self.state_path(log_path)
        try:
            state = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return None
        if not isinstance(state, dict) or not isinstance(state.get("jobs"), list):
            return None
        jobs = []
        for raw_job in state["jobs"]:
            if not isinstance(raw_job, dict):
                continue
            identifier = raw_job.get("id")
            if not isinstance(identifier, str) or self.JOB_IDENTIFIER.fullmatch(identifier) is None:
                continue
            job = dict(raw_job)
            default_name = self.MODULE_NAMES.get(identifier, identifier)
            job["name"] = str(job.get("name", default_name))
            job["log_link"] = f"jobs/{identifier}"
            job["status"] = self.normalize_status(job.get("status"))
            checks = []
            for raw_check in job.get("checks", []):
                if not isinstance(raw_check, dict):
                    continue
                check_identifier = raw_check.get("id")
                if (
                    not isinstance(check_identifier, str)
                    or self.JOB_IDENTIFIER.fullmatch(check_identifier) is None
                ):
                    continue
                item = dict(raw_check)
                item["name"] = str(item.get("name", check_identifier))
                item["status"] = self.normalize_status(item.get("status"))
                checks.append(item)
            job["checks"] = checks
            jobs.append(job)
        return {**state, "jobs": jobs} if jobs else None

    @classmethod
    def job_results(cls, contents: str) -> dict[str, str]:
        """Extract legacy results for historical logs without a sidecar."""

        return {match.group(2): match.group(1) for match in cls.RESULT_LINE.finditer(contents)}

    @classmethod
    def legacy_status(cls, contents: str) -> str:
        """Return the overall state of one historical text-only log."""

        results = cls.job_results(contents)
        if "QUALITY SUMMARY" not in contents:
            return "RUNNING"
        return "FAILED" if "FAILED" in results.values() else "PASSED"

    @classmethod
    def overall_status(cls, contents: str, state: dict[str, Any] | None = None) -> str:
        """Return the structured final-gate status or legacy overall state."""

        if state is not None:
            gate = next((job for job in state["jobs"] if job["id"] == "host-quality-gate"), None)
            if gate is not None:
                return cls.normalize_status(gate.get("status"))
        return cls.legacy_status(contents)

    @staticmethod
    def format_duration(value: object) -> str:
        """Format an optional duration compactly for a graph node."""

        if not isinstance(value, (int, float)) or value < 0:
            return "Duration unavailable"
        seconds = int(round(value))
        minutes, seconds = divmod(seconds, 60)
        return f"{minutes}m {seconds}s" if minutes else f"{seconds}s"

    def graph_node(self, change: int, patch_set: int, job: dict[str, Any]) -> str:
        """Render one accessible linked dependency node."""

        identifier = escape(str(job["id"]), quote=True)
        name = escape(str(job["name"]))
        status = self.normalize_status(job.get("status"))
        duration = escape(self.format_duration(job.get("duration_seconds")))
        route = self.public_route(f"/changes/{change}/{patch_set}/jobs/{identifier}")
        return (
            f'<a class="graph-node status-{status.lower()}" href="{route}" '
            f'aria-label="{name}: {status}, {duration}">'
            f'<strong><span aria-hidden="true">{STATUS_ICONS[status]}</span> {name}</strong>'
            f'<small class="{status.lower()}">{status} · {duration}</small></a>'
        )

    def render_graph(self, change: int, patch_set: int, state: dict[str, Any]) -> str:
        """Render the responsive three-stage module dependency graph."""

        jobs = {job["id"]: job for job in state["jobs"]}
        preparation = jobs.get("preparation")
        gate = jobs.get("host-quality-gate")
        modules = [
            job for job in state["jobs"]
            if job["id"] not in {"preparation", "host-quality-gate"}
        ]
        preparation_html = self.graph_node(change, patch_set, preparation) if preparation else ""
        gate_html = self.graph_node(change, patch_set, gate) if gate else ""
        module_html = "".join(self.graph_node(change, patch_set, job) for job in modules)
        return (
            '<section id="ci-graph" aria-labelledby="graph-heading">'
            '<h2 id="graph-heading">Module dependency graph</h2>'
            '<p>Preparation must pass before each module; every module must pass before the gate.</p>'
            '<div class="ci-graph" data-layout="preparation-modules-gate">'
            f'<div class="graph-stage" aria-label="Preparation stage">{preparation_html}</div>'
            '<div class="connector" aria-hidden="true"><span>→</span></div>'
            f'<div class="graph-stage graph-modules" aria-label="Module stage">{module_html}</div>'
            '<div class="connector" aria-hidden="true"><span>→</span></div>'
            f'<div class="graph-stage" aria-label="Quality gate stage">{gate_html}</div>'
            '</div></section>'
        )

    def render_details(self, change: int, patch_set: int, state: dict[str, Any]) -> str:
        """Render linked status, duration, and individual checks for every module."""

        sections = []
        for job in state["jobs"]:
            identifier = escape(str(job["id"]), quote=True)
            name = escape(str(job["name"]))
            status = self.normalize_status(job.get("status"))
            checks = self.render_checks(change, patch_set, job)
            module_log = self.public_route(f"/changes/{change}/{patch_set}/jobs/{identifier}")
            sections.append(
                f'<section class="job-detail" id="job-{identifier}"><h3>{name}</h3>'
                f'<p>Status: <strong class="{status.lower()}">{status}</strong> · '
                f'{escape(self.format_duration(job.get("duration_seconds")))}</p>{checks}'
                f'<p><a href="{module_log}">Open module log</a> · '
                '<a href="#ci-graph">Return to graph</a></p></section>'
            )
        return f'<section class="details" aria-label="Module details">{"".join(sections)}</section>'

    def render_checks(self, change: int, patch_set: int, job: dict[str, Any]) -> str:
        """Render one module's linked individual test results."""

        identifier = escape(str(job["id"]), quote=True)
        rows = []
        for item in job.get("checks", []):
            check_status = self.normalize_status(item.get("status"))
            check_identifier = escape(str(item.get("id", "unknown")), quote=True)
            check_route = self.public_route(
                f"/changes/{change}/{patch_set}/jobs/{identifier}#check-{identifier}-{check_identifier}"
            )
            rows.append(
                f'<tr id="check-{identifier}-{check_identifier}"><td>'
                f'<a href="{check_route}">{escape(str(item.get("name", "Unnamed check")))}</a></td>'
                f'<td>{escape(self.format_duration(item.get("duration_seconds")))}</td>'
                f'<td class="{check_status.lower()}">{check_status}</td></tr>'
            )
        if not rows:
            return '<p>Aggregate dependency result; this gate does not execute an independent test.</p>'
        return (
            '<table><thead><tr><th>Check</th><th>Duration</th><th>Status</th></tr></thead>'
            f'<tbody>{"".join(rows)}</tbody></table>'
        )

    @classmethod
    def job_contents(cls, contents: str, identifier: str) -> str:
        """Extract all complete check sections retained for one module."""

        pattern = re.compile(
            rf"^-{{24}} CHECK {re.escape(identifier)}/.*?"
            r"(?=^-{24} CHECK |^={24} MODULE |^={24} QUALITY SUMMARY|\Z)",
            re.MULTILINE | re.DOTALL,
        )
        sections = pattern.findall(contents)
        return "\n".join(section.rstrip() for section in sections) + ("\n" if sections else "")

    def render_dashboard(self, log_path: Path, contents: str) -> str:
        """Render one complete escaped log and module dependency graph."""

        match = self.LOG_NAME.fullmatch(log_path.name)
        assert match is not None
        change = int(match.group("change"))
        patch_set = int(match.group("patch"))
        state = self.load_state(log_path)
        status = self.overall_status(contents, state)
        refresh = (
            '<meta http-equiv="refresh" content="10">'
            if status in {"WAITING", "PENDING", "QUEUED", "RUNNING"} else ""
        )
        raw_url = self.public_route(f"/logs/{escape(log_path.name, quote=True)}")
        summary = (
            '<section class="summary"><h1>xWalk CI</h1>'
            f'<p>Gerrit change {change}, patch set {patch_set}</p>'
            f'<p>Overall status: <strong class="{status.lower()}">{status}</strong></p>'
            f'<p><a href="{raw_url}">Open raw full log</a></p></section>'
        )
        if state is None:
            legacy = (
                '<section class="summary"><h2>Historical CI result</h2>'
                '<p>This run predates structured module state. The complete log remains available.</p></section>'
            )
            graph_and_details = legacy
        else:
            graph_and_details = (
                self.render_graph(change, patch_set, state)
                + self.render_details(change, patch_set, state)
            )
        complete = f'<h2>Complete log</h2><pre>{escape(contents)}</pre>'
        return html_page(
            f"xWalk CI change {change}, patch set {patch_set}",
            refresh,
            summary + graph_and_details + complete,
        )

    def render_job_dashboard(
        self, log_path: Path, contents: str, identifier: str, state: dict[str, Any]
    ) -> str:
        """Render one module's checks and escaped retained output."""

        match = self.LOG_NAME.fullmatch(log_path.name)
        assert match is not None
        job = next(job for job in state["jobs"] if job["id"] == identifier)
        status = self.normalize_status(job.get("status"))
        refresh = (
            '<meta http-equiv="refresh" content="10">'
            if status in {"WAITING", "PENDING", "QUEUED", "RUNNING"} else ""
        )
        change = int(match.group("change"))
        patch_set = int(match.group("patch"))
        dashboard = self.public_route(f"/changes/{change}/{patch_set}")
        output = self.job_contents(contents, identifier)
        if not output and identifier == "host-quality-gate":
            output = "The quality gate status is calculated from its module dependencies.\n"
        elif not output and status in {"WAITING", "PENDING", "QUEUED", "SKIPPED"}:
            output = f"Module is {status.lower()} and has not produced log output.\n"
        elif not output and status == "CANCELLED":
            output = "Module was cancelled before retained log output was available.\n"
        elif not output:
            output = "No retained module log output is available.\n"
        checks = self.render_checks(change, patch_set, job)
        body = (
            f'<section class="summary"><h1>{escape(str(job["name"]))}</h1>'
            f'<p>Gerrit change {change}, patch set {patch_set}</p>'
            f'<p>Status: <strong class="{status.lower()}">{status}</strong> · '
            f'{escape(self.format_duration(job.get("duration_seconds")))}</p>'
            f'<p><a href="{dashboard}">Back to module graph and complete log</a></p></section>'
            f'<section class="job-detail"><h2>Individual tests</h2>{checks}</section>'
            f'<h2>Module log</h2><pre>{escape(output)}</pre>'
        )
        return html_page(f'xWalk CI {job["name"]}', refresh, body)

    def job_response(self, parts: list[str]) -> tuple[int, str, bytes]:
        """Return one validated module dashboard response."""

        identifier = parts[4]
        log_path = self.latest_log(int(parts[1]), int(parts[2]))
        if identifier not in self.MODULE_NAMES or log_path is None:
            return 404, "text/plain; charset=utf-8", b"Module log not found\n"
        state = self.load_state(log_path)
        if state is None or not any(job["id"] == identifier for job in state["jobs"]):
            return 404, "text/plain; charset=utf-8", b"Module log not found\n"
        contents = log_path.read_text(encoding="utf-8", errors="replace")
        body = self.render_job_dashboard(log_path, contents, identifier, state).encode("utf-8")
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
        """Route one read-only health, dashboard, module, or raw-log request."""

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
            target=self.server.serve_forever, name="xwalk-gerrit-log-server", daemon=True,
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
        """Serve one health, dashboard, module, or raw-log GET request."""

        log_server = self.server.log_server  # type: ignore[attr-defined]
        response = log_server.response(unquote(urlparse(self.path).path))
        self.send_content(*response)

    def log_message(self, format_text: str, *arguments: object) -> None:
        """Suppress routine access output from the Gerrit event log."""

        unused = (format_text, arguments)
        del unused
