"""Host-only tests for the non-root Gerrit server installer."""

from __future__ import annotations

import hashlib
import importlib.util
import os
import pathlib
import shutil
import socket
import subprocess
import tempfile
import types
import unittest
from unittest import mock


GERRIT_ROOT = pathlib.Path(__file__).parents[1]
MODULE_PATH = GERRIT_ROOT / "py-src" / "xWalkGerritServerSetup.py"
SPECIFICATION = importlib.util.spec_from_file_location("xwalk_gerrit_server_setup", MODULE_PATH)
assert SPECIFICATION is not None
assert SPECIFICATION.loader is not None
SETUP = importlib.util.module_from_spec(SPECIFICATION)
SPECIFICATION.loader.exec_module(SETUP)


def caddy_arguments(server_ip: str, https_port: int, backend_port: int) -> types.SimpleNamespace:
    """Build the common Caddy configuration arguments."""

    return types.SimpleNamespace(
        admin_username="joxy",
        server_ip=server_ip,
        https_port=https_port,
        http_port=backend_port,
    )


def caddy_password_hash(binary: pathlib.Path) -> str:
    """Create the fixed test password hash with the supplied Caddy binary."""

    result = subprocess.run(
        [str(binary), "hash-password", "--algorithm", "bcrypt"],
        check=True,
        input="temporary-test-password\n",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return result.stdout.strip()


def create_caddy_fixture(
    home: pathlib.Path,
    arguments: types.SimpleNamespace,
) -> types.SimpleNamespace:
    """Create a complete local Caddy test fixture."""

    site = home / "gerrit-site"
    (site / "etc").mkdir(parents=True)
    (site / "logs").mkdir()
    caddy_home = home / "apps" / "caddy" / "current"
    caddy_home.mkdir(parents=True)
    binary = caddy_home / "caddy"
    shutil.copyfile(os.environ["XWALK_CADDY_TEST_BINARY"], binary)
    binary.chmod(0o700)
    certificate, private_key, unused_fingerprint = SETUP.configure_self_signed_certificate(
        arguments,
        site,
    )
    configuration = SETUP.write_caddy_configuration(
        arguments, home, site, certificate, private_key, caddy_password_hash(binary)
    )
    return types.SimpleNamespace(
        site=site, caddy_home=caddy_home, binary=binary,
        certificate=certificate, configuration=configuration,
    )


def available_ports() -> tuple[int, int]:
    """Reserve and release two distinct loopback test ports."""

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as https_probe:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as backend_probe:
            https_probe.bind(("127.0.0.1", 0))
            backend_probe.bind(("127.0.0.1", 0))
            return https_probe.getsockname()[1], backend_probe.getsockname()[1]


def run_control(control: pathlib.Path, environment: dict[str, str], action: str) -> None:
    """Run one Caddy control action and require success."""

    result = subprocess.run(
        [str(control), action], check=False, env=environment,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=15,
    )
    if result.returncode != 0:
        raise AssertionError(result.stderr)


def response_status(certificate: pathlib.Path, port: int, path: str) -> str:
    """Return the HTTPS status for one Caddy route."""

    result = subprocess.run(
        [
            "curl", "--silent", "--show-error", "--output", "/dev/null",
            "--write-out", "%{http_code}", "--cacert", str(certificate),
            f"https://127.0.0.1:{port}{path}",
        ],
        check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, timeout=15,
    )
    return result.stdout


def documentation_replacements() -> dict[str, str]:
    """Return complete concrete values for rendering documentation templates."""

    return {
        "GERRIT_VERSION": "3.14.2", "JAVA_VERSION": "Java 21",
        "JAVA_HOME": "/usr/lib/jvm/java-21", "JAVA_SOURCE": "server-provided Java 21 runtime",
        "JAVA_SHA256": "not applicable to server-provided Java", "SERVER_IP": "10.20.30.40",
        "HTTPS_PORT": "18443", "SSH_PORT": "29418", "HTTP_PORT": "8080",
        "PROJECT_NAME": "MyPiCarX",
        "PROJECT_BRANCH": "master", "INTERFACE": "ens3", "HOME": "/home/joxy",
        "GERRIT_SHA256": "a" * 64,
        "GERRIT_URL": "https://example.invalid/gerrit.war", "ADMIN_USERNAME": "joxy",
        "ADMIN_NAME": "Joxy John", "ADMIN_ROLE": "Student",
        "ADMIN_EMAIL": "joxjoh24@student.hh.se",
        "CADDY_VERSION": "v2.11.3", "CADDY_URL": "https://example.invalid/caddy.tar.gz",
        "CADDY_SHA256": "b" * 64, "CERTIFICATE_FINGERPRINT": "sha256 fingerprint",
        "INSTALL_DATE": "2026-08-13T00:00:00+00:00",
        "GERRIT_SITE": "/shared storage/joxy/gerrit-site",
        "CANONICAL_WEB_URL": "https://10.20.30.40:18443/",
        "SSH_LISTEN_ADDRESS": "10.20.30.40:29418",
        "HTTP_LISTEN_URL": "proxy-https://127.0.0.1:8080/",
    }


def expected_documentation_names() -> set[str]:
    """Return the complete rendered Gerrit documentation set."""

    return {
        "Gerrit Server Overview.md", "Gerrit Setup Installer.md",
        "Gerrit Local Linux Setup.md",
        "Gerrit Administrator Request.md", "Gerrit Storage and Migration.md",
        "Gerrit Admin Setup.md", "Gerrit User Configuration.md",
        "Gerrit CI Configuration.md", "Gerrit Backup and Restore.md",
        "CodeScene Code Health CI.md", "CodeScene CI Changelog.md",
        "Integrated Uplift Workflow.md",
        "Gerrit Multi Repository Architecture.md",
        "Gerrit Security and Remote Access.md", "Gerrit Troubleshooting.md",
        "XWALK_CI_ENV.example",
    }


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

    def test_storage_defaults_to_home_and_supports_spaces(self) -> None:
        """Home fallback and an absolute shared path containing spaces validate safely."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            shared_parent = home / "shared disk"
            shared_parent.mkdir()
            mount = {"target": str(home), "source": "/dev/test", "fstype": "ext4", "options": "rw"}
            with mock.patch.object(SETUP, "storage_mount", return_value=mount):
                fallback, unused_mount = SETUP.validate_storage_path(None, home, 1)
                shared, unused_mount = SETUP.validate_storage_path(
                    str(shared_parent / "gerrit site"), home, 1
                )
            self.assertEqual(fallback, home / "gerrit-site")
            self.assertEqual(shared, shared_parent / "gerrit site")
            self.assertEqual(list(home.glob(".gerrit-storage-check-*")), [])
            self.assertEqual(list(shared_parent.glob(".gerrit-storage-check-*")), [])

    def test_storage_rejects_unsafe_missing_and_symbolic_paths(self) -> None:
        """Broad, relative, missing-parent, and symbolic storage paths are rejected."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            with self.assertRaisesRegex(SETUP.SetupError, "broad"):
                SETUP.resolve_storage_path(str(home), home)
            with self.assertRaisesRegex(SETUP.SetupError, "absolute"):
                SETUP.resolve_storage_path("relative/gerrit-site", home)
            with self.assertRaisesRegex(SETUP.SetupError, "parent does not exist"):
                SETUP.validate_storage_path(str(home / "missing" / "site"), home, 1)
            actual = home / "actual"
            actual.mkdir()
            link = home / "linked"
            link.symlink_to(actual, target_is_directory=True)
            with self.assertRaisesRegex(SETUP.SetupError, "symbolic-link"):
                SETUP.resolve_storage_path(str(link / "gerrit-site"), home)

    def test_storage_rejects_read_only_full_and_unwritable_mounts(self) -> None:
        """Read-only, insufficient, and permission-denied storage never proceeds."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            target = str(home / "gerrit-site")
            read_only = {
                "target": str(home), "source": "/dev/test", "fstype": "ext4", "options": "ro",
            }
            with mock.patch.object(SETUP, "storage_mount", return_value=read_only):
                with self.assertRaisesRegex(SETUP.SetupError, "read-only"):
                    SETUP.validate_storage_path(target, home, 1)
            writable = {
                "target": str(home), "source": "/dev/test", "fstype": "ext4", "options": "rw",
            }
            disk = types.SimpleNamespace(total=100, used=99, free=1)
            with mock.patch.object(SETUP, "storage_mount", return_value=writable):
                with mock.patch.object(SETUP.shutil, "disk_usage", return_value=disk):
                    with self.assertRaisesRegex(SETUP.SetupError, "insufficient"):
                        SETUP.validate_storage_path(target, home, 2)
                with mock.patch.object(SETUP.tempfile, "mkdtemp", side_effect=PermissionError("denied")):
                    with self.assertRaisesRegex(SETUP.SetupError, "ownership and permissions"):
                        SETUP.validate_storage_path(target, home, 1)
            temporary = {"target": str(home), "source": "tmpfs", "fstype": "tmpfs", "options": "rw"}
            with mock.patch.object(SETUP, "storage_mount", return_value=temporary):
                with self.assertRaisesRegex(SETUP.SetupError, "not confirmed persistent"):
                    SETUP.validate_storage_path(target, home, 1)

    def test_endpoint_validation_requires_safe_explicit_bindings(self) -> None:
        """Configured endpoints reject wildcards, mismatched ports, and occupied ports."""

        addresses = [{"interface": "ens3", "address": "10.20.30.40", "scope": "global"}]
        arguments = types.SimpleNamespace(
            canonical_web_url="https://10.20.30.40:18443/",
            http_listen_url="proxy-https://127.0.0.1:8080/", http_port=8080,
            ssh_listen_address="10.20.30.40:29418", ssh_port=29418,
            https_port=18443, server_ip="10.20.30.40",
        )
        SETUP.validate_endpoint_configuration(arguments, addresses)
        arguments.ssh_listen_address = "0.0.0.0:29418"
        with self.assertRaisesRegex(SETUP.SetupError, "specific"):
            SETUP.validate_endpoint_configuration(arguments, addresses)
        arguments.ssh_listen_address = "10.20.30.40:29418"
        arguments.canonical_web_url = "https://different.example:18443/"
        with self.assertRaisesRegex(SETUP.SetupError, "host must match"):
            SETUP.validate_endpoint_configuration(arguments, addresses)
        arguments.canonical_web_url = "https://10.20.30.40:18443/"
        arguments.ssh_listen_address = "10.20.30.40:29418"
        with mock.patch.object(SETUP, "port_is_free", return_value=False):
            with self.assertRaisesRegex(SETUP.SetupError, "occupied"):
                SETUP.validate_ports(arguments)

    def test_repeated_install_preserves_existing_site(self) -> None:
        """A repeated installation stops before changing an existing Gerrit site."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            site = home / "gerrit-site"
            site.mkdir()
            marker = site / "preserve-me"
            marker.write_text("existing data\n", encoding="utf-8")
            arguments = types.SimpleNamespace(
                storage_path=None, http_listen_url=None, canonical_web_url=None,
                ssh_listen_address=None, http_port=8080, ssh_port=29418,
                https_port=18443, server_ip="10.20.30.40",
            )
            mount = {
                "target": str(home), "source": "/dev/test", "fstype": "ext4", "options": "rw",
            }
            with mock.patch.object(SETUP.pathlib.Path, "home", return_value=home):
                with mock.patch.object(SETUP, "validate_storage_path", return_value=(site, mount)):
                    with self.assertRaisesRegex(SETUP.SetupError, "No files were changed"):
                        SETUP.install(arguments)
            self.assertEqual(marker.read_text(encoding="utf-8"), "existing data\n")

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

    def test_local_authentication_requires_verified_caddy_and_safe_admin(self) -> None:
        """Local authentication requires verified artifacts and a safe administrator."""

        arguments = types.SimpleNamespace(
            admin_username="joxy",
            caddy_url="https://example.invalid/caddy.tar.gz",
            caddy_sha256="a" * 64,
        )
        with mock.patch.dict(os.environ, {"GERRIT_ADMIN_PASSWORD": "valid-password-value"}):
            SETUP.validate_authentication(arguments)
            arguments.admin_username = "unsafe user"
            with self.assertRaisesRegex(SETUP.SetupError, "safe Gerrit username"):
                SETUP.validate_authentication(arguments)

    def test_install_interface_uses_only_local_authentication(self) -> None:
        """The install command has no external identity or certificate-input options."""

        arguments = SETUP.parser().parse_args(
            [
                "install",
                "--server-ip", "10.20.30.40",
                "--gerrit-sha256", "a" * 64,
                "--project-name", "MyPiCarX",
            ]
        )
        self.assertEqual(arguments.admin_username, "joxy")
        self.assertEqual(arguments.admin_full_name, "Joxy John")
        self.assertEqual(arguments.admin_role, "Student")
        self.assertEqual(arguments.admin_email, "joxjoh24@student.hh.se")
        self.assertEqual(
            set(vars(arguments)),
            {
                "admin_email", "admin_full_name", "admin_role", "admin_username",
                "caddy_sha256", "caddy_url", "command", "gerrit_sha256",
                "gerrit_url", "https_port", "http_listen_url", "http_port", "jdk_sha256",
                "jdk_url", "canonical_web_url", "process_manager", "project_branch",
                "project_name", "server_ip", "ssh_listen_address", "ssh_port", "storage_path",
            },
        )

    def test_initialization_installs_standard_download_commands(self) -> None:
        """Install Gerrit's official download command provider from the verified WAR."""

        arguments = types.SimpleNamespace(http_port=18080, ssh_port=29418)
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = pathlib.Path(temporary_directory)
            with mock.patch.object(SETUP, "run") as run_command:
                with mock.patch.object(SETUP, "git_config"):
                    SETUP.initialize_site(arguments, root / "site", root / "java", root / "gerrit.war")
            command = run_command.call_args_list[0].args[0]
            self.assertIn("--install-plugin=download-commands", command)

    def test_final_configuration_offers_authenticated_downloads(self) -> None:
        """Offer standard commands through SSH and authenticated HTTPS."""

        arguments = types.SimpleNamespace(
            server_ip="10.20.30.40", https_port=18443, ssh_port=29418,
            http_port=18080, canonical_web_url="https://review.example:18443/",
            ssh_listen_address="10.20.30.40:29418",
            http_listen_url="proxy-https://127.0.0.1:18080/",
        )
        with tempfile.TemporaryDirectory() as temporary_directory:
            config = pathlib.Path(temporary_directory) / "gerrit.config"
            SETUP.configure_final_site(arguments, config)

            def values(key: str) -> list[str]:
                result = subprocess.run(
                    ["git", "config", "--file", str(config), "--get-all", key],
                    check=True, stdout=subprocess.PIPE, text=True,
                )
                return result.stdout.splitlines()

            self.assertEqual(values("download.command"), ["checkout", "cherry_pick", "pull", "format_patch"])
            self.assertEqual(values("download.scheme"), ["ssh", "http"])
            self.assertEqual(values("download.archive"), ["tar", "tgz"])

    def test_local_authentication_selects_pinned_architecture_artifact(self) -> None:
        """Supported Linux architectures receive a complete pinned Caddy artifact pair."""

        arguments = types.SimpleNamespace(caddy_url=None, caddy_sha256=None)
        SETUP.select_caddy_artifact(arguments, "x86_64")
        self.assertIn("caddy_2.11.3_linux_amd64.tar.gz", arguments.caddy_url)
        self.assertEqual(len(arguments.caddy_sha256), 64)
        arguments = types.SimpleNamespace(caddy_url="https://example.invalid/caddy.tar.gz", caddy_sha256=None)
        with self.assertRaisesRegex(SETUP.SetupError, "both"):
            SETUP.select_caddy_artifact(arguments, "x86_64")

    @unittest.skipUnless(os.environ.get("XWALK_CADDY_TEST_ARCHIVE"), "real Caddy archive not provided")
    def test_pinned_caddy_archive_extracts_verified_binary(self) -> None:
        """The official archive digest and layout produce the expected portable binary."""

        source = pathlib.Path(os.environ["XWALK_CADDY_TEST_ARCHIVE"])
        unused_url, expected_digest = SETUP.CADDY_ARTIFACTS["x86_64"]
        self.assertEqual(SETUP.sha256(source), expected_digest)
        arguments = types.SimpleNamespace(
            caddy_url="https://example.invalid/caddy.tar.gz",
            caddy_sha256=expected_digest,
        )
        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)

            def retain_verified_archive(
                unused_source_url: str,
                supplied_digest: str,
                destination: pathlib.Path,
            ) -> None:
                self.assertEqual(supplied_digest, expected_digest)
                destination.parent.mkdir(parents=True)
                shutil.copyfile(source, destination)

            with mock.patch.object(SETUP, "download_verified", side_effect=retain_verified_archive):
                caddy_home, version = SETUP.install_portable_caddy(arguments, home)
            self.assertTrue((caddy_home / "caddy").is_file())
            self.assertTrue(version.startswith("v2.11.3"))

    def test_local_caddy_configuration_trusts_only_its_authenticated_user(self) -> None:
        """The generated proxy strips credentials and supplies its own identity header."""

        arguments = caddy_arguments("10.20.30.40", 18443, 18080)
        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            (home / "gerrit-site" / "logs").mkdir(parents=True)
            configuration = SETUP.write_caddy_configuration(
                arguments, home, home / "gerrit-site", home / "certificate.pem",
                home / "private-key.pem",
                "$2a$14$" + "A" * 53,
            )
            content = configuration.read_text(encoding="utf-8")
            users = (home / "gerrit-proxy" / "users.caddy").read_text(encoding="utf-8")
            expected = (
                "https://10.20.30.40:18443", "bind 10.20.30.40",
                "reverse_proxy 127.0.0.1:18080",
                "@public_git_info", "/DevloperNote/info/refs", "/xWalkHal/info/refs",
                "/xWalkLibrary/info/refs", "/xWalkTrace/info/refs",
                "@public_git_upload", "/DevloperNote/git-upload-pack",
                "@git_http path */info/refs */git-upload-pack */git-receive-pack",
                "handle @git_http", "handle_path /ci/*", "reverse_proxy 127.0.0.1:8091",
                "@login path /login /login/*", "handle @login",
                "header_up -Authorization", "header_up -X-Forwarded-For",
                "header_up -X-Gerrit-User",
                "header_up X-Gerrit-User {http.auth.user.id}",
            )
            for value in expected:
                self.assertIn(value, content)
            login_route = content.split("@login path", 1)[1].split("        handle {", 1)[0]
            self.assertNotIn("header_up -X-Gerrit-User", login_route)
            self.assertTrue(users.startswith("joxy $2a$14$"))

    @unittest.skipUnless(os.environ.get("XWALK_CADDY_TEST_BINARY"), "real Caddy binary not provided")
    def test_local_caddy_configuration_validates_with_real_binary(self) -> None:
        """The reviewed Caddyfile loads in the explicitly supplied official binary."""

        arguments = caddy_arguments("127.0.0.2", 18443, 18080)
        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            fixture = create_caddy_fixture(home, arguments)
            result = subprocess.run(
                [str(fixture.binary), "validate", "--config", str(fixture.configuration)],
                check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)

    @unittest.skipUnless(os.environ.get("XWALK_CADDY_TEST_BINARY"), "real Caddy binary not provided")
    def test_local_caddy_control_owns_its_exact_process(self) -> None:
        """The user-owned control script starts, reloads, and stops only its pinned Caddy."""

        https_port, backend_port = available_ports()
        arguments = caddy_arguments("127.0.0.1", https_port, backend_port)
        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            fixture = create_caddy_fixture(home, arguments)
            SETUP.write_environment(
                home, fixture.site, pathlib.Path("/unused-java"), "local-http", fixture.caddy_home
            )
            bin_directory = home / "bin"
            bin_directory.mkdir()
            for name in ("gerrit-caddy-control", "gerrit-site-check"):
                target = bin_directory / name
                shutil.copyfile(GERRIT_ROOT / "bin" / name, target)
                target.chmod(0o700)
            control = bin_directory / "gerrit-caddy-control"
            environment = os.environ.copy()
            environment["HOME"] = str(home)
            try:
                for action in ("start", "status", "reload"):
                    run_control(control, environment, action)
                self.assertEqual(response_status(fixture.certificate, https_port, "/login/"), "401")
                git_path = "/project/info/refs?service=git-upload-pack"
                self.assertEqual(response_status(fixture.certificate, https_port, git_path), "401")
                public_git_path = "/xWalkHal/info/refs?service=git-upload-pack"
                self.assertEqual(
                    response_status(fixture.certificate, https_port, public_git_path), "502"
                )
                self.assertEqual(response_status(fixture.certificate, https_port, "/"), "502")
            finally:
                run_control(control, environment, "stop")
            self.assertFalse((fixture.site / "logs" / "caddy.pid").exists())

    def test_curl_configuration_escapes_credentials(self) -> None:
        """Local validation credentials cannot inject curl configuration fields."""

        self.assertEqual(SETUP.curl_config_value('a\\b"c'), 'a\\\\b\\"c')
        with self.assertRaises(SETUP.SetupError):
            SETUP.curl_config_value("unsafe\nurl")

    def test_shell_templates_are_valid_and_do_not_use_sudo(self) -> None:
        """Every installed management command is valid POSIX shell and non-root."""

        template_directory = GERRIT_ROOT / "bin"
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

    def test_management_check_accepts_owned_site_outside_home_with_spaces(self) -> None:
        """Generated controls accept one resolved owner-managed shared Gerrit site."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            root = pathlib.Path(temporary_directory)
            home = root / "user home"
            commands = home / "bin"
            commands.mkdir(parents=True)
            site = root / "shared disk" / "gerrit site"
            site.mkdir(parents=True)
            (home / "gerrit-env.sh").write_text(
                f"export GERRIT_SITE='{site}'\n", encoding="utf-8"
            )
            checker = commands / "gerrit-site-check"
            shutil.copyfile(GERRIT_ROOT / "bin" / checker.name, checker)
            checker.chmod(0o700)
            result = subprocess.run(
                [str(checker)], check=False, env={**os.environ, "HOME": str(home)},
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), str(site))

    def test_management_commands_start_restart_status_and_stop_shared_site(self) -> None:
        """User controls operate one exact shared site and shut it down cleanly."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            root = pathlib.Path(temporary_directory)
            home = root / "home"
            commands = home / "bin"
            commands.mkdir(parents=True)
            site = root / "shared path" / "gerrit-site"
            control_directory = site / "bin"
            control_directory.mkdir(parents=True)
            state = site / "running"
            control = control_directory / "gerrit.sh"
            control.write_text(
                "#!/bin/sh\ncase \"$1\" in\n"
                f"start) touch '{state}' ;;\n"
                f"stop) rm -f '{state}' ;;\n"
                f"status) test -f '{state}' ;;\n"
                "*) exit 2 ;;\nesac\n",
                encoding="utf-8",
            )
            control.chmod(0o700)
            (home / "gerrit-env.sh").write_text(
                f"export GERRIT_SITE='{site}'\nexport GERRIT_PROXY_MODE='none'\n",
                encoding="utf-8",
            )
            for name in ("gerrit-site-check", "gerrit-start", "gerrit-stop",
                         "gerrit-restart", "gerrit-status"):
                target = commands / name
                shutil.copyfile(GERRIT_ROOT / "bin" / name, target)
                target.chmod(0o700)
            environment = {**os.environ, "HOME": str(home)}
            for action in ("gerrit-start", "gerrit-status", "gerrit-restart", "gerrit-stop"):
                result = subprocess.run(
                    [str(commands / action)], check=False, env=environment,
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
            self.assertFalse(state.exists())

    def test_management_commands_include_configured_ci_lifecycle(self) -> None:
        """Start, status, restart, and stop include CI when its environment exists."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            commands = home / "bin"
            commands.mkdir()
            site = home / "gerrit-site"
            control_directory = site / "bin"
            control_directory.mkdir(parents=True)
            gerrit_state = site / "gerrit-running"
            ci_state = site / "ci-running"
            gerrit_control = control_directory / "gerrit.sh"
            gerrit_control.write_text(
                "#!/bin/sh\ncase \"$1\" in\n"
                f"start) touch '{gerrit_state}' ;;\n"
                f"stop) rm -f '{gerrit_state}' ;;\n"
                f"status) test -f '{gerrit_state}' ;;\n"
                "*) exit 2 ;;\nesac\n",
                encoding="utf-8",
            )
            gerrit_control.chmod(0o700)
            (home / "gerrit-env.sh").write_text(
                f"export GERRIT_SITE='{site}'\nexport GERRIT_PROXY_MODE='none'\n",
                encoding="utf-8",
            )
            (home / ".xwalk-ci.env").write_text("GERRIT_PROJECT=test\n", encoding="utf-8")
            ci_control = commands / "gerrit-ci-control"
            ci_control.write_text(
                "#!/bin/sh\ncase \"$1\" in\n"
                f"start) test -f '{gerrit_state}' && touch '{ci_state}' ;;\n"
                f"stop) rm -f '{ci_state}' ;;\n"
                f"status) test -f '{ci_state}' ;;\n"
                "*) exit 2 ;;\nesac\n",
                encoding="utf-8",
            )
            ci_control.chmod(0o700)
            for name in (
                "gerrit-site-check", "gerrit-start", "gerrit-stop",
                "gerrit-restart", "gerrit-status",
            ):
                target = commands / name
                shutil.copyfile(GERRIT_ROOT / "bin" / name, target)
                target.chmod(0o700)
            environment = {**os.environ, "HOME": str(home)}
            for action in ("gerrit-start", "gerrit-status", "gerrit-restart"):
                result = subprocess.run(
                    [str(commands / action)], check=False, env=environment,
                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                )
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertTrue(gerrit_state.is_file())
                self.assertTrue(ci_state.is_file())
            result = subprocess.run(
                [str(commands / "gerrit-stop")], check=False, env=environment,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
            )
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertFalse(gerrit_state.exists())
            self.assertFalse(ci_state.exists())

    def test_management_start_rolls_back_new_gerrit_when_ci_fails(self) -> None:
        """A configured CI startup failure stops Gerrit started by the same command."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            commands = home / "bin"
            commands.mkdir()
            site = home / "gerrit-site"
            control_directory = site / "bin"
            control_directory.mkdir(parents=True)
            state = site / "gerrit-running"
            control = control_directory / "gerrit.sh"
            control.write_text(
                "#!/bin/sh\ncase \"$1\" in\n"
                f"start) touch '{state}' ;;\n"
                f"stop) rm -f '{state}' ;;\n"
                f"status) test -f '{state}' ;;\n"
                "*) exit 2 ;;\nesac\n",
                encoding="utf-8",
            )
            control.chmod(0o700)
            (home / "gerrit-env.sh").write_text(
                f"export GERRIT_SITE='{site}'\nexport GERRIT_PROXY_MODE='none'\n",
                encoding="utf-8",
            )
            (home / ".xwalk-ci.env").write_text("GERRIT_PROJECT=test\n", encoding="utf-8")
            ci_control = commands / "gerrit-ci-control"
            ci_control.write_text("#!/bin/sh\nexit 1\n", encoding="utf-8")
            ci_control.chmod(0o700)
            for name in ("gerrit-site-check", "gerrit-start"):
                target = commands / name
                shutil.copyfile(GERRIT_ROOT / "bin" / name, target)
                target.chmod(0o700)
            result = subprocess.run(
                [str(commands / "gerrit-start")], check=False,
                env={**os.environ, "HOME": str(home)},
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
            )
            self.assertEqual(result.returncode, 1)
            self.assertFalse(state.exists())

    def test_setup_script(self) -> None:
        """The repository setup command uses the Python installer and safe controls."""

        script = GERRIT_ROOT / "shell-script" / "gerrit-setup.sh"
        result = subprocess.run(
            ["sh", "-n", str(script)], check=False,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        contents = script.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("py-src/xWalkGerritServerSetup.py", contents)
        self.assertIn("config/gerrit-setup.conf", contents)
        self.assertIn("GERRIT_ADMIN_PASSWORD", contents)
        self.assertIn('"$HOME/bin/gerrit-start"', contents)
        self.assertIn('"$HOME/bin/gerrit-check"', contents)
        self.assertNotIn("sudo", contents)

    def test_local_script(self) -> None:
        """The local entry point selects its isolated configuration."""

        script = GERRIT_ROOT / "local-linux" / "gerrit-local.sh"
        result = subprocess.run(
            ["sh", "-n", str(script)], check=False,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
        )
        contents = script.read_text(encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("gerrit-local.conf", contents)
        self.assertIn("XWALK_GERRIT_SETUP_CONFIG", contents)
        self.assertNotIn("sudo", contents)

    def test_local_conf(self) -> None:
        """The local configuration contains no administrator password."""

        config = GERRIT_ROOT / "local-linux" / "gerrit-local.conf"
        contents = config.read_text(encoding="utf-8")
        self.assertIn("GERRIT_SERVER_IP", contents)
        self.assertIn("GERRIT_SHA256", contents)
        self.assertNotIn("GERRIT_ADMIN_PASSWORD=", contents)

    def test_setup_start(self) -> None:
        """Start a stopped installed service and require its validation command."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            commands = home / "bin"
            commands.mkdir()
            scripts = {
                "gerrit-status": "#!/bin/sh\nexit 1\n",
                "gerrit-start": "#!/bin/sh\ntouch \"$HOME/started\"\n",
                "gerrit-check": "#!/bin/sh\ntest -e \"$HOME/started\"\n",
            }
            for name, contents in scripts.items():
                command = commands / name
                command.write_text(contents, encoding="utf-8")
                command.chmod(0o700)
            environment = {**os.environ, "HOME": str(home)}
            script = GERRIT_ROOT / "shell-script" / "gerrit-setup.sh"
            result = subprocess.run([str(script), "start"], check=False, env=environment)
            self.assertEqual(result.returncode, 0)
            self.assertTrue((home / "started").is_file())

    def test_start_proxy(self) -> None:
        """Restart a stopped proxy when Gerrit itself is already running."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            commands = home / "bin"
            commands.mkdir()
            (home / "gerrit-env.sh").write_text("GERRIT_PROXY_MODE=local-http\n", encoding="utf-8")
            scripts = {
                "gerrit-status": "#!/bin/sh\nexit 0\n",
                "gerrit-start": "#!/bin/sh\nexit 99\n",
                "gerrit-caddy-control": "#!/bin/sh\n[ \"$1\" = start ] && touch \"$HOME/proxy\"\n",
                "gerrit-check": "#!/bin/sh\ntest -e \"$HOME/proxy\"\n",
            }
            for name, contents in scripts.items():
                command = commands / name
                command.write_text(contents, encoding="utf-8")
                command.chmod(0o700)
            script = GERRIT_ROOT / "shell-script" / "gerrit-setup.sh"
            result = subprocess.run([str(script), "start"], check=False, env={**os.environ, "HOME": str(home)})
            self.assertEqual(result.returncode, 0)

    def test_documentation_templates_render_without_placeholders(self) -> None:
        """Rendered guides contain concrete endpoints and valid SSH user syntax."""

        with tempfile.TemporaryDirectory() as temporary_directory:
            home = pathlib.Path(temporary_directory)
            SETUP.copy_templates(GERRIT_ROOT, home, documentation_replacements())
            documentation = home / "gerrit-site" / "docs"
            self.assertEqual(expected_documentation_names(), {path.name for path in documentation.iterdir()})
            guides = "\n".join(
                path.read_text(encoding="utf-8")
                for path in documentation.iterdir()
            )
            installed_commands = "\n".join(
                path.read_text(encoding="utf-8")
                for path in (home / "bin").iterdir()
            )
            self.assertNotIn("@@", guides)
            self.assertNotIn("@@", installed_commands)
            self.assertIn("USERNAME@10.20.30.40", guides)
            self.assertIn("ssh -p 29418 joxy@10.20.30.40 gerrit set-account", guides)
            self.assertIn("--full-name 'Joxy John' --add-email joxjoh24@student.hh.se joxy", guides)
            self.assertIn("| Role | `Student` |", guides)
            self.assertIn("https://10.20.30.40:18443/q/project:MyPiCarX", guides)
            self.assertIn("gerrit.canonicalWebUrl", guides)
            self.assertIn("refs/for/master", guides)
            self.assertIn("git fetch ssh://your_gerrit_username@10.20.30.40:29418/MyPiCarX", guides)
            self.assertIn("git fetch https://your_gerrit_username@10.20.30.40:18443/MyPiCarX", guides)
            self.assertIn("git checkout FETCH_HEAD", guides)
            self.assertIn("git cherry-pick FETCH_HEAD", guides)
            self.assertIn("git clone ssh://your_gerrit_username@10.20.30.40:29418/MyPiCarX", guides)
            self.assertIn("git clone https://your_gerrit_username@10.20.30.40:18443/MyPiCarX", guides)
            local_guide = (documentation / "Gerrit Local Linux Setup.md").read_text(encoding="utf-8")
            self.assertIn("## Local Download and review workflow", local_guide)
            self.assertIn("git push ssh://your_gerrit_username@10.20.30.40:29418/MyPiCarX "
                          "HEAD:refs/for/master", local_guide)
            self.assertIn("git push https://your_gerrit_username@10.20.30.40:18443/MyPiCarX "
                          "HEAD:refs/for/master", local_guide)
            self.assertIn("Patch File → Zip", local_guide)
            self.assertIn('username" != "joxy', installed_commands)
            self.assertTrue((home / "apps" / "gerrit" / "tools" / "xWalkGerritCi.py").is_file())
            self.assertTrue((home / "gerrit-site" / "plugins" / "xWalkReviewControls.js").is_file())
            self.assertIn("XWALK_CI_LOG_WEB_URL=https://10.20.30.40:18443/ci", guides)

    def test_readmes_document_standard_download_button_commands(self) -> None:
        """Document SSH, HTTP, clone, checkout, cherry-pick, and zipped patches."""

        installer = (GERRIT_ROOT / "README.md").read_text(encoding="utf-8")
        local = (GERRIT_ROOT / "local-linux" / "README.md").read_text(encoding="utf-8")
        expected = (
            "git checkout FETCH_HEAD", "git cherry-pick FETCH_HEAD", "git clone ssh://",
            "git clone https://", "git push ssh://", "git push https://",
            "HEAD:refs/for/", "Patch File → Zip",
        )
        for value in expected:
            self.assertIn(value, installer)
            self.assertIn(value, local)
        self.assertIn("## Local Gerrit", installer)
        self.assertIn("ssh://joxy@${GERRIT_SERVER_HOST}:${GERRIT_SSH_PORT}/xWalk-rpi5", local)
        self.assertIn("https://joxy@${GERRIT_SERVER_HOST}:${GERRIT_HTTPS_PORT}/xWalk-rpi5", local)
        self.assertIn("HEAD:refs/for/main", local)

    def test_installer_has_no_forbidden_execution_commands(self) -> None:
        """The executable installer does not contain privileged management commands."""

        source = MODULE_PATH.read_text(encoding="utf-8")
        for forbidden in ("sudo", "apt-get", "dnf", "yum", "zypper", "snap"):
            self.assertNotIn(forbidden, source)


if __name__ == "__main__":
    unittest.main()
