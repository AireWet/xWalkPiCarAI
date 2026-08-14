#!/usr/bin/env python3
"""Verify Gerrit patch sets and synchronize submitted xWalk-rpi5 integration."""

from __future__ import annotations

from datetime import datetime, timezone
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile
import time
from typing import Any, TextIO

from xWalkGerritLogServer import XWalkGerritLogServer
from xWalkGerritQuality import XWalkGerritQuality


class XWalkGerritCi:
    """Own Gerrit verification and guarded xWalk-rpi5 GitHub synchronization."""

    integration_repositories = {"xWalk-rpi5", "xWalkPiCarAI"}
    repositories = {
        "DevloperNote", "xWalkAgent", "xWalkAudioResources", "xWalkController",
        "xWalkHal", "xWalkIW", "xWalkLibrary", "xWalkTrace", "xWalk-rpi5",
    }

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
        self.project = os.environ.get("GERRIT_PROJECT", "xWalk-rpi5")
        self.branch = os.environ.get("GERRIT_BRANCH", "main")
        self.verification_targets = self.parse_verification_targets(
            os.environ.get("GERRIT_VERIFICATION_TARGETS", ""),
            (self.project, self.branch),
        )
        self.private_key = Path(
            os.environ.get("GERRIT_SSH_KEY", str(home / ".ssh" / "id_ed25519_xwalk_ci"))
        ).expanduser()
        self.uplift_enabled = os.environ.get("GERRIT_UPLIFT_ENABLED", "false") == "true"
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

        self.github_remote = os.environ.get("GITHUB_XWALK_RPI5_REMOTE", "")
        self.github_web_url = os.environ.get("GITHUB_XWALK_RPI5_WEB_URL", "").rstrip("/")
        self.github_branch = os.environ.get("GITHUB_XWALK_RPI5_BRANCH", "main")
        self.github_push_enabled = os.environ.get("GITHUB_PUSH_ENABLED", "false") == "true"
        self.github_direct_push_owner_email = os.environ.get(
            "GITHUB_DIRECT_PUSH_OWNER_EMAIL", ""
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
            "StrictHostKeyChecking=accept-new",
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
            "StrictHostKeyChecking=accept-new",
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
        result = subprocess.run(
            [*self.ssh_arguments(), command], check=False, text=True
        )
        return result.returncode == 0

    def post_message(
        self, change: int, patch_set: int, message: str,
        tag: str = "autogenerated:xwalk-ci",
    ) -> bool:
        """Post one informational CI message without changing a label vote."""

        target = f"{change},{patch_set}"
        command = " ".join(
            [
                "gerrit review",
                f"--tag {shlex.quote(tag)}",
                f"--message {shlex.quote(message)}",
                "--notify NONE",
                shlex.quote(target),
            ]
        )
        result = subprocess.run(
            [*self.ssh_arguments(), command], check=False, text=True
        )
        return result.returncode == 0

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

    def validate_github_destination(self) -> bool:
        """Accept only an explicitly enabled remote whose repository is xWalk-rpi5."""

        if (
            not self.github_push_enabled or not self.github_remote
            or not self.github_direct_push_owner_email or self.branch != "main"
            or self.github_branch != "main"
        ):
            return False
        repository = self.github_remote.rstrip("/").rsplit("/", 1)[-1].rsplit(":", 1)[-1]
        return repository.removesuffix(".git") == "xWalk-rpi5"

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
        project = (
            target_project
            if target_project in self.integration_repositories
            else "xWalk-rpi5"
        )
        remote = f"ssh://{self.user}@{self.host}:{self.port}/{project}"
        commands: list[tuple[list[str], dict[str, str] | None]] = [
            (["git", "init", "--quiet"], None),
            (["git", "remote", "add", "origin", remote], None),
            (
                ["git", "fetch", "origin", ref if project == target_project else "main"],
                self.git_environment(),
            ),
            (["git", "checkout", "--detach", "FETCH_HEAD"], None),
        ]
        if project != target_project:
            commands.extend([
                (["git", "submodule", "sync", "--recursive"], None),
                (
                    ["git", "submodule", "update", "--init", "--recursive"],
                    self.git_environment(),
                ),
                (
                    ["git", "-C", target_project, "fetch", "origin", ref],
                    self.git_environment(),
                ),
                (["git", "-C", target_project, "checkout", "--detach", "FETCH_HEAD"], None),
            ])
        return commands

    def standalone_commands(
        self, directory: Path, verification_project: str | None = None,
    ) -> list[list[str]]:
        """Return the module-specific validation commands before integration CI."""

        target_project = verification_project or self.project
        if target_project in self.integration_repositories:
            return []
        module = directory / target_project
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
                XWalkGerritQuality(directory, log).run_all() if standalone_passed else {}
            )
            if target_project not in self.integration_repositories:
                results = {f"{target_project} standalone": standalone_passed, **results}
        return checkout_passed and all(results.values()), results

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
            message = "\n".join([
                str(job["name"]),
                f"Status: {status}",
                f"Duration: {self.log_server.format_duration(job.get('duration_seconds'))}",
                f"Tests passed: {completed}/{len(checks)}",
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
                ref, temporary_directory, log_path, project
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
        print(
            f"{message}; module reports accepted={modules_reported}; "
            f"Gerrit gate report accepted={reported}", flush=True,
        )

    def mirror_commands(
        self, revision: str,
    ) -> list[tuple[list[str], dict[str, str] | None]]:
        """Build Gerrit and GitHub setup commands for one submitted revision."""

        gerrit = f"ssh://{self.user}@{self.host}:{self.port}/{self.project}"
        return [
            (["git", "init", "--quiet"], None),
            (["git", "remote", "add", "gerrit", gerrit], None),
            (
                ["git", "fetch", "gerrit", "main:refs/remotes/gerrit/main"],
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
                self.run_command(command, directory, log, environment)
                for command, environment in self.mirror_commands(revision)
            )
            branch = self.branch
            if not setup_passed:
                return False, branch
            submitted = self.command_output(
                ["git", "rev-parse", "refs/remotes/gerrit/main"], directory, log
            )
            if submitted != revision:
                log.write("Submitted revision is not the current Gerrit xWalk-rpi5/main tip\n")
                return False, branch
            command = [
                "git", "push", "github",
                "refs/remotes/gerrit/main:refs/heads/main",
            ]
            return self.run_command(
                command, directory, log, self.github_git_environment()
            ), branch

    def mirror_message(
        self, mirrored: bool, branch: str, owner_email: str,
        revision: str, log_path: Path,
    ) -> str:
        """Build the submitted-change publication result message."""

        commit_url = f"{self.github_web_url}/commit/{revision}"
        if not mirrored:
            return "\n".join(["GitHub mirror failed", f"Target: {commit_url}", f"Log: {log_path.name}"])
        result = "Verified submitted xWalk-rpi5 integration synchronized to GitHub main"
        return "\n".join([
            result, f"Owner: {owner_email or 'unavailable'}", f"Branch: {branch}",
            f"Commit: {commit_url}", f"Log: {log_path.name}",
        ])

    @staticmethod
    def change_owner_email(change_data: dict[str, Any]) -> str:
        """Return the Gerrit owner email when the event exposes one."""

        owner = change_data.get("owner", {})
        return str(owner.get("email", "")) if isinstance(owner, dict) else ""

    def submitted_revision_verified(self, change: int, revision: str) -> bool:
        """Require the configured CI account's Verified +1 on the submitted revision."""

        command = f"gerrit query --current-patch-set --format=JSON change:{change}"
        result = subprocess.run(
            [*self.ssh_arguments(), command], check=False, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL,
        )
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

    def mirror(self, event: dict[str, Any]) -> None:
        """Publish a verified submitted xWalk-rpi5 change to GitHub main."""

        change_data = event["change"]
        patch_data = event["patchSet"]
        change = int(change_data["number"])
        patch_set = int(patch_data["number"])
        revision = str(event["newRev"])
        owner_email = self.change_owner_email(change_data)
        timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        log_path = self.log_directory / f"mirror-{change}-{patch_set}-{timestamp}.log"
        print(f"Mirroring merged change {change}: {revision}", flush=True)

        if not self.submitted_revision_verified(change, revision):
            message = "GitHub synchronization skipped: submitted revision lacks CI Verified +1"
            self.post_message(change, patch_set, message)
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
        reported = self.post_message(change, patch_set, message)
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
            and self.project == "xWalk-rpi5"
            and change.get("project") == self.project
            and change.get("branch") == self.branch
            and self.change_owner_email(change).casefold()
            == self.github_direct_push_owner_email
            and isinstance(event.get("patchSet"), dict)
            and isinstance(event.get("newRev"), str)
        )

    def matching_uplift_event(self, event: dict[str, Any]) -> bool:
        """Report whether one submitted component requires an integration uplift."""

        change = event.get("change", {})
        return (
            event.get("type") == "change-merged"
            and self.uplift_enabled
            and self.project in self.repositories - {"xWalk-rpi5"}
            and change.get("project") == self.project
            and change.get("branch") == "main"
            and isinstance(event.get("newRev"), str)
        )

    def uplift(self, event: dict[str, Any]) -> None:
        """Run the locked integration-uplift workflow for one submitted module."""

        change = event["change"]
        topic = str(change.get("topic", ""))
        command = [
            str(self.uplift_script), "--apply", self.project, str(event["newRev"]),
            str(change["number"]),
        ]
        if topic:
            command.append(topic)
        result = subprocess.run(command, check=False, text=True)
        if result.returncode != 0:
            print(
                f"Automatic uplift failed for {self.project} change {change['number']}",
                flush=True,
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
            elif self.matching_merge_event(event):
                self.mirror(event)
        process.wait()

    def run(self) -> None:
        """Reconnect indefinitely so temporary Gerrit outages do not stop CI."""

        self.validate_private_key(self.private_key)
        if self.project not in self.repositories or self.branch != "main":
            raise SystemExit("GERRIT_PROJECT must be an xWalk repository on branch main")
        supported_targets = {(project, "main") for project in self.repositories}
        supported_targets.add(("xWalkPiCarAI", "master"))
        if not self.verification_targets <= supported_targets:
            raise SystemExit(
                "GERRIT_VERIFICATION_TARGETS contains an unsupported project or branch"
            )
        if self.uplift_enabled and not self.uplift_script.is_file():
            raise SystemExit(f"Missing automatic uplift script: {self.uplift_script}")
        if self.github_push_enabled:
            if not self.validate_github_destination():
                raise SystemExit("GitHub synchronization requires the exact xWalk-rpi5/main destination")
            self.validate_private_key(self.github_private_key)
        self.log_server.start()
        print(
            f"xWalk CI log dashboard listening at {self.log_server.public_url}",
            flush=True,
        )
        while True:
            self.consume_stream()
            print("Gerrit event stream closed; reconnecting in five seconds", flush=True)
            time.sleep(5)


if __name__ == "__main__":
    XWalkGerritCi().run()
