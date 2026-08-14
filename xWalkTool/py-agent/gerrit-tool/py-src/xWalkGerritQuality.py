#!/usr/bin/env python3
"""Run and record the module-oriented xWalk Host Quality graph."""

from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import threading
import time
from typing import TextIO


@dataclass(frozen=True)
class XWalkCheckPlan:
    """Describe one repository-owned validation command."""

    identifier: str
    name: str
    arguments: tuple[str, ...]


@dataclass(frozen=True)
class XWalkModulePlan:
    """Describe one visible CI module and its dependencies."""

    identifier: str
    name: str
    needs: tuple[str, ...]
    checks: tuple[XWalkCheckPlan, ...]


def check(identifier: str, name: str, *arguments: str) -> XWalkCheckPlan:
    """Create one concise immutable check declaration."""

    return XWalkCheckPlan(identifier, name, arguments)


PREPARATION = XWalkModulePlan(
    "preparation", "xWalk Preparation", (),
    (check("metadata", "Shell scripts and CI metadata", "preparation"),),
)

MODULES = (
    XWalkModulePlan("xwalk-agent", "xWalkAgent", ("preparation",), (
        check("build", "Configure and build xWalkAgent", "xwalk-agent-build"),
        check("aggregate", "Agent aggregate tests", "xwalk-agent-aggregate"),
        check("groups", "Agent functional-group tests", "xwalk-agent-groups"),
        check("functional", "Agent configuration and communication tests", "xwalk-agent-functional"),
    )),
    XWalkModulePlan("xwalk-controller", "xWalkController", ("preparation",), (
        check("build", "Configure and build xWalkController", "xwalk-controller-build"),
        check("tests", "Controller, CLI, and Controller-to-HAL tests", "xwalk-controller-tests"),
        check("diagnostic", "Device-free deployment diagnosis", "xwalk-controller-diagnostic"),
    )),
    XWalkModulePlan("xwalk-hal", "xWalkHal", ("preparation",), (
        check("build", "Configure and build xWalkHal", "xwalk-hal-build"),
        check("unit", "HAL unit and host-safe sequence tests", "xwalk-hal-unit"),
        check("groups", "HAL interface, device, sensor, and layer tests", "xwalk-hal-groups"),
        check("simulation", "Robot HAT and lifecycle simulations", "xwalk-hal-simulation"),
        check("soak", "Device-free Robot HAT soak test", "xwalk-hal-soak"),
    )),
    XWalkModulePlan("xwalk-iw", "xWalkIW", ("preparation",), (
        check("build", "Configure and build xWalkIW", "xwalk-iw-build"),
        check("tests", "Schema, serialization, gRPC, signal, and transport tests", "xwalk-iw-tests"),
    )),
    XWalkModulePlan("xwalk-library", "xWalkLibrary", ("preparation",), (
        check("build", "Configure and build xWalkLibrary consumers", "xwalk-library-build"),
        check(
            "tests", "Shared-library, configuration, utility, licence, and architecture tests",
            "xwalk-library-tests",
        ),
    )),
    XWalkModulePlan("xwalk-trace", "xWalkTrace", ("preparation",), (
        check("tests", "Trace formatting, routing, macro, and selector tests", "xwalk-trace-tests"),
    )),
    XWalkModulePlan("xwalk-vision", "xWalk Vision", ("preparation",), (
        check("build", "Configure and build recorded vision scenarios", "xwalk-vision-build"),
        check("tests", "Recorded media, OpenCV scenarios, safety, and assets", "xwalk-vision-tests"),
    )),
    XWalkModulePlan("xwalk-streaming", "xWalk Streaming", ("preparation",), (
        check("build", "Configure and build xWalk Streaming", "xwalk-streaming-build"),
        check(
            "tests", "Loopback HTTP, MJPEG, lifecycle, timeout, and backpressure tests",
            "xwalk-streaming-tests",
        ),
    )),
    XWalkModulePlan("xwalk-quality", "xWalk Quality", ("preparation",), (
        check("gcc-debug", "GCC Debug build and tests", "build-and-test", "gcc", "Debug"),
        check("gcc-release", "GCC Release build and tests", "build-and-test", "gcc", "Release"),
        check("clang-debug", "Clang Debug build and tests", "build-and-test", "clang", "Debug"),
        check("clang-release", "Clang Release build and tests", "build-and-test", "clang", "Release"),
        check("asan-ubsan", "AddressSanitizer and UndefinedBehaviorSanitizer", "asan-ubsan"),
        check("leak-sanitizer", "LeakSanitizer", "leak-sanitizer"),
        check("thread-sanitizer", "ThreadSanitizer", "thread-sanitizer"),
        check("static-analysis", "Clang-Tidy and Cppcheck", "static-analysis"),
        check("clang-static-analyzer", "Clang Static Analyzer", "clang-static-analyzer"),
        check("stress-tests", "Host stress tests", "stress-tests"),
        check("fuzz-smoke", "Bounded fuzz smoke tests", "fuzz-smoke"),
        check("valgrind", "Valgrind analysis", "valgrind"),
        check("coverage", "GCC and Clang coverage", "coverage"),
    )),
    XWalkModulePlan("xwalk-deployment", "xWalk Deployment", ("preparation",), (
        check("scripts", "Deployment and provisioning scripts", "deployment-scripts"),
        check("staged-install", "Staged installation and artifact validation", "staged-install"),
    )),
    XWalkModulePlan("codescene-code-health", "MyPiCarX / Code Health", ("preparation",), (
        check("delta", "CodeScene changed-code delta analysis", "codescene"),
    )),
)

