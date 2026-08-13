#!/usr/bin/env python3
"""Verify Gerrit patch sets and mirror submitted changes to GitHub."""

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
    """Own Gerrit verification and submitted-change GitHub mirroring."""

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
        self.private_key = Path(
            os.environ.get("GERRIT_SSH_KEY", str(home / ".ssh" / "id_ed25519_xwalk_ci"))
        ).expanduser()

    def configure_github(self, home: Path) -> None:
        """Load GitHub mirror endpoint and identity settings."""

        self.github_host = os.environ.get("GITHUB_HOST", "github.com")
        self.github_repository = os.environ.get(
            "GITHUB_REPOSITORY", "jochuuu/xWalkPiCarAI"
        )
        self.github_web_url = os.environ.get(
            "GITHUB_WEB_URL", "https://github.com/jochuuu/xWalkPiCarAI"
        ).rstrip("/")
        self.github_primary_merger_email = os.environ.get(
            "GITHUB_PRIMARY_MERGER_EMAIL", "joxjoh24@student.hh.se"
        ).casefold()
        self.github_review_branch = os.environ.get(
            "GITHUB_REVIEW_BRANCH", "gerrit-submitted"
        )
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

    def post_message(self, change: int, patch_set: int, message: str) -> bool:
        """Post one informational CI message without changing a label vote."""

        target = f"{change},{patch_set}"
        command = " ".join(
            [
                "gerrit review",
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

    @staticmethod
    def select_github_branch(
        owner_email: str,
        primary_merger_email: str,
        primary_branch: str,
        review_branch: str,
        gerrit_parent: str,
        github_revision: str,
    ) -> str:
        """Select master only for one directly applicable Joxy-owned change."""

        owner_is_primary_merger = (
            bool(owner_email)
            and owner_email.casefold() == primary_merger_email.casefold()
        )
        directly_applicable = (
            bool(gerrit_parent)
            and gerrit_parent == github_revision
        )
        return primary_branch if owner_is_primary_merger and directly_applicable else review_branch

    def announce_verification(self, change: int, patch_set: int, revision: str) -> str:
        """Post dashboard links before running one patch-set quality gate."""

        results_url = self.log_server.dashboard_url(change, patch_set)
        links = [f"{name}: {url}" for name, url in self.log_server.job_links(change, patch_set)]
        message = "\n".join([
            "xWalk host verification started", f"Overall results and full log: {results_url}",
            "Separate job logs:", *links,
        ])
        print(f"Verifying change {change}, patch set {patch_set}: {revision}", flush=True)
        self.report(change, patch_set, 0, message)
        return results_url

    def checkout_commands(self, ref: str) -> list[tuple[list[str], dict[str, str] | None]]:
        """Build the isolated Gerrit patch-set checkout commands."""

        remote = f"ssh://{self.user}@{self.host}:{self.port}/{self.project}"
        return [
            (["git", "init", "--quiet"], None),
            (["git", "remote", "add", "origin", remote], None),
            (["git", "fetch", "--depth=1", "origin", ref], self.git_environment()),
            (["git", "checkout", "--detach", "FETCH_HEAD"], None),
        ]

    def execute_verification(
        self, ref: str, directory: Path, log_path: Path,
    ) -> tuple[bool, dict[str, bool]]:
        """Checkout one patch set and execute the unchanged quality matrix."""

        with log_path.open("w", encoding="utf-8") as log:
            checkout_passed = all(
                self.run_command(command, directory, log, environment)
                for command, environment in self.checkout_commands(ref)
            )
            results = XWalkGerritQuality(directory, log).run_all() if checkout_passed else {}
        return checkout_passed and all(results.values()), results

    def verification_message(
        self, change: int, patch_set: int, passed: bool,
        results: dict[str, bool], results_url: str, log_path: Path,
    ) -> tuple[int, str]:
        """Build the aggregate Gerrit vote and linked quality summary."""

        jobs = [
            f"{name}: {'PASSED' if value else 'FAILED'} - "
            f"{self.log_server.job_url(change, patch_set, name)}"
            for name, value in results.items()
        ]
        if passed:
            lines = [
                "Complete xWalk host quality gate passed",
                f"Jobs: {len(results)}/{len(results)}",
            ]
            details = [*jobs, f"Overall results and full log: {results_url}", f"Log: {log_path.name}"]
            return 1, "\n".join([*lines, *details])
        failed = ", ".join(name for name, value in results.items() if not value) or "checkout"
        lines = ["Complete xWalk host quality gate failed", f"Failed jobs: {failed}"]
        details = [*jobs, f"Overall results and full log: {results_url}", f"Log: {log_path.name}"]
        return -1, "\n".join([*lines, *details])

    def verify(self, event: dict[str, Any]) -> None:
        """Fetch, build, test, and report one matching Gerrit patch set."""

        change = int(event["change"]["number"])
        patch_set = int(event["patchSet"]["number"])
        revision = str(event["patchSet"]["revision"])
        ref = str(event["patchSet"]["ref"])
        timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        log_path = self.log_directory / f"change-{change}-{patch_set}-{timestamp}.log"
        log_path.touch(exist_ok=False)
        results_url = self.announce_verification(change, patch_set, revision)

        temporary_directory = Path(
            tempfile.mkdtemp(prefix=f"xwalk-gerrit-{change}-{patch_set}-")
        )
        try:
            passed, quality_results = self.execute_verification(
                ref, temporary_directory, log_path
            )
        finally:
            shutil.rmtree(temporary_directory, ignore_errors=True)

        vote, message = self.verification_message(
            change, patch_set, passed, quality_results, results_url, log_path
        )
        reported = self.report(change, patch_set, vote, message)
        print(f"{message}; Gerrit report accepted={reported}", flush=True)

    def mirror_commands(
        self, revision: str,
    ) -> list[tuple[list[str], dict[str, str] | None]]:
        """Build Gerrit and GitHub setup commands for one submitted revision."""

        gerrit = f"ssh://{self.user}@{self.host}:{self.port}/{self.project}"
        github = f"git@{self.github_host}:{self.github_repository}.git"
        github_ref = f"refs/heads/{self.branch}:refs/remotes/github/{self.branch}"
        return [
            (["git", "init", "--quiet"], None),
            (["git", "remote", "add", "gerrit", gerrit], None),
            (
                ["git", "fetch", "gerrit", f"{revision}:refs/remotes/gerrit/submitted"],
                self.git_environment(),
            ),
            (["git", "remote", "add", "github", github], None),
            (
                ["git", "fetch", "github", github_ref],
                self.github_git_environment(),
            ),
        ]

    def select_mirror_branch(self, directory: Path, log: TextIO, owner_email: str) -> str:
        """Select the owner-aware destination from current repository revisions."""

        parent = self.command_output(
            ["git", "rev-parse", "refs/remotes/gerrit/submitted^"], directory, log
        )
        github_revision = self.command_output(
            ["git", "rev-parse", f"refs/remotes/github/{self.branch}"], directory, log
        )
        return self.select_github_branch(
            owner_email, self.github_primary_merger_email, self.branch,
            self.github_review_branch, parent, github_revision,
        )

    def execute_mirror(
        self, revision: str, owner_email: str, directory: Path, log_path: Path,
    ) -> tuple[bool, str]:
        """Fetch and publish one submitted Gerrit revision without force push."""

        with log_path.open("w", encoding="utf-8") as log:
            setup_passed = all(
                self.run_command(command, directory, log, environment)
                for command, environment in self.mirror_commands(revision)
            )
            branch = self.github_review_branch
            if not setup_passed:
                return False, branch
            branch = self.select_mirror_branch(directory, log, owner_email)
            command = [
                "git", "push", "github",
                f"refs/remotes/gerrit/submitted:refs/heads/{branch}",
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
        result = (
            "Joxy-owned Gerrit change mirrored to GitHub master"
            if branch == self.branch else "Gerrit change published for Joxy GitHub review"
        )
        return "\n".join([
            result, f"Owner: {owner_email or 'unavailable'}", f"Branch: {branch}",
            f"Commit: {commit_url}", f"Log: {log_path.name}",
        ])

    @staticmethod
    def change_owner_email(change_data: dict[str, Any]) -> str:
        """Return the Gerrit owner email when the event exposes one."""

        owner = change_data.get("owner", {})
        return str(owner.get("email", "")) if isinstance(owner, dict) else ""

    def mirror(self, event: dict[str, Any]) -> None:
        """Publish a submitted change to GitHub master or its review branch."""

        change_data = event["change"]
        patch_data = event["patchSet"]
        change = int(change_data["number"])
        patch_set = int(patch_data["number"])
        revision = str(event["newRev"])
        owner_email = self.change_owner_email(change_data)
        timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        log_path = self.log_directory / f"mirror-{change}-{patch_set}-{timestamp}.log"
        print(f"Mirroring merged change {change}: {revision}", flush=True)

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
            change.get("project") == self.project
            and change.get("branch") == self.branch
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
            and change.get("project") == self.project
            and change.get("branch") == self.branch
            and isinstance(event.get("patchSet"), dict)
            and isinstance(event.get("newRev"), str)
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
            elif self.matching_merge_event(event):
                self.mirror(event)
        process.wait()

    def run(self) -> None:
        """Reconnect indefinitely so temporary Gerrit outages do not stop CI."""

        self.validate_private_key(self.private_key)
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
