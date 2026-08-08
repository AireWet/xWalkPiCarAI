#!/usr/bin/env python3
"""Test secure xWalk netrc credential loading without using the real home file."""

from __future__ import annotations

import contextlib
import io
import os
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import mock

from xWalkJiraImport.xWalkJiraImportApplication import print_credential_status
from xWalkJiraImport.xWalkJiraImportConfig import ConfigurationError, load_config
from xWalkJiraImport.xWalkJiraImportCredentials import (
    MalformedNetrcError,
    MissingNetrcError,
    UnsafeNetrcPermissionsError,
    load_credentials,
)
from xWalkJiraImport.xWalkJiraImportGitHubClient import GitHubClient
from xWalkJiraImport.xWalkJiraImportJiraClient import JiraClient
from xWalkJiraImport.xWalkJiraImportModels import ImportSummary
from xWalkJiraImport.xWalkJiraImportReport import write_reports


GITHUB_TOKEN = "fake-github-token-must-remain-secret"
JIRA_TOKEN = "fake-jira-token-must-remain-secret"
JIRA_EMAIL = "student@example.invalid"


class HeaderSession:
    """Provide the mutable authentication surface used by client constructors."""

    def __init__(self) -> None:
        """Start without inherited headers or authentication."""
        self.headers: dict[str, str] = {}
        self.auth: tuple[str, str] | None = None
        self.trust_env = True


