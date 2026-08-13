"""Host-only tests for the non-root Gerrit server installer."""

from __future__ import annotations

import hashlib
import importlib.util
import pathlib
import subprocess
import tempfile
import unittest


MODULE_PATH = pathlib.Path(__file__).with_name("xWalkGerritServerSetup.py")
SPECIFICATION = importlib.util.spec_from_file_location("xwalk_gerrit_server_setup", MODULE_PATH)
assert SPECIFICATION is not None
assert SPECIFICATION.loader is not None
SETUP = importlib.util.module_from_spec(SPECIFICATION)
SPECIFICATION.loader.exec_module(SETUP)


class GerritServerSetupTest(unittest.TestCase):
    """Verify installer validation and reviewed shell templates."""

    def test_accepts_assigned_non_virtual_address(self) -> None:
        """A concrete address on a normal interface is accepted."""

        addresses = [{"interface": "ens3", "address": "10.20.30.40", "scope": "global"}]
        self.assertEqual(SETUP.validate_server_ip("10.20.30.40", addresses), "ens3")

    def test_rejects_unassigned_and_virtual_addresses(self) -> None:
        """Guessed, loopback, and container bridge addresses are rejected."""

        addresses = [{"interface": "docker0", "address": "172.17.0.1", "scope": "global"}]
        with self.assertRaises(SETUP.SetupError):
            SETUP.validate_server_ip("127.0.0.1", addresses)
        with self.assertRaises(SETUP.SetupError):
            SETUP.validate_server_ip("10.20.30.40", addresses)
        with self.assertRaises(SETUP.SetupError):
            SETUP.validate_server_ip("172.17.0.1", addresses)

    def test_verified_download_rejects_non_https_source(self) -> None:
        """Artifacts cannot be fetched over an unauthenticated transport."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            destination = pathlib.Path(temporary_directory) / "artifact"
            with self.assertRaises(SETUP.SetupError):
                SETUP.download_verified("http://example.invalid/file", "0" * 64, destination)

    def test_sha256_calculates_known_digest(self) -> None:
        """The streaming digest helper matches hashlib."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            target = pathlib.Path(temporary_directory) / "sample"
            target.write_bytes(b"verified Gerrit artifact\n")
            self.assertEqual(SETUP.sha256(target), hashlib.sha256(target.read_bytes()).hexdigest())

    def test_shell_templates_are_valid_and_do_not_use_sudo(self) -> None:
        """Every installed management command is valid POSIX shell and non-root."""

        template_directory = MODULE_PATH.parent / "templates" / "bin"
        for template in template_directory.iterdir():
            result = subprocess.run(
                ["sh", "-n", str(template)],
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertNotIn("sudo", template.read_text(encoding="utf-8"))

    def test_documentation_templates_render_without_placeholders(self) -> None:
        """Rendered guides contain concrete endpoints and valid SSH user syntax."""

        replacements = {
            "GERRIT_VERSION": "3.14.2",
            "JAVA_VERSION": "Java 21",
            "JAVA_HOME": "/usr/lib/jvm/java-21",
            "JAVA_SOURCE": "server-provided Java 21 runtime",
            "JAVA_SHA256": "not applicable to server-provided Java",
            "SERVER_IP": "10.20.30.40",
            "HTTPS_PORT": "18443",
            "SSH_PORT": "29418",
            "PROJECT_NAME": "MyPiCarX",
            "INTERFACE": "ens3",
            "GERRIT_SHA256": "a" * 64,
            "GERRIT_URL": "https://example.invalid/gerrit.war",
            "TLS_MODE": "import",
            "LDAP_SERVER": "ldaps://directory.example",
            "TLS_FINGERPRINT": "sha256 fingerprint",
            "INSTALL_DATE": "2026-08-13T00:00:00+00:00",
        }
        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            SETUP.copy_templates(MODULE_PATH.parent, home, replacements)
            guides = "\n".join(
                path.read_text(encoding="utf-8")
                for path in (home / "gerrit-site" / "docs").iterdir()
            )
            self.assertNotIn("@@", guides)
            self.assertIn("USERNAME@10.20.30.40", guides)
            self.assertIn("https://10.20.30.40:18443/q/project:MyPiCarX", guides)
            self.assertIn("gerrit.canonicalWebUrl", guides)

    def test_installer_has_no_forbidden_execution_commands(self) -> None:
        """The executable installer does not contain privileged management commands."""

        source = MODULE_PATH.read_text(encoding="utf-8")
        for forbidden in ("sudo", "apt-get", "dnf", "yum", "zypper", "snap"):
            self.assertNotIn(forbidden, source)


if __name__ == "__main__":
    unittest.main()
