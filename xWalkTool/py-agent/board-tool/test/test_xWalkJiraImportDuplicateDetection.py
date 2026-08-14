#!/usr/bin/env python3
"""Test xWalk Jira duplicate prevention, workflows, and dry-run safety."""

from __future__ import annotations

import unittest
from dataclasses import replace
from datetime import date
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import Any, Mapping

from test_xWalkJiraImportCommitAnalyser import make_commit
from test_xWalkJiraImportJiraPayload import FakeResponse, QueueSession
from xWalkJiraImport.xWalkJiraImportApplication import run_import
from xWalkJiraImport.xWalkJiraImportConfig import ConfigurationError, ImportConfig, load_config
from xWalkJiraImport.xWalkJiraImportCredentials import Credentials
from xWalkJiraImport.xWalkJiraImportGitHubClient import GitHubClient
from xWalkJiraImport.xWalkJiraImportJiraClient import JiraClient, JiraSafetyError, select_done_transition
from xWalkJiraImport.xWalkJiraImportModels import ChangedFile, CommitRecord, JiraField, JiraMetadata


class DuplicateAndWorkflowTest(unittest.TestCase):
    """Verify repeatability and the explicit mutation boundary."""

    def test_duplicate_requires_exact_description_marker(self) -> None:
        """A full SHA search result is accepted only with the exact import marker."""
        sha = "a" * 40
        description = {
            "type": "doc",
            "version": 1,
            "content": [
                {"type": "paragraph", "content": [{"type": "text", "text": f"xwalk-git-import:{sha}"}]}
            ],
        }
        session = QueueSession(
            [
                FakeResponse(
                    200,
                    {
                        "issues": [
                            {
                                "key": "TARS-7",
                                "fields": {"description": description, "status": {"name": "Done"}},
                            }
                        ]
                    },
                )
            ]
        )
        client = JiraClient("https://example.atlassian.net", "fake", "fake", "TARS", 3, session=session)
        self.assertEqual(client.find_existing_import(sha), ("TARS-7", "Done"))

    def test_similar_text_without_marker_is_not_duplicate(self) -> None:
        """A summary or short SHA cannot trigger duplicate prevention."""
        session = QueueSession(
            [
                FakeResponse(
                    200,
                    {
                        "issues": [
                            {
                                "key": "TARS-8",
                                "fields": {
                                    "description": {
                                        "type": "doc",
                                        "version": 1,
                                        "content": [
                                            {"type": "paragraph", "content": [{"type": "text", "text": "similar"}]}
                                        ],
                                    }
                                },
                            }
                        ]
                    },
                )
            ]
        )
        client = JiraClient("https://example.atlassian.net", "fake", "fake", "TARS", 3, session=session)
        self.assertIsNone(client.find_existing_import("a" * 40))

    def test_read_only_client_blocks_create_and_transition(self) -> None:
        """No Jira mutation request can pass through a dry-run client."""
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            allow_writes=False,
            session=QueueSession([]),
        )
        with self.assertRaises(JiraSafetyError):
            client.create_issue({"fields": {}})
        with self.assertRaises(JiraSafetyError):
            client.transition_to_done("TARS-1")
        with self.assertRaises(JiraSafetyError):
            client.update_original_estimate("TARS-1", "8h")
        with self.assertRaises(JiraSafetyError):
            client.update_planning("TARS-1", "[HW] Work", "customfield_1", date(2026, 9, 1), date(2026, 9, 1))
        with self.assertRaises(JiraSafetyError):
            client.assign_issue_to_sprint("TARS-1", 35)
        with self.assertRaises(JiraSafetyError):
            client.update_parent("TARS-1", "TARS-15")

    def test_updates_original_estimate_with_guarded_jira_payload(self) -> None:
        """Apply-enabled clients update only Jira's time-tracking estimate field."""
        session = QueueSession([FakeResponse(204)])
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            allow_writes=True,
            session=session,
        )
        client.update_original_estimate("TARS-1", "24h")
        method, url, arguments = session.requests[0]
        self.assertEqual(method, "PUT")
        self.assertEqual(url, "https://example.atlassian.net/rest/api/3/issue/TARS-1")
        self.assertEqual(
            arguments["json"],
            {"fields": {"timetracking": {"originalEstimate": "24h"}}},
        )

    def test_reads_anchor_due_date_and_updates_sequential_planning(self) -> None:
        """Planning updates use the anchor Due date and dynamically discovered Start date field."""
        session = QueueSession([FakeResponse(200, {"fields": {"duedate": "2026-08-31"}}), FakeResponse(204)])
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            allow_writes=True,
            session=session,
        )
        self.assertEqual(client.get_due_date("TARS-15"), date(2026, 8, 31))
        client.update_planning(
            "TARS-17",
            "[HW] Reorganize Controller and Agent architecture",
            "customfield_10015",
            date(2026, 9, 1),
            date(2026, 9, 14),
        )
        method, url, arguments = session.requests[1]
        self.assertEqual(method, "PUT")
        self.assertEqual(url, "https://example.atlassian.net/rest/api/3/issue/TARS-17")
        self.assertEqual(
            arguments["json"],
            {
                "fields": {
                    "summary": "[HW] Reorganize Controller and Agent architecture",
                    "customfield_10015": "2026-09-01",
                    "duedate": "2026-09-14",
                }
            },
        )

    def test_selects_sprint_by_due_date_and_verifies_assignment(self) -> None:
        """Sprint assignment follows the board's existing half-open date ranges."""
        session = QueueSession(
            [
                FakeResponse(
                    200,
                    {
                        "values": [
                            {
                                "id": 34,
                                "name": "TARS PHASE I-2026-W16-W18-S7",
                                "startDate": "2026-07-07T22:00:00.000Z",
                                "endDate": "2026-07-28T22:00:00.000Z",
                            },
                            {
                                "id": 35,
                                "name": "TARS PHASE I-2026-W16-W18-S8",
                                "startDate": "2026-07-28T22:00:00.000Z",
                                "endDate": "2026-08-18T22:00:00.000Z",
                            },
                        ],
                        "isLast": True,
                        "total": 2,
                    },
                ),
                FakeResponse(204),
                FakeResponse(200, {"issues": [{"key": "TARS-18"}]}),
            ]
        )
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            allow_writes=True,
            session=session,
        )
        sprint_id, name = client.find_sprint_for_date(date(2026, 8, 6))
        self.assertEqual((sprint_id, name), (35, "TARS PHASE I-2026-W16-W18-S8"))
        client.assign_issue_to_sprint("TARS-18", sprint_id)
        self.assertTrue(client.issue_matches_sprint("TARS-18", sprint_id))
        self.assertEqual(session.requests[1][0], "POST")
        self.assertEqual(session.requests[1][2]["json"], {"issues": ["TARS-18"]})

    def test_validates_epic_and_verifies_parent_relationship(self) -> None:
        """Existing imports use Jira's standard parent field only after Epic validation."""
        session = QueueSession(
            [
                FakeResponse(200, {"fields": {"issuetype": {"name": "Epic"}}}),
                FakeResponse(204),
                FakeResponse(200, {"fields": {"parent": {"key": "TARS-15"}}}),
            ]
        )
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            allow_writes=True,
            session=session,
        )
        client.require_epic("TARS-15")
        client.update_parent("TARS-18", "TARS-15")
        self.assertTrue(client.issue_has_parent("TARS-18", "TARS-15"))
        self.assertEqual(
            session.requests[1][2]["json"],
            {"fields": {"parent": {"key": "TARS-15"}}},
        )

    def test_board_membership_uses_board_issue_endpoint(self) -> None:
        """Board verification delegates saved-filter evaluation to Jira Agile."""
        session = QueueSession([FakeResponse(200, {"issues": [{"key": "TARS-1"}]})])
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            session=session,
        )
        self.assertTrue(client.issue_matches_board("TARS-1"))
        method, url, arguments = session.requests[0]
        self.assertEqual(method, "GET")
        self.assertEqual(url, "https://example.atlassian.net/rest/agile/1.0/board/3/issue")
        self.assertEqual(arguments["params"]["jql"], 'key = "TARS-1"')

    def test_backlog_membership_uses_board_backlog_endpoint(self) -> None:
        """Backlog verification rejects issues that Jira does not expose in the board backlog."""
        session = QueueSession([FakeResponse(200, {"issues": [{"key": "TARS-1"}]})])
        client = JiraClient(
            "https://example.atlassian.net",
            "fake",
            "fake",
            "TARS",
            3,
            session=session,
        )
        self.assertTrue(client.issue_matches_backlog("TARS-1"))
        method, url, arguments = session.requests[0]
        self.assertEqual(method, "GET")
        self.assertEqual(url, "https://example.atlassian.net/rest/agile/1.0/board/3/backlog")
        self.assertEqual(arguments["params"]["jql"], 'key = "TARS-1"')

    def test_selects_direct_done_transition(self) -> None:
        """Done is selected before intermediate workflow steps."""
        transitions = [
            {"id": "1", "to": {"id": "10", "name": "In Progress", "statusCategory": {"key": "indeterminate"}}},
            {"id": "2", "to": {"id": "20", "name": "Resolved", "statusCategory": {"key": "done"}}},
        ]
        self.assertEqual(select_done_transition(transitions)["id"], "2")

    def test_prefers_done_over_canceled_status_category(self) -> None:
        """A terminal cancellation cannot take precedence over completed work."""
        transitions = [
            {"id": "1", "to": {"id": "10", "name": "Canceled", "statusCategory": {"key": "done"}}},
            {"id": "2", "to": {"id": "20", "name": "Done", "statusCategory": {"key": "done"}}},
        ]
        self.assertEqual(select_done_transition(transitions)["id"], "2")

    def test_selects_forward_intermediate_transition_without_loop(self) -> None:
        """A valid forward step is selected when Done is not directly available."""
        transitions = [
            {"id": "1", "to": {"id": "10", "name": "To Do", "statusCategory": {"key": "new"}}},
            {"id": "2", "to": {"id": "20", "name": "In Progress", "statusCategory": {"key": "indeterminate"}}},
        ]
        self.assertEqual(select_done_transition(transitions, {"10"})["id"], "2")


