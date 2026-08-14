"""Load xWalk GitHub and Jira credentials without exposing secret values."""

from __future__ import annotations

import netrc
import os
from dataclasses import dataclass
from pathlib import Path
from typing import Mapping


class CredentialError(Exception):
    """Report a non-sensitive credential configuration failure."""


class MissingNetrcError(CredentialError):
    """Report that the selected netrc file does not exist."""


class MalformedNetrcError(CredentialError):
    """Report that the selected netrc file cannot be parsed safely."""


class UnsafeNetrcPermissionsError(CredentialError):
    """Report group or other access to a POSIX netrc file."""


@dataclass(frozen=True)
class Credentials:
    """Contain credentials loaded from netrc or the CI environment fallback."""

    github_login: str | None
    github_token: str | None
    jira_email: str | None
    jira_api_token: str | None

    @property
    def has_jira_credentials(self) -> bool:
        """Return whether Jira Basic authentication can be configured."""
        return bool(self.jira_email and self.jira_api_token)


@dataclass(frozen=True)
class CredentialResolution:
    """Contain resolved credentials plus safe user-facing diagnostics."""

    credentials: Credentials
    source: str
    warnings: tuple[str, ...]


EMPTY_CREDENTIALS = Credentials(None, None, None, None)
GITHUB_UNAUTHENTICATED_WARNING = (
    "GitHub authentication is not configured; unauthenticated requests have a lower rate limit."
)


def resolve_netrc_path(value: Path | None) -> Path:
    """Resolve the selected path without reading it or relying on a shell variable."""
    selected = value if value is not None else Path.home() / ".netrc"
    text = str(selected)
    if text == "~":
        selected = Path.home()
    elif text.startswith("~/"):
        selected = Path.home() / text[2:]
    else:
        selected = selected.expanduser()
    return selected.resolve(strict=False)


def validate_netrc_permissions(netrc_path: Path) -> None:
    """Reject netrc files accessible to group or other users on POSIX."""
    if os.name == "nt":
        return
    try:
        permissions = netrc_path.stat().st_mode & 0o777
    except FileNotFoundError as error:
        raise MissingNetrcError("The selected .netrc file does not exist.") from error
    except OSError as error:
        raise CredentialError("The selected .netrc file cannot be inspected.") from error
    if permissions & 0o077:
        raise UnsafeNetrcPermissionsError(
            "The selected .netrc has unsafe permissions; run 'chmod 600 ~/.netrc' and retry."
        )


def _machine_credentials(parsed: netrc.netrc, host: str) -> tuple[str | None, str | None]:
    """Return normalized login and password values for one exact machine name."""
    authentication = parsed.hosts.get(host)
    if authentication is None:
        return None, None
    login, _, password = authentication
    normalized_login = login.strip() if login and login.strip() else None
    normalized_password = password.strip() if password and password.strip() else None
    return normalized_login, normalized_password


def load_credentials(netrc_path: Path, github_host: str, jira_host: str) -> Credentials:
    """Load exact GitHub and derived Jira machine entries from one safe netrc file."""
    validate_netrc_permissions(netrc_path)
    try:
        parsed = netrc.netrc(str(netrc_path))
    except FileNotFoundError as error:
        raise MissingNetrcError("The selected .netrc file does not exist.") from error
    except (netrc.NetrcParseError, UnicodeError) as error:
        raise MalformedNetrcError("The selected .netrc file is malformed.") from error
    except OSError as error:
        raise CredentialError("The selected .netrc file cannot be read.") from error
    github_login, github_token = _machine_credentials(parsed, github_host)
    jira_email, jira_api_token = _machine_credentials(parsed, jira_host)
    return Credentials(github_login, github_token, jira_email, jira_api_token)


def _environment_credentials(environment: Mapping[str, str]) -> Credentials:
    """Load the optional secret environment fallback used by CI."""
    return Credentials(
        None,
        environment.get("GITHUB_TOKEN") or None,
        environment.get("JIRA_EMAIL") or None,
        environment.get("JIRA_API_TOKEN") or None,
    )


def _merge_credentials(primary: Credentials, fallback: Credentials) -> tuple[Credentials, bool]:
    """Fill only missing netrc values from the CI environment fallback."""
    fallback_used = any(
        primary_value is None and fallback_value is not None
        for primary_value, fallback_value in (
            (primary.github_token, fallback.github_token),
            (primary.jira_email, fallback.jira_email),
            (primary.jira_api_token, fallback.jira_api_token),
        )
    )
    return (
        Credentials(
            primary.github_login,
            primary.github_token or fallback.github_token,
            primary.jira_email or fallback.jira_email,
            primary.jira_api_token or fallback.jira_api_token,
        ),
        fallback_used,
    )


def resolve_credentials(
    netrc_path: Path,
    github_host: str,
    jira_host: str,
    environment: Mapping[str, str],
    apply: bool,
) -> CredentialResolution:
    """Apply netrc precedence, permission safety, and the CI environment fallback."""
    warnings: list[str] = []
    netrc_credentials = EMPTY_CREDENTIALS
    netrc_available = False
    try:
        netrc_credentials = load_credentials(netrc_path, github_host, jira_host)
        netrc_available = any(
            (
                netrc_credentials.github_login,
                netrc_credentials.github_token,
                netrc_credentials.jira_email,
                netrc_credentials.jira_api_token,
            )
        )
    except UnsafeNetrcPermissionsError:
        message = "Unsafe .netrc permissions; run 'chmod 600 ~/.netrc'."
        if apply:
            raise CredentialError(f"{message} Jira apply mode was blocked.") from None
        warnings.append(f"{message} Authenticated requests are disabled for this dry run.")
        warnings.append(GITHUB_UNAUTHENTICATED_WARNING)
        return CredentialResolution(EMPTY_CREDENTIALS, "none", tuple(warnings))
    except MissingNetrcError as error:
        warnings.append(str(error))
    except MalformedNetrcError as error:
        if apply:
            raise CredentialError(f"{error} Jira apply mode was blocked.") from None
        warnings.append(str(error))

    environment_credentials = _environment_credentials(environment)
    credentials, fallback_used = _merge_credentials(netrc_credentials, environment_credentials)
    if netrc_available and fallback_used:
        source = "netrc + environment fallback"
    elif netrc_available:
        source = "netrc"
    elif fallback_used:
        source = "environment fallback"
    else:
        source = "none"

    if credentials.github_token is None:
        warnings.append(GITHUB_UNAUTHENTICATED_WARNING)
    if apply and not credentials.has_jira_credentials:
        if credentials.jira_email is None and credentials.jira_api_token is None:
            detail = f"missing Jira machine entry for '{jira_host}' or its login and API token"
        elif credentials.jira_email is None:
            detail = f"Jira machine entry for '{jira_host}' is missing its login"
        else:
            detail = f"Jira machine entry for '{jira_host}' is missing its API token password"
        raise CredentialError(f"--apply requires Jira credentials: {detail}.")
    return CredentialResolution(credentials, source, tuple(warnings))


def mask_email(value: str) -> str:
    """Mask an email address without revealing the full Jira login."""
    local, separator, domain = value.partition("@")
    if not separator:
        return f"{local[:1]}***" if local else "***"
    return f"{local[:1]}***@{domain}"
