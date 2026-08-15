#!/usr/bin/env python3
"""Verify Gerrit patch sets and synchronize a submitted xWalk integration."""

from __future__ import annotations

from datetime import datetime, timezone
import json
import os
from pathlib import Path
import shlex
import shutil
import signal
import subprocess
import tempfile
import time
from typing import Any, TextIO

from xWalkGerritLogServer import XWalkGerritLogServer
from xWalkGerritQuality import XWalkGerritQuality


class XWalkGerritCi:
    """Own Gerrit verification, integrated uplift submission, and GitHub synchronization."""

    integration_repositories = {"xWalk-rpi5", "xWalkPiCarAI"}
    component_repositories = {
        "DevloperNote", "xWalkAgent", "xWalkAudioResources", "xWalkController",
        "xWalkHal", "xWalkIW", "xWalkLibrary", "xWalkTrace",
    }
    repositories = {
        "DevloperNote", "xWalkAgent", "xWalkAudioResources", "xWalkController",
        "xWalkHal", "xWalkIW", "xWalkLibrary", "xWalkTrace", "xWalk-rpi5-sim",
        "xWalk-rpi5",
    }

    @staticmethod
    def terminate_on_signal(signal_number: int, unused_frame: object) -> None:
        """Convert an orderly service signal into a catchable process exit."""

        del unused_frame
        raise SystemExit(128 + signal_number)

    def __init__(self) -> None:
        """Load the fixed service configuration from the server-user environment."""

        home = Path.home()
        self.configure_gerrit(home)
        self.configure_github(home)
        self.configure_storage(home)

    def configure_gerrit(self, home: Path) -> None:
        """Load Gerrit endpoint, project, and identity settings."""

        self.host = os.environ.get("GERRIT_HOST", "127.0.0.1")
        self.port = os.environ.get("GERRIT_SSH_PORT", "29418")
        self.user = os.environ.get("GERRIT_USER", "xwalk-ci")
        self.project = os.environ.get("GERRIT_PROJECT", "xWalkPiCarAI")
        self.branch = os.environ.get("GERRIT_BRANCH", "master")
        self.web_url = os.environ.get(
            "GERRIT_WEB_URL", os.environ.get("GERRIT_HTTP_BASE_URL", f"https://{self.host}")
        ).rstrip("/")
        self.verification_targets = self.parse_verification_targets(
            os.environ.get("GERRIT_VERIFICATION_TARGETS", ""),
            (self.project, self.branch),
        )
        self.private_key = Path(
            os.environ.get("GERRIT_SSH_KEY", str(home / ".ssh" / "id_ed25519_xwalk_ci"))
        ).expanduser()
        self.uplift_enabled = os.environ.get("GERRIT_UPLIFT_ENABLED", "false") == "true"
        self.integration_project = os.environ.get(
            "GERRIT_INTEGRATION_PROJECT", "xWalkPiCarAI"
        )
        self.integration_branch = os.environ.get("GERRIT_INTEGRATION_BRANCH", "master")
        self.auto_submit = os.environ.get("GERRIT_UPLIFT_AUTO_SUBMIT", "false") == "true"
        self.auto_review = os.environ.get("GERRIT_UPLIFT_AUTO_REVIEW", "false") == "true"
        self.review_user = os.environ.get("GERRIT_REVIEW_USER", "")
        self.review_private_key = Path(
            os.environ.get(
                "GERRIT_REVIEW_SSH_KEY",
                str(home / ".ssh" / "id_ed25519_xwalk_uplift_reviewer"),
            )
        ).expanduser()
        self.retry_attempts = int(os.environ.get("GERRIT_CI_RETRY_ATTEMPTS", "3"))
        self.retry_delay_seconds = int(os.environ.get("GERRIT_CI_RETRY_DELAY_SECONDS", "5"))
        self.uplift_script = Path(
            os.environ.get(
                "GERRIT_UPLIFT_SCRIPT",
                str(Path(__file__).parents[1] / "shell-script" / "gerrit-auto-uplift.sh"),
            )
        ).expanduser()

    @staticmethod
    def parse_verification_targets(
        configured: str, primary: tuple[str, str],
    ) -> set[tuple[str, str]]:
        """Return configured project and branch pairs plus the primary target."""

        targets = {primary}
        for item in configured.split(","):
            item = item.strip()
            if not item:
                continue
            project, separator, branch = item.partition(":")
            if not separator or not project or not branch:
                raise SystemExit(
                    "GERRIT_VERIFICATION_TARGETS entries must use project:branch"
                )
            targets.add((project, branch))
        return targets

    def configure_github(self, home: Path) -> None:
        """Load GitHub mirror endpoint and identity settings."""

        self.github_source_project = os.environ.get(
            "GITHUB_SYNC_SOURCE_PROJECT", self.project
        )
        self.github_source_branch = os.environ.get(
            "GITHUB_SYNC_SOURCE_BRANCH", self.branch
        )
        self.github_remote = os.environ.get(
            "GITHUB_INTEGRATION_REMOTE",
            os.environ.get("GITHUB_XWALK_RPI5_REMOTE", ""),
        )
        self.github_web_url = os.environ.get(
            "GITHUB_INTEGRATION_WEB_URL",
            os.environ.get("GITHUB_XWALK_RPI5_WEB_URL", ""),
        ).rstrip("/")
        self.github_branch = os.environ.get(
            "GITHUB_INTEGRATION_BRANCH",
            os.environ.get("GITHUB_XWALK_RPI5_BRANCH", self.github_source_branch),
        )
        self.github_push_enabled = os.environ.get("GITHUB_PUSH_ENABLED", "false") == "true"
        self.github_direct_push_owner_email = os.environ.get(
            "GITHUB_DIRECT_PUSH_OWNER_EMAIL",
            os.environ.get("GITHUB_PRIMARY_MERGER_EMAIL", ""),
        ).casefold()
        self.github_private_key = Path(
            os.environ.get(
                "GITHUB_SSH_KEY", str(home / ".ssh" / "id_ed25519_xwalk_github_mirror")
            )
        ).expanduser()

    def configure_storage(self, home: Path) -> None:
        """Load and create user-owned state, logs, and dashboard settings."""

        self.state_directory = Path(
            os.environ.get("XWALK_CI_STATE_DIRECTORY", str(home / "gerrit-ci" / "state"))
        ).expanduser()
        self.log_directory = Path(
            os.environ.get("XWALK_CI_LOG_DIRECTORY", str(home / "gerrit-ci" / "logs"))
        ).expanduser()
        self.state_directory.mkdir(parents=True, exist_ok=True)
        self.log_directory.mkdir(parents=True, exist_ok=True)
        self.changelog_path = Path(
            os.environ.get(
                "XWALK_UPLIFT_CHANGELOG", str(self.state_directory / "uplift-changelog.jsonl")
            )
        ).expanduser()
        recovered_files = XWalkGerritQuality.reconcile_interrupted_states(
            self.log_directory
        )
        if recovered_files:
            print(
                f"Cancelled interrupted checks in {recovered_files} retained CI run(s)",
                flush=True,
            )
        self.log_server = XWalkGerritLogServer(
            self.log_directory,
            os.environ.get("XWALK_CI_LOG_HTTP_HOST", "127.0.0.1"),
            int(os.environ.get("XWALK_CI_LOG_HTTP_PORT", "8091")),
            os.environ.get("XWALK_CI_LOG_WEB_URL", "http://127.0.0.1:8091"),
        )

    @staticmethod
    def validate_private_key(path: Path) -> None:
        """Require one current-user private key with no group or other access."""

        if not path.is_file():
            raise SystemExit(f"Missing private key: {path}")
        metadata = path.stat()
        if metadata.st_uid != os.getuid() or metadata.st_mode & 0o077:
            raise SystemExit(f"Private key must be owned by this user with mode 0600: {path}")

    def ssh_arguments(self) -> list[str]:
        """Return noninteractive SSH arguments shared by Git and Gerrit commands."""

        return [
            "ssh",
            "-i",
            str(self.private_key),
            "-o",
            "BatchMode=yes",
            "-o",
            "IdentitiesOnly=yes",
            "-o",
            "ServerAliveInterval=30",
            "-o",
            "ServerAliveCountMax=3",
            "-o",
            "StrictHostKeyChecking=yes",
            "-o",
            f"UserKnownHostsFile={self.state_directory / 'known_hosts'}",
            "-l",
            self.user,
            "-p",
            self.port,
            f"{self.user}@{self.host}",
        ]

    def git_environment(self) -> dict[str, str]:
        """Return a Git environment that uses the dedicated Gerrit SSH identity."""

        environment = os.environ.copy()
        ssh_command = self.ssh_arguments()[:-1]
        environment["GIT_SSH_COMMAND"] = shlex.join(ssh_command)
        return environment

    def review_ssh_arguments(self) -> list[str]:
        """Return SSH arguments for the optional, separate automatic reviewer."""

        arguments = self.ssh_arguments()
        key_index = arguments.index("-i") + 1
        login_index = arguments.index("-l") + 1
        arguments[key_index] = str(self.review_private_key)
        arguments[login_index] = self.review_user
        arguments[-1] = f"{self.review_user}@{self.host}"
        return arguments

    def run_review_ssh(self, command: str) -> subprocess.CompletedProcess[str]:
        """Run one command as the isolated automatic-review service account."""

        return subprocess.run(
            [*self.review_ssh_arguments(), command], check=False, text=True
        )

    def github_git_environment(self) -> dict[str, str]:
        """Return a Git environment using the repository-scoped deploy key."""

        environment = os.environ.copy()
        ssh_command = [
            "ssh",
            "-i",
            str(self.github_private_key),
            "-o",
            "BatchMode=yes",
            "-o",
            "IdentitiesOnly=yes",
            "-o",
            "StrictHostKeyChecking=yes",
            "-o",
            f"UserKnownHostsFile={self.state_directory / 'github_known_hosts'}",
        ]
        environment["GIT_SSH_COMMAND"] = shlex.join(ssh_command)
        return environment

    def report(self, change: int, patch_set: int, vote: int, message: str) -> bool:
        """Post one Verified vote and patch-set message through Gerrit SSH."""

        target = f"{change},{patch_set}"
        command = " ".join(
            [
                "gerrit review",
                f"--label Verified={vote:+d}",
                "--tag autogenerated:xwalk-ci",
                f"--message {shlex.quote(message)}",
                "--notify NONE",
                shlex.quote(target),
            ]
        )
        result = self.run_ssh(command)
        return result.returncode == 0

    def post_message(
        self, change: int, patch_set: int, message: str,
        tag: str = "autogenerated:xwalk-ci",
        notify: str = "NONE",
    ) -> bool:
        """Post one informational CI message without changing a label vote."""

        if notify not in {"NONE", "OWNER", "OWNER_REVIEWERS", "ALL"}:
            raise ValueError(f"Unsupported Gerrit notification level: {notify}")
        target = f"{change},{patch_set}"
        command = " ".join(
            [
                "gerrit review",
                f"--tag {shlex.quote(tag)}",
                f"--message {shlex.quote(message)}",
                f"--notify {notify}",
                shlex.quote(target),
            ]
        )
        result = self.run_ssh(command)
        return result.returncode == 0

    def run_ssh(self, command: str, capture_output: bool = False) -> subprocess.CompletedProcess[str]:
        """Retry one noninteractive Gerrit command after temporary SSH failures."""

        attempts = getattr(self, "retry_attempts", 3)
        delay = getattr(self, "retry_delay_seconds", 5)
        result: subprocess.CompletedProcess[str] | None = None
        for attempt in range(1, attempts + 1):
            result = subprocess.run(
                [*self.ssh_arguments(), command], check=False, text=True,
                stdout=subprocess.PIPE if capture_output else None,
                stderr=subprocess.DEVNULL if capture_output else None,
            )
            if result.returncode == 0 or result.returncode not in {128, 255}:
                return result
            if attempt < attempts:
                print(f"Temporary Gerrit SSH failure; retrying ({attempt}/{attempts})", flush=True)
                time.sleep(delay)
        assert result is not None
        return result

    def append_changelog(
        self, module: str, operation: str, source_change: str, source_commit: str,
        integrated_change: str, integrated_commit: str, result: str,
        explanation: str, link: str,
    ) -> None:
        """Append one non-secret operation record to the service audit log."""

        entry = {
            "timestamp": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
            "module": module,
            "operation": operation,
            "source_change": source_change,
            "source_commit": source_commit,
            "integrated_change": integrated_change,
            "integrated_commit": integrated_commit,
            "result": result,
            "explanation": explanation,
            "link": link,
        }
        self.changelog_path.parent.mkdir(parents=True, exist_ok=True)
        descriptor = os.open(
            self.changelog_path, os.O_APPEND | os.O_CREAT | os.O_WRONLY, 0o600
        )
        try:
            os.write(descriptor, (json.dumps(entry, sort_keys=True) + "\n").encode())
        finally:
            os.close(descriptor)

    @staticmethod
    def run_command(
        command: list[str], working_directory: Path, log: TextIO,
        environment: dict[str, str] | None = None,
    ) -> bool:
        """Run one verification command while appending combined output to its log."""

        log.write(f"\n$ {shlex.join(command)}\n")
        log.flush()
        result = subprocess.run(
            command,
            cwd=working_directory,
            env=environment,
            stdout=log,
            stderr=subprocess.STDOUT,
            check=False,
            text=True,
        )
        return result.returncode == 0

    @staticmethod
    def command_output(
        command: list[str], working_directory: Path, log: TextIO,
        environment: dict[str, str] | None = None,
    ) -> str:
        """Run one inspection command and return stripped output on success."""

        log.write(f"\n$ {shlex.join(command)}\n")
        log.flush()
        result = subprocess.run(
            command,
            cwd=working_directory,
            env=environment,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
            text=True,
        )
        log.write(result.stdout)
        log.flush()
        return result.stdout.strip() if result.returncode == 0 else ""

    def run_network_command(
        self, command: list[str], working_directory: Path, log: TextIO,
        environment: dict[str, str] | None = None,
    ) -> bool:
        """Retry only Git commands whose exit status indicates transport failure."""

        attempts = getattr(self, "retry_attempts", 3)
        delay = getattr(self, "retry_delay_seconds", 5)
        for attempt in range(1, attempts + 1):
            log.write(f"\n$ {shlex.join(command)}\n")
            log.flush()
            result = subprocess.run(
                command, cwd=working_directory, env=environment,
                stdout=log, stderr=subprocess.STDOUT, check=False, text=True,
            )
            if result.returncode == 0:
                return True
            if result.returncode not in {128, 255} or attempt == attempts:
                return False
            log.write(f"Temporary network failure; retrying ({attempt}/{attempts})\n")
            log.flush()
            time.sleep(delay)
        return False

    def validate_github_destination(self) -> bool:
        """Accept only an explicitly enabled integrated-project destination."""

        if (
            not self.github_push_enabled or not self.github_remote
            or (self.github_source_project, self.github_source_branch) not in {
                ("xWalk-rpi5", "main"), ("xWalkPiCarAI", "master"),
            }
            or self.github_branch != self.github_source_branch
            or not (
                self.github_remote.startswith("git@")
                or self.github_remote.startswith("ssh://")
            )
        ):
            return False
        repository = self.github_remote.rstrip("/").rsplit("/", 1)[-1].rsplit(":", 1)[-1]
        return repository.removesuffix(".git") == self.github_source_project

    def announce_verification(self, change: int, patch_set: int, revision: str) -> str:
        """Post the live graph link before running one patch-set quality gate."""

        results_url = self.log_server.dashboard_url(change, patch_set)
        message = "\n".join([
            "xWalk host verification started", f"Overall results and full log: {results_url}",
            "Each module will post a separate result in the change log.",
        ])
        print(f"Verifying change {change}, patch set {patch_set}: {revision}", flush=True)
        self.report(change, patch_set, 0, message)
        return results_url

    def checkout_commands(
        self, ref: str, verification_project: str | None = None,
    ) -> list[tuple[list[str], dict[str, str] | None]]:
        """Build the isolated Gerrit patch-set checkout commands."""

        target_project = verification_project or self.project
        project = target_project if target_project in self.integration_repositories else getattr(
            self, "integration_project", "xWalkPiCarAI"
        )
        integration_branch = getattr(self, "integration_branch", "master")
        remote = f"ssh://{self.user}@{self.host}:{self.port}/{project}"
        commands: list[tuple[list[str], dict[str, str] | None]] = [
            (["git", "init", "--quiet"], None),
            (["git", "remote", "add", "origin", remote], None),
            (
                [
                    "git", "fetch", "origin",
                    ref if project == target_project else integration_branch,
                ],
                self.git_environment(),
            ),
            (["git", "checkout", "--detach", "FETCH_HEAD"], None),
        ]
        if project != target_project:
            overlay = f".xwalk-overlay-{target_project}"
            component_path = "devloper-note" if target_project == "DevloperNote" else target_project
            component_remote = f"ssh://{self.user}@{self.host}:{self.port}/{target_project}"
            commands.extend([
                (["git", "init", "--quiet", overlay], None),
                (["git", "-C", overlay, "remote", "add", "origin", component_remote], None),
                (["git", "-C", overlay, "fetch", "origin", ref], self.git_environment()),
                (["git", "-C", overlay, "checkout", "--detach", "FETCH_HEAD"], None),
                (
                    [
                        "xWalkTool/shell-agent/gerrit-tool/overlay-gerrit-component.sh",
                        overlay, f"xWalk-rpi5/{component_path}",
                    ],
                    None,
                ),
            ])
        return commands

    def standalone_commands(
        self, directory: Path, verification_project: str | None = None,
    ) -> list[list[str]]:
        """Return the module-specific validation commands before integration CI."""

        target_project = verification_project or self.project
        if target_project in self.integration_repositories:
            return []
        component_path = "devloper-note" if target_project == "DevloperNote" else target_project
        module = directory / "xWalk-rpi5" / component_path
        code_repositories = {
            "xWalkAgent", "xWalkController", "xWalkHal", "xWalkIW", "xWalkLibrary", "xWalkTrace",
        }
        if target_project in code_repositories:
            build = directory / "build-host" / f"standalone-{target_project}"
            module_options = {
                "xWalkAgent": "-DXWALK_AGENT_BUILD_HOST=ON",
                "xWalkController": "-DXWALK_CLI_BUILD_HOST=ON",
                "xWalkIW": "-DXWALK_IW_BUILD_HOST_TESTS=ON",
                "xWalkTrace": "-DXWALK_TRACE_BUILD_HOST_TESTS=ON",
            }
            options = ["-DXWALK_STANDALONE_BUILD=ON"]
            if target_project in module_options:
                options.append(module_options[target_project])
            return [
                [
                    "cmake", "--fresh", "-S", str(module), "-B", str(build), "-G", "Ninja",
                    "-DCMAKE_BUILD_TYPE=Debug", *options,
                ],
                ["cmake", "--build", str(build), "--parallel"],
                ["ctest", "--test-dir", str(build), "--output-on-failure", "--no-tests=error"],
            ]
        return [["git", "-C", str(module), "diff", "--check", "HEAD^"]]

    def execute_verification(
        self, ref: str, directory: Path, log_path: Path,
        verification_project: str | None = None,
        verification_environment: dict[str, str] | None = None,
    ) -> tuple[bool, dict[str, bool]]:
        """Checkout one patch set and execute the unchanged quality matrix."""

        target_project = verification_project or self.project
        with log_path.open("w", encoding="utf-8") as log:
            checkout_passed = all(
                self.run_command(command, directory, log, environment)
                for command, environment in self.checkout_commands(ref, target_project)
            )
            standalone_passed = checkout_passed and all(
                self.run_command(command, directory, log)
                for command in self.standalone_commands(directory, target_project)
            )
            results = (
                XWalkGerritQuality(
                    directory, log, verification_environment
                ).run_all() if standalone_passed else {}
            )
            if target_project not in self.integration_repositories:
                results = {f"{target_project} standalone": standalone_passed, **results}
            report_directory = directory / "build-host/codescene"
            if report_directory.is_dir():
                shutil.copytree(
                    report_directory, log_path.with_suffix(".codescene"), dirs_exist_ok=True,
                )
        return checkout_passed and all(results.values()), results

    @classmethod
    def code_health_environment(cls, event: dict[str, Any]) -> dict[str, str]:
        """Build exact, non-secret Gerrit patchset metadata for CodeScene."""

        revision = str(event["patchSet"]["revision"])
        project = str(event["change"]["project"])
        environment = {
            **os.environ,
            "GERRIT_CHANGE_NUMBER": str(event["change"]["number"]),
            "GERRIT_PATCHSET_NUMBER": str(event["patchSet"]["number"]),
            "GERRIT_PATCHSET_REVISION": revision,
            "GERRIT_REFSPEC": str(event["patchSet"]["ref"]),
            "GERRIT_PROJECT": project,
            "GERRIT_BRANCH": str(event["change"]["branch"]),
            "XWALK_CODESCENE_BASE_REVISION": f"{revision}^",
            "XWALK_CODESCENE_REVISION": revision,
        }
        environment.pop("XWALK_CODESCENE_UNAVAILABLE_REASON", None)
        if project not in cls.integration_repositories:
            environment["XWALK_CODESCENE_UNAVAILABLE_REASON"] = (
                "This component patch set is verified in its overlaid integration checkout, "
                "but the installed CodeScene project must analyse the integrated MyPiCarX "
                "Git history after uplift or through licensed native Gerrit integration."
            )
        return environment

    def report_module_results(
        self, change: int, patch_set: int, log_path: Path, results_url: str,
    ) -> bool:
        """Post one uniquely tagged Gerrit change-log entry per completed module."""

        state = self.log_server.load_state(log_path)
        if state is None:
            return False
        reported = True
        for job in state["jobs"]:
            identifier = str(job["id"])
            if identifier == "host-quality-gate":
                continue
            status = self.log_server.normalize_status(job.get("status"))
            checks = list(job.get("checks", []))
            completed = sum(
                self.log_server.normalize_status(item.get("status")) == "PASSED"
                for item in checks
            )
            lines = [
                str(job["name"]), f"Status: {status}",
                f"Duration: {self.log_server.format_duration(job.get('duration_seconds'))}",
                f"Tests passed: {completed}/{len(checks)}",
            ]
            if identifier == "codescene-code-health":
                summary_path = log_path.with_suffix(".codescene") / "summary.json"
                try:
                    summary = json.loads(summary_path.read_text(encoding="utf-8"))
                except (OSError, json.JSONDecodeError):
                    summary = {}
                changed_count = summary.get("changed_files_analysed")
                if not isinstance(changed_count, int):
                    changed_count = "unavailable"
                lines.extend([
                    f"Changed files analysed: {changed_count}",
                    f"Code-health degradation: {summary.get('code_health_degradation', 'UNKNOWN')}",
                    f"Quality gate: {summary.get('quality_gate', 'UNAVAILABLE')}",
                ])
                if summary.get("analysis_link"):
                    lines.append(f"Analysis: {summary['analysis_link']}")
                if summary.get("reason"):
                    lines.append(f"Diagnostic: {summary['reason']}")
            message = "\n".join([
                *lines,
                f"Module tests and log: {self.log_server.job_url(change, patch_set, identifier)}",
                f"Overall dependency graph: {results_url}",
            ])
            accepted = self.post_message(
                change, patch_set, message, f"autogenerated:xwalk-ci:{identifier}"
            )
            reported = accepted and reported
        return reported

    def verification_message(
        self, change: int, patch_set: int, passed: bool,
        results: dict[str, bool], results_url: str, log_path: Path,
    ) -> tuple[int, str]:
        """Build the final-gate Gerrit vote after separate module reports."""

        if passed:
            lines = [
                "Complete xWalk host quality gate passed",
                f"Jobs: {len(results)}/{len(results)}",
            ]
            details = [
                "Module results are listed separately in the change log.",
                f"Overall results and full log: {results_url}", f"Log: {log_path.name}",
            ]
            return 1, "\n".join([*lines, *details])
        failed = ", ".join(name for name, value in results.items() if not value) or "checkout"
        lines = ["Complete xWalk host quality gate failed", f"Failed jobs: {failed}"]
        details = [
            "Module results are listed separately in the change log.",
            f"Overall results and full log: {results_url}", f"Log: {log_path.name}",
        ]
        return -1, "\n".join([*lines, *details])

    def verify(self, event: dict[str, Any]) -> None:
        """Fetch, build, test, and report one matching Gerrit patch set."""

        change = int(event["change"]["number"])
        patch_set = int(event["patchSet"]["number"])
        revision = str(event["patchSet"]["revision"])
        ref = str(event["patchSet"]["ref"])
        project = str(event["change"]["project"])
        timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        log_path = self.log_directory / f"change-{change}-{patch_set}-{timestamp}.log"
        log_path.touch(exist_ok=False)
        results_url = self.announce_verification(change, patch_set, revision)

        temporary_directory = Path(
            tempfile.mkdtemp(prefix=f"xwalk-gerrit-{change}-{patch_set}-")
        )
        try:
            passed, quality_results = self.execute_verification(
                ref, temporary_directory, log_path, project,
                self.code_health_environment(event),
            )
        finally:
            shutil.rmtree(temporary_directory, ignore_errors=True)

        vote, message = self.verification_message(
            change, patch_set, passed, quality_results, results_url, log_path
        )
        modules_reported = self.report_module_results(
            change, patch_set, log_path, results_url
        )
        reported = self.report(change, patch_set, vote, message)
        self.append_changelog(
            project, "CI", f"{change},{patch_set}", revision,
            str(change) if project in self.integration_repositories else "pending-uplift",
            revision if project in self.integration_repositories else "not-created",
            "success" if passed and reported else "failed",
            message.splitlines()[0], results_url,
        )
        self.append_changelog(
            project, "vote", f"{change},{patch_set}", revision,
            str(change), revision, "success" if reported else "failed",
            f"Requested Verified {vote:+d} on the exact tested patch set", results_url,
        )
        print(
            f"{message}; module reports accepted={modules_reported}; "
            f"Gerrit gate report accepted={reported}", flush=True,
        )
        if passed and reported and project == getattr(self, "integration_project", "xWalkPiCarAI"):
            self.submit_if_ready(event)

    def mirror_commands(
        self, revision: str,
    ) -> list[tuple[list[str], dict[str, str] | None]]:
        """Build Gerrit and GitHub setup commands for one submitted revision."""

        gerrit = (
            f"ssh://{self.user}@{self.host}:{self.port}/"
            f"{self.github_source_project}"
        )
        source_ref = (
            f"{self.github_source_branch}:refs/remotes/gerrit/"
            f"{self.github_source_branch}"
        )
        return [
            (["git", "init", "--quiet"], None),
            (["git", "remote", "add", "gerrit", gerrit], None),
            (
                ["git", "fetch", "gerrit", source_ref],
                self.git_environment(),
            ),
            (["git", "remote", "add", "github", self.github_remote], None),
        ]

    def execute_mirror(
        self, revision: str, unused_owner_email: str, directory: Path, log_path: Path,
    ) -> tuple[bool, str]:
        """Fetch and publish one submitted Gerrit revision without force push."""

        with log_path.open("w", encoding="utf-8") as log:
            setup_passed = all(
                (
                    self.run_network_command(command, directory, log, environment)
                    if command[:2] == ["git", "fetch"]
                    else self.run_command(command, directory, log, environment)
                )
                for command, environment in self.mirror_commands(revision)
            )
            branch = self.github_branch
            if not setup_passed:
                return False, branch
            submitted_ref = f"refs/remotes/gerrit/{self.github_source_branch}"
            submitted = self.command_output(
                ["git", "rev-parse", submitted_ref], directory, log
            )
            if submitted != revision:
                log.write(
                    "Submitted revision is not the current configured Gerrit "
                    "integration tip\n"
                )
                return False, branch
            github_tip = self.command_output(
                ["git", "ls-remote", "github", f"refs/heads/{self.github_branch}"],
                directory, log, self.github_git_environment(),
            ).split()
            if github_tip and github_tip[0] == revision:
                log.write("GitHub already contains the exact merged Gerrit commit\n")
                return True, branch
            command = [
                "git", "push", "github",
                f"{submitted_ref}:refs/heads/{self.github_branch}",
            ]
            pushed = self.run_network_command(
                command, directory, log, self.github_git_environment()
            )
            if not pushed:
                return False, branch
            confirmed = self.command_output(
                ["git", "ls-remote", "github", f"refs/heads/{self.github_branch}"],
                directory, log, self.github_git_environment(),
            ).split()
            return bool(confirmed and confirmed[0] == revision), branch

    def mirror_message(
        self, mirrored: bool, branch: str, owner_email: str,
        revision: str, log_path: Path,
    ) -> str:
        """Build the submitted-change publication result message."""

        commit_url = f"{self.github_web_url}/commit/{revision}"
        if not mirrored:
            return "\n".join([
                "xWalk GitHub Uplift", "Status: FAILED", f"Target: {commit_url}",
                f"Log: {log_path.name}",
            ])
        return "\n".join([
            "xWalk GitHub Uplift", "Status: PASSED",
            f"Owner: {owner_email or 'unavailable'}", f"Branch: {branch}",
            f"Commit: {commit_url}", f"Log: {log_path.name}",
        ])

    def integration_uplift_message(
        self, uploaded: bool, source_project: str, revision: str,
    ) -> str:
        """Build one component-to-integration change-log result row."""

        status = "PASSED" if uploaded else "FAILED"
        result = (
            "Integration review uploaded"
            if uploaded else "Integration uplift command failed"
        )
        return "\n".join([
            "xWalk Integration Uplift", f"Status: {status}",
            f"Source: {source_project}@{revision}",
            f"Target: {getattr(self, 'integration_project', 'xWalkPiCarAI')}/"
            f"{getattr(self, 'integration_branch', 'master')}",
            f"Result: {result}",
        ])

    @staticmethod
    def change_owner_email(change_data: dict[str, Any]) -> str:
        """Return the Gerrit owner email when the event exposes one."""

        owner = change_data.get("owner", {})
        return str(owner.get("email", "")) if isinstance(owner, dict) else ""

    def submitted_revision_verified(self, change: int, revision: str) -> bool:
        """Require the configured CI account's Verified +1 on the submitted revision."""

        command = f"gerrit query --current-patch-set --format=JSON change:{change}"
        result = self.run_ssh(command, capture_output=True)
        if result.returncode != 0:
            return False
        for line in result.stdout.splitlines():
            try:
                entry = json.loads(line)
            except json.JSONDecodeError:
                continue
            patch_set = entry.get("currentPatchSet", {})
            if patch_set.get("revision") != revision:
                continue
            approvals = patch_set.get("approvals", [])
            return any(
                approval.get("type") == "Verified"
                and str(approval.get("value")) in {"1", "+1"}
                and approval.get("by", {}).get("username") == self.user
                for approval in approvals
            )
        return False

    def change_details(self, change: int) -> dict[str, Any] | None:
        """Return current patch-set approvals and submit records for one change."""

        command = (
            "gerrit query --current-patch-set --submit-records --format=JSON "
            f"change:{change}"
        )
        result = self.run_ssh(command, capture_output=True)
        if result.returncode != 0:
            return None
        for line in (result.stdout or "").splitlines():
            try:
                entry = json.loads(line)
            except json.JSONDecodeError:
                continue
            if entry.get("number") == change or str(entry.get("number")) == str(change):
                return entry
        return None

    def submission_readiness(
        self, details: dict[str, Any] | None, patch_set: int, revision: str,
    ) -> tuple[bool, str]:
        """Require current approvals and Gerrit's complete submit policy."""

        if details is None:
            return False, "Gerrit change metadata is unavailable"
        if details.get("status") not in {"NEW", "OPEN"}:
            return False, "Integrated change is not open"
        if details.get("project") != getattr(self, "integration_project", "xWalkPiCarAI"):
            return False, "Change is not in the configured integrated repository"
        if details.get("branch") != getattr(self, "integration_branch", "master"):
            return False, "Change is not on the configured integrated branch"
        current = details.get("currentPatchSet", {})
        if (
            str(current.get("number")) != str(patch_set)
            or current.get("revision") != revision
        ):
            return False, "Approval or CI result belongs to a superseded patch set"
        approvals = current.get("approvals", [])
        verified = any(
            approval.get("type") == "Verified"
            and str(approval.get("value")) in {"1", "+1"}
            and approval.get("by", {}).get("username") == self.user
            for approval in approvals
        )
        reviewed = any(
            approval.get("type") == "Code-Review"
            and str(approval.get("value")) in {"2", "+2"}
            for approval in approvals
        )
        if not verified:
            return False, "Verified +1 from the configured CI account is missing"
        if not reviewed:
            return False, "Code-Review +2 from an authorized reviewer is missing"
        submit_records = details.get("submitRecords", [])
        if not submit_records or any(record.get("status") != "OK" for record in submit_records):
            return False, "Gerrit submit requirements are not all satisfied"
        if details.get("mergeable") is False:
            return False, "Integrated change has a merge conflict"
        return True, "Current patch set satisfies all Gerrit submit requirements"

    def apply_automatic_review(self, change: int, patch_set: int) -> bool:
        """Optionally apply Code-Review +2 through a dedicated configured account."""

        if not getattr(self, "auto_review", False):
            return False
        target = f"{change},{patch_set}"
        command = " ".join([
            "gerrit review", "--label Code-Review=+2",
            "--tag autogenerated:xwalk-uplift-review",
            "--message 'All mandatory integrated CI jobs passed; automatic review policy enabled.'",
            "--notify OWNER_REVIEWERS", shlex.quote(target),
        ])
        return self.run_review_ssh(command).returncode == 0

    def submit_if_ready(self, event: dict[str, Any]) -> bool:
        """Submit only the current integrated patch set after Gerrit policy is satisfied."""

        if not getattr(self, "auto_submit", False):
            return False
        change_data = event["change"]
        patch_data = event["patchSet"]
        change = int(change_data["number"])
        patch_set = int(patch_data["number"])
        revision = str(patch_data["revision"])
        details = self.change_details(change)
        ready, reason = self.submission_readiness(details, patch_set, revision)
        if not ready and reason.startswith("Code-Review +2") and self.apply_automatic_review(
            change, patch_set
        ):
            details = self.change_details(change)
            ready, reason = self.submission_readiness(details, patch_set, revision)
        change_url = f"{self.web_url}/c/{self.integration_project}/+/{change}"
        if not ready:
            self.append_changelog(
                self.integration_project, "merge", f"{change},{patch_set}", revision,
                str(change), revision, "skipped", reason, change_url,
            )
            print(f"Integrated submission blocked: {reason}", flush=True)
            return False
        command = f"gerrit review --submit --notify OWNER_REVIEWERS {change},{patch_set}"
        submitted = self.run_ssh(command).returncode == 0
        self.append_changelog(
            self.integration_project, "merge", f"{change},{patch_set}", revision,
            str(change), revision, "success" if submitted else "failed",
            "Gerrit accepted guarded submission" if submitted else "Gerrit rejected guarded submission",
            change_url,
        )
        print(
            f"Integrated merge status: {'submitted' if submitted else 'blocked by Gerrit'}",
            flush=True,
        )
        return submitted

    def remote_branch_revision(
        self, remote: str, branch: str, environment: dict[str, str],
    ) -> str:
        """Return one remote branch revision, or an empty string when it is unavailable."""

        result = subprocess.run(
            ["git", "ls-remote", remote, f"refs/heads/{branch}"],
            check=False, text=True, env=environment,
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        )
        if result.returncode != 0:
            return ""
        fields = result.stdout.split()
        if len(fields) != 2 or fields[1] != f"refs/heads/{branch}":
            return ""
        revision = fields[0]
        valid_revision = len(revision) == 40 and all(
            character in "0123456789abcdefABCDEF" for character in revision
        )
        return revision if valid_revision else ""

    def submitted_change_event(self, revision: str) -> dict[str, Any] | None:
        """Recover the merged-change event fields for one submitted branch revision."""

        command = (
            "gerrit query --current-patch-set --format=JSON "
            f"commit:{revision}"
        )
        result = self.run_ssh(command, capture_output=True)
        if result.returncode != 0:
            return None
        for line in result.stdout.splitlines():
            try:
                change = json.loads(line)
            except json.JSONDecodeError:
                continue
            patch_set = change.get("currentPatchSet", {})
            matches_revision = patch_set.get("revision") == revision
            matches_source = (
                change.get("project") == self.github_source_project
                and change.get("branch") == self.github_source_branch
            )
            if (
                change.get("status") == "MERGED"
                and matches_revision
                and matches_source
                and isinstance(patch_set.get("number"), int)
            ):
                return {
                    "type": "change-merged",
                    "change": change,
                    "patchSet": {"number": patch_set["number"]},
                    "newRev": revision,
                }
        return None

    def reconcile_github_uplift(self) -> None:
        """Recover a guarded GitHub uplift when its live Gerrit merge event was missed."""

        if not self.github_push_enabled or not self.validate_github_destination():
            return
        gerrit_remote = (
            f"ssh://{self.user}@{self.host}:{self.port}/"
            f"{self.github_source_project}"
        )
        gerrit_revision = self.remote_branch_revision(
            gerrit_remote, self.github_source_branch, self.git_environment()
        )
        github_revision = self.remote_branch_revision(
            self.github_remote, self.github_branch, self.github_git_environment()
        )
        if (
            not gerrit_revision
            or not github_revision
            or gerrit_revision == github_revision
        ):
            return
        event = self.submitted_change_event(gerrit_revision)
        if event is None or not self.matching_merge_event(event):
            print(
                "GitHub uplift recovery skipped: submitted Gerrit tip did not "
                "match the guarded merge policy",
                flush=True,
            )
            return
        print(
            f"Recovering missed GitHub uplift for Gerrit tip {gerrit_revision}",
            flush=True,
        )
        self.mirror(event)

    def mirror(self, event: dict[str, Any]) -> None:
        """Publish a verified submitted xWalk integration change to GitHub."""

        change_data = event["change"]
        patch_data = event["patchSet"]
        change = int(change_data["number"])
        patch_set = int(patch_data["number"])
        revision = str(event["newRev"])
        owner_email = self.change_owner_email(change_data)
        timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        log_path = self.log_directory / f"mirror-{change}-{patch_set}-{timestamp}.log"
        print(f"Mirroring merged change {change}: {revision}", flush=True)
        self.append_changelog(
            self.github_source_project, "merge", f"{change},{patch_set}", revision,
            str(change), revision, "success",
            "Gerrit reported the exact integrated change as merged",
            f"{self.github_web_url}/commit/{revision}" if self.github_web_url else "",
        )

        if not self.submitted_revision_verified(change, revision):
            message = "\n".join([
                "xWalk GitHub Uplift", "Status: SKIPPED",
                "Reason: submitted revision lacks CI Verified +1",
            ])
            self.post_message(
                change, patch_set, message,
                "autogenerated:xwalk-ci:github-uplift",
                notify="OWNER_REVIEWERS",
            )
            print(message, flush=True)
            return

        temporary_directory = Path(
            tempfile.mkdtemp(prefix=f"xwalk-github-mirror-{change}-")
        )
        try:
            mirrored, target_branch = self.execute_mirror(
                revision, owner_email, temporary_directory, log_path
            )
        finally:
            shutil.rmtree(temporary_directory, ignore_errors=True)

        message = self.mirror_message(
            mirrored, target_branch, owner_email, revision, log_path
        )
        reported = self.post_message(
            change, patch_set, message,
            "autogenerated:xwalk-ci:github-uplift",
            notify="OWNER_REVIEWERS",
        )
        self.append_changelog(
            self.github_source_project, "GitHub sync", f"{change},{patch_set}", revision,
            str(change), revision, "success" if mirrored else "failed",
            message.splitlines()[1],
            f"{self.github_web_url}/commit/{revision}" if self.github_web_url else "",
        )
        print(f"{message}; Gerrit report accepted={reported}", flush=True)

    def matching_verification_event(self, event: dict[str, Any]) -> bool:
        """Report whether an event requires configured host verification."""

        change = event.get("change", {})
        matches_target = (
            (change.get("project"), change.get("branch")) in self.verification_targets
            and isinstance(event.get("patchSet"), dict)
        )
        if not matches_target:
            return False

        event_type = event.get("type")
        if event_type == "patchset-created":
            return change.get("wip") is not True
        return event_type == "wip-state-changed" and change.get("wip") is not True

    def matching_merge_event(self, event: dict[str, Any]) -> bool:
        """Report whether a submitted change must be mirrored to GitHub."""

        change = event.get("change", {})
        return (
            event.get("type") == "change-merged"
            and self.github_push_enabled
            and self.validate_github_destination()
            and change.get("project") == self.github_source_project
            and change.get("branch") == self.github_source_branch
            and isinstance(event.get("patchSet"), dict)
            and isinstance(event.get("newRev"), str)
        )

    def matching_uplift_event(self, event: dict[str, Any]) -> bool:
        """Report whether one submitted component requires an integration uplift."""

        change = event.get("change", {})
        return (
            event.get("type") == "change-merged"
            and self.uplift_enabled
            and change.get("project") in self.component_repositories
            and change.get("branch") == "main"
            and isinstance(event.get("patchSet"), dict)
            and isinstance(event.get("newRev"), str)
        )

    def uplift(self, event: dict[str, Any]) -> None:
        """Run the locked integration-uplift workflow for one submitted module."""

        change = event["change"]
        patch_set = int(event["patchSet"]["number"])
        source_project = str(change["project"])
        revision = str(event["newRev"])
        topic = str(change.get("topic", ""))
        command = [
            str(self.uplift_script), "--apply", source_project, revision,
            str(change["number"]), str(patch_set), topic,
        ]
        result = subprocess.run(command, check=False, text=True)
        message = self.integration_uplift_message(
            result.returncode == 0, source_project, revision
        )
        reported = self.post_message(
            int(change["number"]), patch_set, message,
            "autogenerated:xwalk-ci:integration-uplift",
            notify="OWNER_REVIEWERS",
        )
        print(f"{message}; Gerrit report accepted={reported}", flush=True)
        if result.returncode != 0:
            print(
                f"Automatic uplift failed for {source_project} change {change['number']}",
                flush=True,
            )

    def matching_submit_event(self, event: dict[str, Any]) -> bool:
        """Select an approval update for guarded integrated submission."""

        change = event.get("change", {})
        return (
            event.get("type") == "comment-added"
            and getattr(self, "auto_submit", False)
            and change.get("project") == getattr(self, "integration_project", "xWalkPiCarAI")
            and change.get("branch") == getattr(self, "integration_branch", "master")
            and isinstance(event.get("patchSet"), dict)
            and isinstance(event["patchSet"].get("revision"), str)
        )

    def consume_stream(self) -> None:
        """Consume one Gerrit event-stream connection until it closes."""

        command = [*self.ssh_arguments(), "gerrit stream-events"]
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        assert process.stdout is not None
        for line in process.stdout:
            stripped = line.strip()
            if not stripped.startswith("{"):
                print(stripped, flush=True)
                continue
            event = json.loads(stripped)
            if self.matching_verification_event(event):
                self.verify(event)
            elif self.matching_uplift_event(event):
                self.uplift(event)
            elif self.matching_submit_event(event):
                self.submit_if_ready(event)
            elif self.matching_merge_event(event):
                self.mirror(event)
        process.wait()

    def run(self) -> None:
        """Reconnect indefinitely so temporary Gerrit outages do not stop CI."""

        signal.signal(signal.SIGTERM, self.terminate_on_signal)

        self.validate_private_key(self.private_key)
        supported_targets = {(project, "main") for project in self.repositories}
        supported_targets.add(("xWalkPiCarAI", "master"))
        if (self.project, self.branch) not in supported_targets:
            raise SystemExit("GERRIT_PROJECT and GERRIT_BRANCH are not a supported target")
        if not self.verification_targets <= supported_targets:
            raise SystemExit(
                "GERRIT_VERIFICATION_TARGETS contains an unsupported project or branch"
            )
        if self.uplift_enabled and not self.uplift_script.is_file():
            raise SystemExit(f"Missing automatic uplift script: {self.uplift_script}")
        if self.auto_review and not self.auto_submit:
            raise SystemExit("GERRIT_UPLIFT_AUTO_REVIEW requires GERRIT_UPLIFT_AUTO_SUBMIT")
        if self.auto_review:
            if not self.review_user or self.review_user == self.user:
                raise SystemExit("Automatic review requires a separate dedicated reviewer account")
            self.validate_private_key(self.review_private_key)
        if self.github_push_enabled:
            if not self.validate_github_destination():
                raise SystemExit(
                    "GitHub synchronization requires an exact integrated-project destination"
                )
            self.validate_private_key(self.github_private_key)
        self.log_server.start()
        print(
            f"xWalk CI log dashboard listening at {self.log_server.public_url}",
            flush=True,
        )
        while True:
            self.reconcile_github_uplift()
            self.consume_stream()
            print("Gerrit event stream closed; reconnecting in five seconds", flush=True)
            time.sleep(5)


if __name__ == "__main__":
    XWalkGerritCi().run()