class GitHubPaginationTest(unittest.TestCase):
    """Verify GitHub summary and changed-file pagination."""

    def test_collects_oldest_commits_first_before_applying_limit(self) -> None:
        """The first Jira candidate is the earliest selected branch commit."""
        newest_first = [
            {
                "sha": character * 40,
                "commit": {"author": {"name": "A", "email": "a@example.invalid"}},
                "parents": [],
            }
            for character in ("c", "b", "a")
        ]

        class OrderedGitHubClient(GitHubClient):
            """Provide deterministic summary and detail results without network access."""

            requested_shas: list[str]

            def __init__(self) -> None:
                super().__init__("owner/repository", api_url="https://api.example.invalid")
                self.requested_shas = []

            def _list_commit_summaries(
                self,
                branch: str,
                since: date | None,
                until: date | None,
            ) -> list[Mapping[str, Any]]:
                return newest_first

            def get_commit(self, sha: str) -> CommitRecord:
                self.requested_shas.append(sha)
                fixture = make_commit(
                    f"Implement {sha[:7]}",
                    [ChangedFile("src/control.py", "modified", 1, 0, 1, patch="+change")],
                )
                return replace(fixture, sha=sha, short_sha=sha[:7])

        client = OrderedGitHubClient()
        commits = client.collect_commits("master", max_commits=2)
        self.assertEqual([commit.sha for commit in commits], ["a" * 40, "b" * 40])
        self.assertEqual(client.requested_shas, ["a" * 40, "b" * 40])

    def test_commit_listing_requests_all_pages(self) -> None:
        """A full 100-item page triggers the next page."""
        first_page = [
            {
                "sha": f"{index:040x}",
                "commit": {"author": {"name": "A", "email": "a@example.invalid"}},
                "parents": [],
            }
            for index in range(100)
        ]
        session = QueueSession([FakeResponse(200, first_page), FakeResponse(200, [first_page[0]])])
        client = GitHubClient("owner/repository", session=session, api_url="https://api.example.invalid")
        results = client._list_commit_summaries("master", None, None)
        self.assertEqual(len(results), 101)
        self.assertEqual(len(session.requests), 2)

    def test_commit_details_request_all_file_pages(self) -> None:
        """A commit with over 100 files is reconstructed from every detail page."""
        sha = "a" * 40
        common = {
            "sha": sha,
            "html_url": f"https://github.com/owner/repository/commit/{sha}",
            "commit": {
                "message": "Add feature",
                "author": {"name": "A", "email": "a@example.invalid", "date": "2026-01-01T10:00:00Z"},
                "committer": {"name": "A", "email": "a@example.invalid", "date": "2026-01-01T10:00:00Z"},
            },
            "parents": [],
            "stats": {"additions": 101, "deletions": 0, "total": 101},
        }
        first = dict(common, files=[{"filename": f"src/{index}.py", "status": "added", "changes": 1,
                                    "additions": 1, "deletions": 0, "patch": "+x"} for index in range(100)])
        second = dict(common, files=[{"filename": "src/100.py", "status": "added", "changes": 1,
                                     "additions": 1, "deletions": 0, "patch": "+x"}])
        session = QueueSession([FakeResponse(200, first), FakeResponse(200, second)])
        client = GitHubClient("owner/repository", session=session, api_url="https://api.example.invalid")
        commit = client.get_commit(sha)
        self.assertEqual(len(commit.files), 101)


