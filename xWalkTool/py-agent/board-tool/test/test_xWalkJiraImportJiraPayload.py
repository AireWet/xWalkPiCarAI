#!/usr/bin/env python3
"""Test xWalk Jira ADF payloads, dynamic fields, failures, and retries."""

from __future__ import annotations

import unittest
from datetime import timedelta
from unittest import mock

import requests

from test_xWalkJiraImportCommitAnalyser import make_commit
from xWalkJiraImport.xWalkJiraImportCommitAnalyser import analyse_commit
from xWalkJiraImport.xWalkJiraImportDateCalculator import calculate_historical_dates
from xWalkJiraImport.xWalkJiraImportEstimator import estimate_effort
from xWalkJiraImport.xWalkJiraImportGitHubClient import ApiError, request_json
from xWalkJiraImport.xWalkJiraImportJiraClient import (
    IMPORT_MARKER_PREFIX,
    JiraClient,
    build_description_adf,
    build_issue_payload,
    flatten_adf_text,
)
from xWalkJiraImport.xWalkJiraImportModels import ChangedFile, JiraField


class FakeResponse:
    """Provide the requests.Response surface used by the clients."""

    def __init__(self, status: int, payload: object | None = None, headers: dict[str, str] | None = None) -> None:
        """Store one deterministic status, body, and header set."""
        self.status_code = status
        self.payload = payload
        self.headers = headers or {}
        self.content = b"" if status == 204 else b"json"

    def json(self) -> object:
        """Return the configured JSON payload."""
        return self.payload


class QueueSession:
    """Return queued responses while recording sanitized request arguments."""

    def __init__(self, responses: list[FakeResponse]) -> None:
        """Create a session with mutable headers and no real transport."""
        self.responses = list(responses)
        self.headers: dict[str, str] = {}
        self.auth: tuple[str, str] | None = None
        self.requests: list[tuple[str, str, dict[str, object]]] = []

    def request(self, method: str, url: str, **kwargs: object) -> FakeResponse:
        """Record one request and return the next response."""
        self.requests.append((method, url, kwargs))
        if not self.responses:
            raise AssertionError("No fake response remains")
        return self.responses.pop(0)


def payload_fixture() -> tuple[object, object, object, object]:
    """Create commit, analysis, estimate, and date fixtures."""
    commit = make_commit(
        "Fix USB camera reconnection failure",
        [
            ChangedFile("src/camera/usb_camera.cpp", "modified", 20, 5, 25, patch="+retry"),
            ChangedFile("test/test_camera.cpp", "added", 30, 0, 30, patch="+test"),
        ],
        body="Add a reconnection test and verify recovery.",
    )
    analysis = analyse_commit(commit)
    effort = estimate_effort(commit, analysis)
    dates = calculate_historical_dates(commit, effort)
    return commit, analysis, effort, dates