class NetrcCredentialTest(unittest.TestCase):
    """Verify credential precedence, validation, and request isolation."""

    def setUp(self) -> None:
        """Create a private fake home directory for every test."""
        self.temporary_directory = TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.directory = Path(self.temporary_directory.name)

    def write_netrc(self, content: str, mode: int = 0o600, name: str = ".netrc") -> Path:
        """Create one fake credential file with an explicit permission mode."""
        path = self.directory / name
        path.write_text(content, encoding="utf-8")
        path.chmod(mode)
        return path

    @staticmethod
    def complete_content(jira_host: str = "student-team-xwalk-rpi5.atlassian.net") -> str:
        """Return valid fake GitHub and Jira entries."""
        return (
            f"machine api.github.com login jochuuu password {GITHUB_TOKEN}\n"
            f"machine {jira_host} login {JIRA_EMAIL} password {JIRA_TOKEN}\n"
        )

    def test_default_path_uses_path_home_without_real_home_access(self) -> None:
        """The default resolves to the patched home directory's standard filename."""
        expected = self.write_netrc(self.complete_content())
        with mock.patch(
            "xWalkJiraImport.xWalkJiraImportCredentials.Path.home",
            return_value=self.directory,
        ):
            configuration = load_config([], {})
        self.assertEqual(configuration.netrc_file, expected.resolve())

    def test_custom_netrc_file_overrides_default(self) -> None:
        """An explicit path is resolved and used before the default location."""
        selected = self.write_netrc(self.complete_content(), name="custom.netrc")
        configuration = load_config(["--netrc-file", str(selected)], {})
        self.assertEqual(configuration.netrc_file, selected.resolve())
        self.assertEqual(configuration.credential_source, "netrc")

    def test_tilde_in_custom_path_uses_path_home(self) -> None:
        """An explicit tilde path expands through Path.home before resolution."""
        expected = self.write_netrc(self.complete_content(), name="selected.netrc")
        with mock.patch(
            "xWalkJiraImport.xWalkJiraImportCredentials.Path.home",
            return_value=self.directory,
        ):
            configuration = load_config(["--netrc-file", "~/selected.netrc"], {})
        self.assertEqual(configuration.netrc_file, expected.resolve())

    def test_missing_netrc_has_a_safe_error(self) -> None:
        """A missing selected file reports no home-directory or credential content."""
        missing = self.directory / "missing.netrc"
        with self.assertRaisesRegex(MissingNetrcError, "does not exist"):
            load_credentials(missing, "api.github.com", "student-team-xwalk-rpi5.atlassian.net")

    def test_command_line_non_secret_values_override_environment(self) -> None:
        """CLI repository, branch, project, and Jira URL values have highest precedence."""
        jira_host = "command-team.atlassian.net"
        path = self.write_netrc(self.complete_content(jira_host))
        configuration = load_config(
            [
                "--netrc-file",
                str(path),
                "--github-repository",
                "command/repository",
                "--branch",
                "command-branch",
                "--jira-url",
                f"https://{jira_host}",
                "--jira-project-key",
                "cmd",
            ],
            {
                "GITHUB_REPOSITORY": "environment/repository",
                "GITHUB_BRANCH": "environment-branch",
                "JIRA_URL": "https://environment.atlassian.net",
                "JIRA_PROJECT_KEY": "ENV",
            },
        )
        self.assertEqual(configuration.github_repository, "command/repository")
        self.assertEqual(configuration.github_branch, "command-branch")
        self.assertEqual(configuration.jira_url, f"https://{jira_host}")
        self.assertEqual(configuration.jira_project_key, "CMD")

    def test_loads_exact_github_and_jira_machines(self) -> None:
        """The standard GitHub host and requested Jira host return their credentials."""
        path = self.write_netrc(self.complete_content())
        credentials = load_credentials(path, "api.github.com", "student-team-xwalk-rpi5.atlassian.net")
        self.assertEqual(credentials.github_login, "jochuuu")
        self.assertEqual(credentials.github_token, GITHUB_TOKEN)
        self.assertEqual(credentials.jira_email, JIRA_EMAIL)
        self.assertEqual(credentials.jira_api_token, JIRA_TOKEN)

    def test_jira_machine_is_derived_from_command_line_url(self) -> None:
        """A Jira URL override selects its hostname rather than a hard-coded machine."""
        jira_host = "team-two.atlassian.net"
        path = self.write_netrc(self.complete_content(jira_host))
        configuration = load_config(
            ["--netrc-file", str(path), "--jira-url", f"https://{jira_host}"],
            {},
        )
        self.assertEqual(configuration.jira_email, JIRA_EMAIL)

    def test_incorrect_jira_hostname_reports_the_requested_machine(self) -> None:
        """Apply mode identifies a Jira URL whose hostname has no matching entry."""
        path = self.write_netrc(self.complete_content())
        wrong_host = "wrong-team.atlassian.net"
        with self.assertRaisesRegex(ConfigurationError, wrong_host):
            load_config(
                [
                    "--netrc-file",
                    str(path),
                    "--jira-url",
                    f"https://{wrong_host}",
                    "--apply",
                ],
                {},
            )

    def test_missing_github_entry_allows_unauthenticated_access(self) -> None:
        """The public repository remains usable without a GitHub machine entry."""
        path = self.write_netrc(
            f"machine student-team-xwalk-rpi5.atlassian.net login {JIRA_EMAIL} password {JIRA_TOKEN}\n"
        )
        configuration = load_config(["--netrc-file", str(path)], {})
        self.assertIsNone(configuration.github_token)
        self.assertTrue(any("lower rate limit" in value for value in configuration.credential_warnings))

    def test_default_netrc_entry_is_not_reused_for_service_hosts(self) -> None:
        """Only exact machine entries may provide GitHub or Jira credentials."""
        path = self.write_netrc(f"default login {JIRA_EMAIL} password {JIRA_TOKEN}\n")
        credentials = load_credentials(path, "api.github.com", "student-team-xwalk-rpi5.atlassian.net")
        self.assertIsNone(credentials.github_token)
        self.assertIsNone(credentials.jira_api_token)

    def test_missing_jira_entry_is_allowed_only_for_dry_run(self) -> None:
        """Jira credentials may be absent for previews but never for apply mode."""
        path = self.write_netrc(f"machine api.github.com login jochuuu password {GITHUB_TOKEN}\n")
        preview = load_config(["--netrc-file", str(path), "--dry-run"], {})
        self.assertFalse(preview.has_jira_credentials)
        with self.assertRaisesRegex(ConfigurationError, "missing Jira machine entry"):
            load_config(["--netrc-file", str(path), "--apply"], {})

    def test_missing_jira_login_and_token_have_specific_errors(self) -> None:
        """Incomplete Jira entries identify the missing field without exposing values."""
        missing_login = self.write_netrc(
            f"machine student-team-xwalk-rpi5.atlassian.net password {JIRA_TOKEN}\n",
            name="missing-login.netrc",
        )
        with self.assertRaisesRegex(ConfigurationError, "missing its login"):
            load_config(["--netrc-file", str(missing_login), "--apply"], {})
        missing_token = self.write_netrc(
            f"machine student-team-xwalk-rpi5.atlassian.net login {JIRA_EMAIL}\n",
            name="missing-token.netrc",
        )
        with self.assertRaisesRegex(ConfigurationError, "missing its API token"):
            load_config(["--netrc-file", str(missing_token), "--apply"], {})

    def test_malformed_netrc_is_safely_rejected(self) -> None:
        """Parser details and file content are not copied into the error."""
        path = self.write_netrc("machine api.github.com unsupported value\n")
        with self.assertRaisesRegex(MalformedNetrcError, "malformed") as raised:
            load_credentials(path, "api.github.com", "student-team-xwalk-rpi5.atlassian.net")
        self.assertNotIn(GITHUB_TOKEN, str(raised.exception))

    def test_malformed_netrc_blocks_apply_before_environment_fallback(self) -> None:
        """Live mode cannot silently bypass a malformed selected credential file."""
        path = self.write_netrc("machine api.github.com unsupported value\n")
        environment = {"JIRA_EMAIL": JIRA_EMAIL, "JIRA_API_TOKEN": JIRA_TOKEN}
        with self.assertRaisesRegex(ConfigurationError, "malformed"):
            load_config(["--netrc-file", str(path), "--apply"], environment)

    @unittest.skipIf(os.name == "nt", "POSIX permission bits are not enforced on Windows")
    def test_unsafe_permissions_block_apply_and_disable_dry_run_authentication(self) -> None:
        """A group-readable file cannot supply credentials to any request."""
        path = self.write_netrc(self.complete_content(), mode=0o640)
        with self.assertRaises(UnsafeNetrcPermissionsError):
            load_credentials(path, "api.github.com", "student-team-xwalk-rpi5.atlassian.net")
        preview = load_config(
            ["--netrc-file", str(path), "--dry-run"],
            {"GITHUB_TOKEN": "ci-token", "JIRA_EMAIL": "ci@example.invalid", "JIRA_API_TOKEN": "ci-jira"},
        )
        self.assertIsNone(preview.github_token)
        self.assertFalse(preview.has_jira_credentials)
        with self.assertRaisesRegex(ConfigurationError, "chmod 600"):
            load_config(["--netrc-file", str(path), "--apply"], {})

    def test_environment_fallback_supports_ci(self) -> None:
        """Missing local netrc values can be supplied by CI-only environment secrets."""
        missing = self.directory / "missing.netrc"
        configuration = load_config(
            ["--netrc-file", str(missing), "--apply"],
            {"GITHUB_TOKEN": GITHUB_TOKEN, "JIRA_EMAIL": JIRA_EMAIL, "JIRA_API_TOKEN": JIRA_TOKEN},
        )
        self.assertEqual(configuration.credential_source, "environment fallback")
        self.assertTrue(configuration.has_jira_credentials)

    def test_netrc_precedes_environment_secrets(self) -> None:
        """Complete netrc entries replace all matching CI fallback variables."""
        path = self.write_netrc(self.complete_content())
        configuration = load_config(
            ["--netrc-file", str(path), "--apply"],
            {
                "GITHUB_TOKEN": "wrong-github-token",
                "JIRA_EMAIL": "wrong@example.invalid",
                "JIRA_API_TOKEN": "wrong-jira-token",
            },
        )
        self.assertEqual(configuration.credential_source, "netrc")
        self.assertEqual(configuration.github_token, GITHUB_TOKEN)
        self.assertEqual(configuration.jira_email, JIRA_EMAIL)
        self.assertEqual(configuration.jira_api_token, JIRA_TOKEN)

    def test_safe_logging_masks_email_and_redacts_tokens(self) -> None:
        """Authentication status output never includes either secret token or full email."""
        path = self.write_netrc(self.complete_content())
        configuration = load_config(["--netrc-file", str(path)], {})
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            print_credential_status(configuration)
        logged = output.getvalue()
        self.assertIn("Credential source: netrc", logged)
        self.assertIn("Jira account: s***@example.invalid", logged)
        self.assertNotIn(GITHUB_TOKEN, logged)
        self.assertNotIn(JIRA_TOKEN, logged)
        self.assertNotIn(JIRA_EMAIL, logged)

        report_base = self.directory / "credential-report"
        json_path, csv_path = write_reports(
            report_base,
            [],
            ImportSummary(limitations=list(configuration.credential_warnings)),
            "dry-run",
        )
        report_text = json_path.read_text(encoding="utf-8") + csv_path.read_text(encoding="utf-8")
        self.assertNotIn(GITHUB_TOKEN, report_text)
        self.assertNotIn(JIRA_TOKEN, report_text)
        self.assertNotIn(JIRA_EMAIL, report_text)

    def test_client_authentication_is_host_and_session_isolated(self) -> None:
        """GitHub and Jira credentials cannot be inherited by unrelated request sessions."""
        github_session = HeaderSession()
        GitHubClient("owner/repository", GITHUB_TOKEN, session=github_session)
        self.assertEqual(github_session.headers["Authorization"], f"Bearer {GITHUB_TOKEN}")
        self.assertFalse(github_session.trust_env)

        unauthenticated_session = HeaderSession()
        GitHubClient("owner/repository", None, session=unauthenticated_session)
        self.assertNotIn("Authorization", unauthenticated_session.headers)

        unrelated_session = HeaderSession()
        GitHubClient(
            "owner/repository",
            GITHUB_TOKEN,
            session=unrelated_session,
            api_url="https://api.example.invalid",
        )
        self.assertNotIn("Authorization", unrelated_session.headers)

        jira_session = HeaderSession()
        JiraClient(
            "https://student-team-xwalk-rpi5.atlassian.net",
            JIRA_EMAIL,
            JIRA_TOKEN,
            "TARS",
            3,
            session=jira_session,
        )
        self.assertEqual(jira_session.auth, (JIRA_EMAIL, JIRA_TOKEN))
        self.assertNotIn("Authorization", jira_session.headers)
        self.assertFalse(jira_session.trust_env)


if __name__ == "__main__":
    unittest.main()
