"""Read xWalk commit evidence from the public GitHub REST API."""

from __future__ import annotations

import time
from datetime import date, datetime, time as clock_time, timezone
from email.utils import parsedate_to_datetime
from typing import Any, Callable, Mapping
from urllib.parse import urlparse
from zoneinfo import ZoneInfo

import requests

from .xWalkJiraImportModels import ChangedFile, CommitRecord


class ApiError(Exception):
    """Report a sanitized remote API failure."""


def _http_error_message(response: requests.Response) -> str:
    """Describe authentication and rate failures without returning response data."""
    status = response.status_code
    rate_remaining = response.headers.get("X-RateLimit-Remaining", "")
    if status == 401:
        return "Remote API returned HTTP 401; the API token is invalid, expired, or revoked."
    if status == 403 and rate_remaining == "0":
        return "GitHub API rate limit exceeded; configure a GitHub token or wait for the limit to reset."
    if status == 403:
        return "Remote API returned HTTP 403; authentication succeeded but permission was denied."
    if status == 429:
        return "Remote API rate limit exceeded (HTTP 429); retry after the server's wait period."
    return f"Remote API returned HTTP {status}."


def _retry_delay(response: requests.Response, attempt: int) -> float:
    """Return a bounded Retry-After or exponential delay."""
    value = response.headers.get("Retry-After", "").strip()
    if value.isdigit():
        return min(float(value), 30.0)
    if value:
        try:
            target = parsedate_to_datetime(value)
            seconds = (target - datetime.now(timezone.utc)).total_seconds()
            return min(max(seconds, 0.0), 30.0)
        except (TypeError, ValueError, OverflowError):
            pass
    return min(2.0**attempt, 8.0)


def request_json(
    session: requests.Session,
    method: str,
    url: str,
    *,
    attempts: int = 4,
    timeout: float = 30.0,
    sleep: Callable[[float], None] = time.sleep,
    expected: tuple[int, ...] = (200,),
    **kwargs: Any,
) -> Any:
    """Perform one JSON request with bounded retries and safe diagnostics."""
    response: requests.Response | None = None
    for attempt in range(attempts):
        try:
            response = session.request(method, url, timeout=timeout, **kwargs)
        except requests.RequestException as error:
            if attempt + 1 >= attempts:
                raise ApiError(f"Remote API request failed after {attempts} attempts.") from error
            sleep(min(2.0**attempt, 8.0))
            continue
        if response.status_code in expected:
            if response.status_code == 204 or not response.content:
                return None
            try:
                return response.json()
            except ValueError as error:
                raise ApiError("Remote API returned malformed JSON.") from error
        retryable = response.status_code == 429 or 500 <= response.status_code < 600
        if retryable and attempt + 1 < attempts:
            sleep(_retry_delay(response, attempt))
            continue
        raise ApiError(_http_error_message(response))
    raise ApiError("Remote API request failed without a response.")


def _parse_datetime(value: object) -> datetime | None:
    """Parse one GitHub ISO timestamp without accepting naive dates."""
    if not isinstance(value, str) or not value.strip():
        return None
    try:
        parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError:
        return None
    return parsed if parsed.tzinfo is not None else None


def _github_boundary(value: date, end_of_day: bool) -> str:
    """Convert one Stockholm calendar boundary to GitHub's UTC timestamp."""
    local_time = clock_time.max if end_of_day else clock_time.min
    local = datetime.combine(value, local_time, ZoneInfo("Europe/Stockholm"))
    return local.astimezone(timezone.utc).isoformat().replace("+00:00", "Z")