class DryRunOrchestrationTest(unittest.TestCase):
    """Prove dry-run orchestration never reaches Jira mutation methods."""

    def test_default_configuration_is_dry_run(self) -> None:
        """Omitting both mode flags cannot enable Jira mutation."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            config = load_config(["--netrc-file", str(netrc_path)], {})
        self.assertTrue(config.dry_run)
        self.assertFalse(config.apply)

    def test_apply_requires_both_jira_credentials(self) -> None:
        """Apply mode fails before clients are created when credentials are absent."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(ConfigurationError):
                load_config(["--apply", "--netrc-file", str(netrc_path)], {})

    def test_dry_run_performs_no_create_or_transition(self) -> None:
        """An accepted preview records a skip and no Jira write calls."""
        commit = make_commit(
            "Add camera application",
            [ChangedFile("src/camera/application.py", "added", 30, 0, 30, patch="+run")],
        )

        class FakeGitHub:
            def collect_commits(self, *args: object, **kwargs: object) -> list[object]:
                return [commit]

        class FakeJira:
            create_calls = 0
            transition_calls = 0

            def discover_metadata(self) -> JiraMetadata:
                fields = {
                    "labels": JiraField("labels", "Labels", "array", "", False),
                }
                return JiraMetadata({"Task": "1", "Story": "2"}, {"1": fields, "2": fields}, fields)

            def get_board_filter_jql(self) -> str:
                return "project = TARS"

            def find_existing_import(self, sha: str) -> None:
                return None

            def create_issue(self, payload: object) -> None:
                self.create_calls += 1
                raise AssertionError("dry-run attempted issue creation")

            def transition_to_done(self, issue_key: str) -> None:
                self.transition_calls += 1
                raise AssertionError("dry-run attempted transition")

        config = ImportConfig(
            jira_url="https://example.atlassian.net",
            jira_project_key="TARS",
            jira_board_id=3,
            github_repository="owner/repository",
            github_branch="master",
            credentials=Credentials(None, None, "fake", "fake"),
            netrc_file=Path("fake.netrc"),
            credential_source="test fixture",
            credential_warnings=(),
            apply=False,
            since=None,
            until=None,
            author=None,
            max_commits=1,
            include_merges=False,
            include_insignificant=False,
            apply_manual_review=False,
            output_report=Path("build/test"),
            verbose=False,
        )
        fake_jira = FakeJira()
        records, summary = run_import(config, github_client=FakeGitHub(), jira_client=fake_jira)
        self.assertEqual(summary.jira_created, 0)
        self.assertEqual(records[0].result, "skipped")
        self.assertEqual(fake_jira.create_calls, 0)
        self.assertEqual(fake_jira.transition_calls, 0)

        unauthenticated = replace(
            config,
            credentials=Credentials(None, None, None, None),
            credential_source="none",
        )
        offline_records, offline_summary = run_import(
            unauthenticated,
            github_client=FakeGitHub(),
        )
        self.assertEqual(offline_records[0].result, "skipped")
        self.assertTrue(any("Jira credentials are unavailable" in value for value in offline_summary.limitations))

    def test_apply_keeps_created_issue_in_todo_backlog(self) -> None:
        """Apply mode verifies To Do and backlog placement without transitioning the created issue."""
        commit = make_commit(
            "Enhance Jira importer planning",
            [
                ChangedFile(
                    "xWalkTool/py-agent/board-tool/py-src/xWalkJiraImport/importer.py",
                    "modified",
                    10,
                    2,
                    12,
                    patch="+planning",
                )
            ],
        )

        class FakeGitHub:
            def collect_commits(self, *args: object, **kwargs: object) -> list[object]:
                return [commit]

        class FakeJira:
            transition_calls = 0

            def discover_metadata(self) -> JiraMetadata:
                fields = {"labels": JiraField("labels", "Labels", "array", "", False)}
                return JiraMetadata({"Task": "1"}, {"1": fields}, fields)

            def get_board_filter_jql(self) -> str:
                return "project = TARS"

            def find_existing_import(self, sha: str) -> None:
                return None

            def create_issue(self, payload: object) -> tuple[str, str]:
                return "TARS-23", "https://example.atlassian.net/browse/TARS-23"

            def get_status(self, issue_key: str) -> tuple[str, str, str]:
                return "10000", "To Do", "new"

            def issue_matches_board(self, issue_key: str) -> bool:
                return True

            def issue_matches_backlog(self, issue_key: str) -> bool:
                return True

            def transition_to_done(self, issue_key: str) -> None:
                self.transition_calls += 1
                raise AssertionError("created issue was transitioned out of To Do")

        config = ImportConfig(
            jira_url="https://example.atlassian.net",
            jira_project_key="TARS",
            jira_board_id=3,
            github_repository="owner/repository",
            github_branch="master",
            credentials=Credentials(None, None, "fake", "fake"),
            netrc_file=Path("fake.netrc"),
            credential_source="test fixture",
            credential_warnings=(),
            apply=True,
            since=None,
            until=None,
            author=None,
            max_commits=1,
            include_merges=False,
            include_insignificant=False,
            apply_manual_review=False,
            output_report=Path("build/test"),
            verbose=False,
        )
        fake_jira = FakeJira()
        records, summary = run_import(config, github_client=FakeGitHub(), jira_client=fake_jira)
        self.assertEqual(summary.jira_created, 1)
        self.assertEqual(summary.created_todo, 1)
        self.assertEqual(summary.failures, 0)
        self.assertEqual(records[0].final_jira_status, "To Do")
        self.assertEqual(records[0].result, "created")
        self.assertEqual(fake_jira.transition_calls, 0)

    def test_apply_updates_estimate_on_exact_existing_import(self) -> None:
        """The explicit update option changes an existing import without creating another issue."""
        commit = make_commit(
            "Stabilize host quality gates",
            [
                ChangedFile("xWalkTool/shell-agent/env-tool/quality/gcovr.cfg", "modified", 4, 3, 7, patch="+coverage"),
                ChangedFile("xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh", "modified", 2, 2, 4, patch="+export"),
            ],
        )

        class FakeGitHub:
            def collect_commits(self, *args: object, **kwargs: object) -> list[object]:
                return [commit]

        class FakeJira:
            updated: list[tuple[str, str]]

            def __init__(self) -> None:
                self.updated = []

            def discover_metadata(self) -> JiraMetadata:
                fields = {"timetracking": JiraField("timetracking", "Time tracking", "", "", False)}
                return JiraMetadata({"Task": "1"}, {"1": fields}, fields)

            def get_board_filter_jql(self) -> str:
                return "project = TARS"

            def find_existing_import(self, sha: str) -> tuple[str, str]:
                return "TARS-20", "Done"

            def update_original_estimate(self, issue_key: str, jira_time: str) -> None:
                self.updated.append((issue_key, jira_time))

        config = ImportConfig(
            jira_url="https://example.atlassian.net",
            jira_project_key="TARS",
            jira_board_id=3,
            github_repository="owner/repository",
            github_branch="master",
            credentials=Credentials(None, None, "fake", "fake"),
            netrc_file=Path("fake.netrc"),
            credential_source="test fixture",
            credential_warnings=(),
            apply=True,
            since=None,
            until=None,
            author=None,
            max_commits=1,
            include_merges=False,
            include_insignificant=False,
            apply_manual_review=False,
            output_report=Path("build/test"),
            verbose=False,
            update_existing_estimates=True,
        )
        fake_jira = FakeJira()
        records, summary = run_import(config, github_client=FakeGitHub(), jira_client=fake_jira)
        self.assertEqual(fake_jira.updated, [("TARS-20", "2h")])
        self.assertEqual(summary.estimates_updated, 1)
        self.assertEqual(summary.jira_created, 0)
        self.assertEqual(records[0].result, "updated")

    def test_manual_review_override_requires_apply_mode(self) -> None:
        """Broad-commit creation cannot be enabled without the explicit write mode."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(SystemExit):
                load_config(
                    ["--dry-run", "--apply-manual-review", "--netrc-file", str(netrc_path)],
                    {},
                )

    def test_existing_estimate_update_requires_apply_mode(self) -> None:
        """Existing Jira estimates cannot be changed from preview mode."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(SystemExit):
                load_config(
                    ["--dry-run", "--update-existing-estimates", "--netrc-file", str(netrc_path)],
                    {},
                )

    def test_existing_planning_update_requires_apply_mode(self) -> None:
        """Sequential planning cannot mutate Jira from preview mode."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(SystemExit):
                load_config(
                    [
                        "--dry-run",
                        "--update-existing-planning-after",
                        "TARS-15",
                        "--netrc-file",
                        str(netrc_path),
                    ],
                    {},
                )

    def test_explicit_existing_planning_start_requires_apply_mode(self) -> None:
        """An explicit first Start date cannot mutate Jira from preview mode."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(SystemExit):
                load_config(
                    [
                        "--dry-run",
                        "--update-existing-planning-start",
                        "2026-07-23",
                        "--netrc-file",
                        str(netrc_path),
                    ],
                    {},
                )

    def test_existing_sprint_update_requires_apply_mode(self) -> None:
        """Sprint assignment cannot mutate Jira from preview mode."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(SystemExit):
                load_config(
                    ["--dry-run", "--update-existing-sprints", "--netrc-file", str(netrc_path)],
                    {},
                )

    def test_existing_epic_update_requires_apply_mode(self) -> None:
        """Epic-parent attachment cannot mutate Jira from preview mode."""
        with TemporaryDirectory() as directory:
            netrc_path = Path(directory) / "missing.netrc"
            with self.assertRaises(SystemExit):
                load_config(
                    ["--dry-run", "--update-existing-epic", "TARS-15", "--netrc-file", str(netrc_path)],
                    {},
                )


if __name__ == "__main__":
    unittest.main()