GATE = XWalkModulePlan(
    "host-quality-gate", "xWalk Host Quality Gate",
    tuple(module.identifier for module in MODULES), (),
)


class XWalkGerritQuality:
    """Execute the shared CI dispatcher and persist live module state."""

    SECRET_ASSIGNMENT = re.compile(
        r"(?i)\b([A-Z0-9_]*(?:PASSWORD|TOKEN|SECRET|PRIVATE_KEY|AUTHORIZATION|COOKIE)[A-Z0-9_]*)=([^\s]+)"
    )
    AUTHENTICATED_URL = re.compile(r"(https?://)[^/@\s:]+:[^/@\s]+@")
    PRIVATE_KEY = re.compile(
        r"-----BEGIN [^-]*PRIVATE KEY-----.*?-----END [^-]*PRIVATE KEY-----",
        re.DOTALL,
    )

    def __init__(
        self, workspace: Path, log: TextIO, environment: dict[str, str] | None = None,
    ) -> None:
        """Store checkout paths and initialize the structured result model."""

        self.workspace = workspace
        self.log = log
        self.environment = os.environ.copy() if environment is None else environment.copy()
        self.dispatcher = workspace / "xWalkTool/shell-agent/gerrit-tool/run-host-ci-job.sh"
        self.lock = threading.Lock()
        log_name = getattr(log, "name", "")
        self.state_path = Path(log_name).with_suffix(".json") if log_name else None
        self.plans = (PREPARATION, *MODULES, GATE)
        self.state = {
            "schema_version": 1,
            "name": "xWalk Host Quality",
            "updated_at": self.timestamp(),
            "jobs": [self.initial_job(plan) for plan in self.plans],
        }
        self.write_state()

    @staticmethod
    def timestamp() -> str:
        """Return an ISO 8601 UTC timestamp."""

        return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")

    @staticmethod
    def initial_job(plan: XWalkModulePlan) -> dict[str, object]:
        """Create one serializable pending module result."""

        return {
            "id": plan.identifier, "name": plan.name, "needs": list(plan.needs),
            "log_link": f"jobs/{plan.identifier}",
            "status": "WAITING", "started_at": None, "completed_at": None,
            "duration_seconds": None,
            "checks": [
                {
                    "id": item.identifier, "name": item.name, "status": "WAITING",
                    "started_at": None, "completed_at": None, "duration_seconds": None,
                }
                for item in plan.checks
            ],
        }

    def write_state(self) -> None:
        """Atomically publish the latest non-secret execution state."""

        if self.state_path is None:
            return
        self.state["updated_at"] = self.timestamp()
        temporary = self.state_path.with_suffix(".json.tmp")
        temporary.write_text(json.dumps(self.state, indent=2) + "\n", encoding="utf-8")
        temporary.replace(self.state_path)

    def job_state(self, identifier: str) -> dict[str, object]:
        """Return one known job state by stable identifier."""

        return next(job for job in self.state["jobs"] if job["id"] == identifier)

    def check_state(self, module: str, identifier: str) -> dict[str, object]:
        """Return one known check state by stable identifiers."""

        job = self.job_state(module)
        return next(item for item in job["checks"] if item["id"] == identifier)

    def update_status(self, target: dict[str, object], status: str, started: float | None = None) -> None:
        """Update one job or check status, timestamps, and elapsed time."""

        now = time.monotonic()
        if status == "RUNNING":
            target["started_at"] = self.timestamp()
        target["status"] = status
        if status in {"PASSED", "FAILED", "SKIPPED", "CANCELLED", "UNAVAILABLE"}:
            target["completed_at"] = self.timestamp()
            if started is not None:
                target["duration_seconds"] = round(now - started, 3)
        self.write_state()

    @classmethod
    def redact(cls, output: str) -> str:
        """Remove common credential forms before retaining command output."""

        output = cls.PRIVATE_KEY.sub("[REDACTED PRIVATE KEY]", output)
        output = cls.AUTHENTICATED_URL.sub(r"\1[REDACTED]@", output)
        return cls.SECRET_ASSIGNMENT.sub(r"\1=[REDACTED]", output)

    def run_check(self, module: XWalkModulePlan, plan: XWalkCheckPlan) -> bool:
        """Run one dispatcher selection and retain its bounded module log section."""

        state = self.check_state(module.identifier, plan.identifier)
        started = time.monotonic()
        with self.lock:
            self.update_status(state, "RUNNING")
        command = [str(self.dispatcher), *plan.arguments]
        with tempfile.TemporaryFile(mode="w+", encoding="utf-8") as output:
            result = subprocess.run(
                command, cwd=self.workspace, stdout=output, stderr=subprocess.STDOUT,
                check=False, text=True, env=self.environment.copy(),
            )
            output.seek(0)
            retained = self.redact(output.read())
        status = "PASSED" if result.returncode == 0 else "FAILED"
        if result.returncode < 0:
            status = "CANCELLED"
        if module.identifier == "codescene-code-health" and result.returncode == 0:
            try:
                summary = json.loads(
                    (self.workspace / "build-host/codescene/summary.json").read_text(
                        encoding="utf-8"
                    )
                )
            except (OSError, json.JSONDecodeError):
                summary = {}
            if summary.get("status") in {"FAILED", "UNAVAILABLE"}:
                status = str(summary["status"])
        with self.lock:
            self.log.write(
                f"\n{'-' * 24} CHECK {module.identifier}/{plan.identifier} {'-' * 24}\n"
                f"$ {' '.join(command)}\n{retained}\n[{status}] {module.identifier}/{plan.identifier}\n"
            )
            self.log.flush()
            self.update_status(state, status, started)
        return result.returncode == 0

    def run_module(self, plan: XWalkModulePlan) -> bool:
        """Run every check in one visible module and retain all failures."""

        job = self.job_state(plan.identifier)
        started = time.monotonic()
        with self.lock:
            self.update_status(job, "RUNNING")
            self.log.write(f"\n{'=' * 24} MODULE {plan.identifier} {'=' * 24}\n")
            self.log.flush()
        passed = True
        for item in plan.checks:
            passed = self.run_check(plan, item) and passed
        cancelled = any(item["status"] == "CANCELLED" for item in job["checks"])
        unavailable = any(item["status"] == "UNAVAILABLE" for item in job["checks"])
        reported_failure = any(item["status"] == "FAILED" for item in job["checks"])
        status = (
            "UNAVAILABLE" if passed and unavailable
            else "FAILED" if passed and reported_failure
            else "PASSED" if passed
            else "CANCELLED" if cancelled
            else "FAILED"
        )
        with self.lock:
            self.update_status(job, status, started)
            self.log.write(f"\n[{status}] {plan.identifier}\n")
            self.log.flush()
        return passed

    @staticmethod
    def worker_count(job_count: int) -> int:
        """Return the bounded module worker count configured for this host."""

        try:
            return max(1, min(job_count, int(os.environ.get("XWALK_CI_MAX_WORKERS", "4"))))
        except ValueError:
            return min(job_count, 4)

    def skip_modules(self, plans: tuple[XWalkModulePlan, ...]) -> None:
        """Mark modules and checks skipped after a failed prerequisite."""

        for plan in plans:
            job = self.job_state(plan.identifier)
            self.update_status(job, "SKIPPED")
            for item in job["checks"]:
                self.update_status(item, "SKIPPED")

    def execute_modules(self, plans: tuple[XWalkModulePlan, ...]) -> dict[str, bool]:
        """Execute independent modules concurrently with bounded host load."""

        results: dict[str, bool] = {}
        for plan in plans:
            self.update_status(self.job_state(plan.identifier), "QUEUED")
        with ThreadPoolExecutor(
            max_workers=self.worker_count(len(plans)), thread_name_prefix="xwalk-module"
        ) as executor:
            pending = {executor.submit(self.run_module, plan): plan for plan in plans}
            for future in as_completed(pending):
                plan = pending[future]
                try:
                    results[plan.identifier] = future.result()
                except Exception as error:  # pragma: no cover - defensive worker boundary
                    with self.lock:
                        self.log.write(f"\n[FAILED] {plan.identifier}: {self.redact(str(error))}\n")
                        self.update_status(self.job_state(plan.identifier), "FAILED")
                    results[plan.identifier] = False
        return results

    def finalize_gate(self, module_results: dict[str, bool]) -> bool:
        """Set the final gate from every required module result."""

        gate = self.job_state(GATE.identifier)
        started = time.monotonic()
        self.update_status(gate, "RUNNING")
        passed = all(module_results.get(identifier, False) for identifier in GATE.needs)
        self.update_status(gate, "PASSED" if passed else "FAILED", started)
        return passed

    def write_summary(self, results: dict[str, bool]) -> None:
        """Write the deterministic module summary retained by legacy readers."""

        self.log.write("\n======================== QUALITY SUMMARY ========================\n")
        for identifier, passed in results.items():
            self.log.write(f"[{'PASSED' if passed else 'FAILED'}] {identifier}\n")
        self.log.flush()

    def run_all(self) -> dict[str, bool]:
        """Run Preparation, resource-safe modules, and the aggregate gate."""

        self.update_status(self.job_state(PREPARATION.identifier), "QUEUED")
        preparation_passed = self.run_module(PREPARATION)
        results = {PREPARATION.identifier: preparation_passed}
        if not preparation_passed:
            self.skip_modules(MODULES)
            module_results = {plan.identifier: False for plan in MODULES}
        else:
            module_results = self.execute_modules(MODULES)
        results.update(module_results)
        results[GATE.identifier] = self.finalize_gate(module_results)
        self.write_summary(results)
        return results