class GitHubClient:
    """Retrieve branch commits and every paginated changed-file record."""

    def __init__(
        self,
        repository: str,
        token: str | None = None,
        session: requests.Session | None = None,
        api_url: str = "https://api.github.com",
    ) -> None:
        """Configure one read-only client without requiring a public-repository token."""
        self.repository = repository
        self.api_url = api_url.rstrip("/")
        self.session = session or requests.Session()
        self.session.trust_env = False
        self.session.headers.update(
            {
                "Accept": "application/vnd.github+json",
                "X-GitHub-Api-Version": "2022-11-28",
                "User-Agent": "xwalk-thesis-jira-importer",
            }
        )
        api_host = urlparse(self.api_url).hostname
        if token and api_host == "api.github.com":
            self.session.headers["Authorization"] = f"Bearer {token}"

    def collect_commits(
        self,
        branch: str,
        since: date | None = None,
        until: date | None = None,
        author: str | None = None,
        max_commits: int | None = None,
        include_merges: bool = False,
    ) -> list[CommitRecord]:
        """Collect selected commits oldest-first using summary and changed-file pagination."""
        summaries = self._list_commit_summaries(branch, since, until)
        selected: list[Mapping[str, Any]] = []
        for summary in summaries:
            parents = summary.get("parents", [])
            if not include_merges and isinstance(parents, list) and len(parents) > 1:
                continue
            if author and not self._matches_author(summary, author):
                continue
            selected.append(summary)
        selected.reverse()
        if max_commits is not None:
            selected = selected[:max_commits]
        return [self.get_commit(str(summary["sha"])) for summary in selected]

    def _list_commit_summaries(
        self,
        branch: str,
        since: date | None,
        until: date | None,
    ) -> list[Mapping[str, Any]]:
        """Retrieve every summary page from one branch."""
        url = f"{self.api_url}/repos/{self.repository}/commits"
        page = 1
        results: list[Mapping[str, Any]] = []
        while True:
            parameters: dict[str, object] = {"sha": branch, "per_page": 100, "page": page}
            if since is not None:
                parameters["since"] = _github_boundary(since, False)
            if until is not None:
                parameters["until"] = _github_boundary(until, True)
            payload = request_json(self.session, "GET", url, params=parameters)
            if not isinstance(payload, list):
                raise ApiError("GitHub commit listing returned an unexpected response.")
            results.extend(item for item in payload if isinstance(item, dict))
            if len(payload) < 100:
                break
            page += 1
        return results

    @staticmethod
    def _matches_author(summary: Mapping[str, Any], requested: str) -> bool:
        """Match Git author name, email, or GitHub login case-insensitively."""
        needle = requested.strip().casefold()
        commit = summary.get("commit", {})
        git_author = commit.get("author", {}) if isinstance(commit, dict) else {}
        github_author = summary.get("author", {})
        candidates = {
            str(git_author.get("name", "")).casefold(),
            str(git_author.get("email", "")).casefold(),
            str(github_author.get("login", "")).casefold() if isinstance(github_author, dict) else "",
        }
        return needle in candidates

    def get_commit(self, sha: str) -> CommitRecord:
        """Retrieve complete commit details and all changed-file pages."""
        url = f"{self.api_url}/repos/{self.repository}/commits/{sha}"
        page = 1
        raw_commit: Mapping[str, Any] | None = None
        raw_files: list[Mapping[str, Any]] = []
        while True:
            payload = request_json(
                self.session,
                "GET",
                url,
                params={"per_page": 100, "page": page},
            )
            if not isinstance(payload, dict):
                raise ApiError("GitHub commit detail returned an unexpected response.")
            if raw_commit is None:
                raw_commit = payload
            page_files = payload.get("files", [])
            if not isinstance(page_files, list):
                raise ApiError("GitHub commit files returned an unexpected response.")
            raw_files.extend(item for item in page_files if isinstance(item, dict))
            if len(page_files) < 100:
                break
            page += 1
        return self._decode_commit(raw_commit, raw_files)

    @staticmethod
    def _decode_commit(
        payload: Mapping[str, Any],
        raw_files: list[Mapping[str, Any]],
    ) -> CommitRecord:
        """Convert GitHub JSON into a stable internal record."""
        sha = str(payload.get("sha", ""))
        if len(sha) != 40:
            raise ApiError("GitHub returned an invalid full commit SHA.")
        commit = payload.get("commit", {})
        if not isinstance(commit, dict):
            raise ApiError("GitHub omitted commit metadata.")
        message = str(commit.get("message", ""))
        title, _, body = message.partition("\n")
        author = commit.get("author", {})
        committer = commit.get("committer", {})
        author = author if isinstance(author, dict) else {}
        committer = committer if isinstance(committer, dict) else {}
        files = tuple(
            ChangedFile(
                filename=str(item.get("filename", "")),
                status=str(item.get("status", "")),
                additions=int(item.get("additions", 0)),
                deletions=int(item.get("deletions", 0)),
                changes=int(item.get("changes", 0)),
                previous_filename=(
                    str(item["previous_filename"]) if item.get("previous_filename") else None
                ),
                patch=str(item["patch"]) if item.get("patch") is not None else None,
            )
            for item in raw_files
            if item.get("filename")
        )
        stats = payload.get("stats", {})
        stats = stats if isinstance(stats, dict) else {}
        parents = payload.get("parents", [])
        parent_shas = tuple(
            str(item["sha"])
            for item in parents
            if isinstance(item, dict) and item.get("sha")
        )
        return CommitRecord(
            sha=sha,
            short_sha=sha[:7],
            title=title.strip(),
            body=body.strip(),
            author_name=str(author.get("name", "Unknown")),
            author_email=str(author.get("email", "")),
            committer_name=str(committer.get("name", "Unknown")),
            committer_email=str(committer.get("email", "")),
            author_date=_parse_datetime(author.get("date")),
            commit_date=_parse_datetime(committer.get("date")),
            url=str(payload.get("html_url", "")),
            parents=parent_shas,
            files=files,
            additions=int(stats.get("additions", sum(item.additions for item in files))),
            deletions=int(stats.get("deletions", sum(item.deletions for item in files))),
            total_changes=int(stats.get("total", sum(item.changes for item in files))),
        )
