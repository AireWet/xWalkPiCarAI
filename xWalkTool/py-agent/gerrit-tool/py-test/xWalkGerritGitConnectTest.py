"""Host-only tests for the profile-aware Gerrit Git connector."""

from __future__ import annotations

import os
import pathlib
import subprocess
import tempfile
import unittest


CONNECTOR = pathlib.Path(__file__).parents[1] / "bin" / "xwalk-gerrit-git-connect"


class GerritGitConnectTest(unittest.TestCase):
    """Verify push-triggered startup without opening a real SSH connection."""

    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.home = pathlib.Path(self.temporary.name)
        self.binary_directory = self.home / "bin"
        self.binary_directory.mkdir()
        self.log = self.home / "operations.log"
        self.ssh = self.binary_directory / "fake-ssh"
        self.ssh.write_text(
            "#!/bin/sh\n"
            "printf 'ssh:%s\\n' \"$*\" >> \"$XWALK_TEST_LOG\"\n",
            encoding="utf-8",
        )
        self.ssh.chmod(0o700)
        self.environment = os.environ.copy()
        self.environment.update(
            {
                "HOME": str(self.home),
                "XWALK_GIT_SSH_BIN": str(self.ssh),
                "XWALK_TEST_LOG": str(self.log),
            }
        )

    def install_start_command(self) -> None:
        """Install a fake machine-local Gerrit lifecycle command."""

        start = self.binary_directory / "gerrit-start"
        start.write_text(
            "#!/bin/sh\n"
            "printf 'start\\n' >> \"$XWALK_TEST_LOG\"\n",
            encoding="utf-8",
        )
        start.chmod(0o700)

    def connect(self, host: str, auto_start: str = "true") -> subprocess.CompletedProcess[str]:
        """Run one simulated Gerrit receive-pack connection."""

        return subprocess.run(
            [
                str(CONNECTOR),
                host,
                "29419",
                "xWalkPiCarAI",
                auto_start,
                "git-receive-pack",
            ],
            check=False,
            env=self.environment,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

    def test_personal_and_college_profiles_start_the_current_machine(self) -> None:
        """Use the installed lifecycle command for either configured endpoint."""

        self.install_start_command()
        for host in ("joxy@192.168.1.158", "joxy@gerrit.college.example"):
            with self.subTest(host=host):
                self.log.write_text("", encoding="utf-8")
                result = self.connect(host)
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(
                    self.log.read_text(encoding="utf-8").splitlines(),
                    ["start", f"ssh:-p 29419 {host} git-receive-pack 'xWalkPiCarAI'"],
                )

    def test_remote_client_connects_without_local_start_command(self) -> None:
        """Skip lifecycle management when the current machine has no installation."""

        result = self.connect("joxy@gerrit.college.example")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            self.log.read_text(encoding="utf-8").splitlines(),
            ["ssh:-p 29419 joxy@gerrit.college.example git-receive-pack 'xWalkPiCarAI'"],
        )

    def test_disabled_startup_connects_without_starting(self) -> None:
        """Honor the machine-local automatic-start override."""

        self.install_start_command()
        result = self.connect("joxy@192.168.1.158", "false")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(
            self.log.read_text(encoding="utf-8").splitlines(),
            ["ssh:-p 29419 joxy@192.168.1.158 git-receive-pack 'xWalkPiCarAI'"],
        )

    def test_rejects_non_push_git_services(self) -> None:
        """Restrict the connector to the receive-pack service used by pushes."""

        result = subprocess.run(
            [
                str(CONNECTOR),
                "joxy@192.168.1.158",
                "29419",
                "xWalkPiCarAI",
                "true",
                "git-upload-pack",
            ],
            check=False,
            env=self.environment,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("Unsupported Gerrit Git service", result.stderr)


if __name__ == "__main__":
    unittest.main()