class JiraPayloadTest(unittest.TestCase):
    """Verify ADF validity and supported-field-only issue creation."""

    def test_adf_contains_link_notice_testing_and_marker(self) -> None:
        """The description preserves all required historical evidence."""
        commit, analysis, effort, dates = payload_fixture()
        adf = build_description_adf(commit, analysis, effort, dates)
        flattened = flatten_adf_text(adf)
        self.assertEqual(adf["type"], "doc")
        self.assertIn(commit.url, flattened)
        self.assertIn("Historical record:", flattened)
        self.assertIn(f"{IMPORT_MARKER_PREFIX}{commit.sha}", flattened)
        self.assertIn("Validation and testing", flattened)

    def test_adf_uses_detailed_sections_paragraphs_and_bullets(self) -> None:
        """The Jira description is readable and explicitly estimates human work."""
        commit, analysis, effort, dates = payload_fixture()
        adf = build_description_adf(commit, analysis, effort, dates)
        headings = [node for node in adf["content"] if node["type"] == "heading"]
        paragraphs = [node for node in adf["content"] if node["type"] == "paragraph"]
        bullet_lists = [node for node in adf["content"] if node["type"] == "bulletList"]
        heading_text = {flatten_adf_text(node) for node in headings}
        self.assertIn("Overview", heading_text)
        self.assertIn("Completed work", heading_text)
        self.assertIn("Human development estimate", heading_text)
        self.assertIn("Git traceability", heading_text)
        self.assertGreaterEqual(len(paragraphs), 8)
        self.assertGreaterEqual(len(bullet_lists), 5)
        self.assertTrue(all(node["content"] for node in bullet_lists))
        self.assertIn("expected effort for a human developer", flatten_adf_text(adf))
        self.assertIn("not based on AI generation speed", flatten_adf_text(adf))

    def test_payload_uses_dynamically_discovered_fields(self) -> None:
        """Story points and historical dates use supplied metadata IDs only."""
        commit, analysis, effort, dates = payload_fixture()
        fields = {
            "labels": JiraField("labels", "Labels", "array", "", False),
            "timetracking": JiraField("timetracking", "Time tracking", "timetracking", "", False),
            "duedate": JiraField("duedate", "Due date", "date", "", False),
            "customfield_10020": JiraField("customfield_10020", "Story Points", "number", "", False),
            "customfield_10021": JiraField("customfield_10021", "Start date", "date", "", False),
            "customfield_10022": JiraField(
                "customfield_10022", "Historical done date", "datetime", "", False
            ),
        }
        payload = build_issue_payload("TARS", "Bug", "10001", fields, commit, analysis, effort, dates)
        values = payload["fields"]
        self.assertEqual(values["customfield_10020"], effort.story_points)
        self.assertEqual(values["customfield_10021"], dates.start.date().isoformat())
        self.assertEqual(values["customfield_10022"], dates.completion.isoformat())
        self.assertEqual(values["timetracking"]["originalEstimate"], effort.jira_time)
        self.assertNotIn("customfield_99999", values)

    def test_discovers_issue_types_and_field_ids_with_pagination_shape(self) -> None:
        """Modern create metadata is decoded without hard-coded custom IDs."""
        session = QueueSession(
            [
                FakeResponse(
                    200,
                    [
                        {
                            "id": "customfield_10020",
                            "name": "Story Points",
                            "schema": {"type": "number", "custom": "story-points"},
                        }
                    ],
                ),
                FakeResponse(
                    200,
                    {"issueTypes": [{"id": "10001", "name": "Task", "subtask": False}], "total": 1},
                ),
                FakeResponse(
                    200,
                    {
                        "fields": [
                            {
                                "fieldId": "customfield_10020",
                                "name": "Story Points",
                                "required": False,
                                "schema": {"type": "number", "custom": "story-points"},
                            }
                        ],
                        "total": 1,
                    },
                ),
            ]
        )
        client = JiraClient("https://example.atlassian.net", "fake", "fake", "TARS", 3, session=session)
        metadata = client.discover_metadata()
        self.assertEqual(metadata.issue_types, {"Task": "10001"})
        field = metadata.fields_by_issue_type["10001"]["customfield_10020"]
        self.assertEqual(field.name, "Story Points")
        self.assertEqual(field.schema_type, "number")

    def test_retries_server_failure_without_exposing_request_data(self) -> None:
        """A retryable failure uses bounded retry and then returns JSON."""
        session = QueueSession([FakeResponse(500), FakeResponse(200, {"ok": True})])
        with mock.patch("time.sleep") as unused_sleep:
            result = request_json(session, "GET", "https://api.example.invalid/value", sleep=unused_sleep)
        self.assertEqual(result, {"ok": True})
        unused_sleep.assert_called_once()

    def test_does_not_retry_non_retryable_failure(self) -> None:
        """A client error fails immediately with a sanitized status message."""
        session = QueueSession([FakeResponse(400, {"token": "must-not-appear"})])
        with self.assertRaisesRegex(ApiError, "HTTP 400") as raised:
            request_json(session, "GET", "https://api.example.invalid/value", sleep=lambda _: None)
        self.assertNotIn("must-not-appear", str(raised.exception))

    def test_authentication_errors_explain_status_without_response_content(self) -> None:
        """Invalid, expired, and forbidden tokens produce actionable redacted errors."""
        unauthorized = QueueSession([FakeResponse(401, {"token": "secret-response"})])
        with self.assertRaisesRegex(ApiError, "invalid, expired, or revoked") as raised:
            request_json(unauthorized, "GET", "https://api.example.invalid/value")
        self.assertNotIn("secret-response", str(raised.exception))

        forbidden = QueueSession([FakeResponse(403, {"password": "secret-response"})])
        with self.assertRaisesRegex(ApiError, "permission was denied") as raised:
            request_json(forbidden, "GET", "https://api.example.invalid/value")
        self.assertNotIn("secret-response", str(raised.exception))

    def test_github_rate_limit_has_a_specific_sanitized_error(self) -> None:
        """A depleted GitHub rate limit explains token and waiting options."""
        session = QueueSession(
            [FakeResponse(403, {"token": "secret-response"}, {"X-RateLimit-Remaining": "0"})]
        )
        with self.assertRaisesRegex(ApiError, "GitHub API rate limit exceeded") as raised:
            request_json(session, "GET", "https://api.github.com/value")
        self.assertNotIn("secret-response", str(raised.exception))

    def test_retries_transport_failure(self) -> None:
        """A transient requests exception is retried without echoing its details."""
        session = mock.Mock()
        session.request.side_effect = [requests.ConnectionError("secret query"), FakeResponse(200, {"ok": 1})]
        result = request_json(session, "GET", "https://api.example.invalid", sleep=lambda _: None)
        self.assertEqual(result, {"ok": 1})


if __name__ == "__main__":
    unittest.main()
