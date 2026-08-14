#!/usr/bin/env python3
"""Assess and install a non-root Gerrit server on validated user-owned storage."""

from __future__ import annotations

import argparse
import datetime
import fcntl
import hashlib
import ipaddress
import json
import os
import pathlib
import re
import shutil
import socket
import stat
import subprocess
import sys
import tarfile
import tempfile
import urllib.request
import urllib.parse


GERRIT_VERSION = "3.14.2"
GERRIT_URL = (
    "https://gerrit-releases.storage.googleapis.com/"
    f"gerrit-{GERRIT_VERSION}.war"
)
CADDY_VERSION = "2.11.3"
CADDY_ARTIFACTS = {
    "x86_64": (
        f"https://github.com/caddyserver/caddy/releases/download/v{CADDY_VERSION}/"
        f"caddy_{CADDY_VERSION}_linux_amd64.tar.gz",
        "3894577b14657feab3624d782f64175050211e52a228a6f57b4f24f4b0d970f3",
    ),
    "aarch64": (
        f"https://github.com/caddyserver/caddy/releases/download/v{CADDY_VERSION}/"
        f"caddy_{CADDY_VERSION}_linux_arm64.tar.gz",
        "866d2b226a4c07adc38aea7df653ee63b98890691d4cd41db1f1c83e18300bbe",
    ),
}
REJECTED_INTERFACES = ("docker", "virbr", "vbox", "vmnet")
ASSESSMENT_PROGRAMS = (
    "git", "ssh", "ssh-keygen", "ssh-keyscan", "curl", "wget", "tar",
    "unzip", "openssl", "keytool", "readlink", "findmnt", "stat", "ss", "nc",
    "nohup", "screen", "tmux",
)
REQUIRED_PROGRAMS = (
    "awk", "df", "du", "findmnt", "free", "git", "ip", "openssl", "ps", "readlink",
    "sed", "sha256sum", "ssh", "ssh-keygen", "ssh-keyscan", "ss", "stat", "tar", "curl",
)
MINIMUM_FREE_BYTES = 10 * 1024**3
UNSAFE_STORAGE_PATHS = (
    pathlib.Path("/"), pathlib.Path("/home"), pathlib.Path("/mnt"),
    pathlib.Path("/media"), pathlib.Path("/opt"), pathlib.Path("/srv"),
    pathlib.Path("/tmp"), pathlib.Path("/usr"), pathlib.Path("/var"),
)


class SetupError(RuntimeError):
    """Raised when a safe installation prerequisite is not satisfied."""


def run(
    arguments: list[str],
    *,
    check: bool = True,
    environment: dict[str, str] | None = None,
    input_text: str | None = None,
) -> subprocess.CompletedProcess[str]:
    """Run one command without a shell and return its captured result."""

    result = subprocess.run(
        arguments,
        check=False,
        env=environment,
        input=input_text,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    if check and result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip()
        raise SetupError(f"Command failed ({' '.join(arguments)}): {detail}")
    return result


def command_path(name: str) -> str | None:
    """Return an executable path without changing the host."""

    return shutil.which(name)


def sha256(path: pathlib.Path) -> str:
    """Calculate the SHA-256 digest of one regular file."""

    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def java_major(java: str) -> int | None:
    """Return the detected Java major version or None for unrecognised output."""

    result = run([java, "-version"], check=False)
    text = result.stderr + result.stdout
    marker = 'version "'
    if marker not in text:
        return None
    version = text.split(marker, 1)[1].split('"', 1)[0]
    first = version.split(".", 1)[0]
    return int(first) if first.isdigit() else None


def network_addresses() -> list[dict[str, object]]:
    """Read assigned IPv4 addresses using machine-readable iproute output."""

    result = run(["ip", "-j", "-4", "address", "show"])
    addresses: list[dict[str, object]] = []
    for interface in json.loads(result.stdout):
        for address in interface.get("addr_info", []):
            if address.get("family") == "inet":
                addresses.append(
                    {
                        "interface": interface["ifname"],
                        "address": address["local"],
                        "scope": address.get("scope", "unknown"),
                    }
                )
    return addresses


def validate_server_ip(value: str, addresses: list[dict[str, object]]) -> str:
    """Require a concrete, assigned, non-loopback server address."""

    try:
        selected = ipaddress.ip_address(value)
    except ValueError as error:
        raise SetupError(f"Invalid --server-ip: {value}") from error
    if selected.version != 4 or selected.is_loopback or selected.is_link_local or selected.is_unspecified:
        raise SetupError("--server-ip must be an assigned, non-loopback IPv4 address")
    match = next((item for item in addresses if item["address"] == value), None)
    if match is None:
        raise SetupError(f"--server-ip {value} is not assigned on this server")
    interface = str(match["interface"])
    if interface.startswith(REJECTED_INTERFACES):
        raise SetupError(f"--server-ip belongs to excluded interface {interface}")
    return interface


def port_is_free(address: str, port: int) -> bool:
    """Check a specific local address by attempting a non-listening bind."""

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 0)
        try:
            probe.bind((address, port))
        except OSError:
            return False
    return True


def persistent_storage(home: pathlib.Path) -> tuple[bool, str]:
    """Classify obvious temporary filesystems while retaining mount evidence."""

    result = run(["findmnt", "-J", "-T", str(home), "-o", "TARGET,SOURCE,FSTYPE,OPTIONS"])
    filesystems = json.loads(result.stdout).get("filesystems", [])
    if not filesystems:
        return False, "No mount information returned"
    filesystem = filesystems[0]
    filesystem_type = filesystem.get("fstype", "unknown")
    persistent = filesystem_type not in {"tmpfs", "ramfs", "overlay"}
    return persistent, json.dumps(filesystem, sort_keys=True)


def storage_mount(path: pathlib.Path) -> dict[str, object]:
    """Return the mounted filesystem containing one validated parent path."""

    result = run(["findmnt", "-J", "-T", str(path), "-o", "TARGET,SOURCE,FSTYPE,OPTIONS"])
    filesystems = json.loads(result.stdout).get("filesystems", [])
    if not filesystems:
        raise SetupError(f"No mounted filesystem contains {path}")
    return filesystems[0]


def has_symbolic_component(path: pathlib.Path) -> bool:
    """Return whether an existing component of an absolute path is symbolic."""

    current = pathlib.Path(path.anchor)
    for part in path.parts[1:]:
        current /= part
        if current.is_symlink():
            return True
        if not current.exists():
            break
    return False


def resolve_storage_path(value: str | None, home: pathlib.Path) -> pathlib.Path:
    """Resolve the requested Gerrit site and reject broad or symbolic paths."""

    requested = pathlib.Path(value).expanduser() if value else home / "gerrit-site"
    if not requested.is_absolute():
        raise SetupError("GERRIT_STORAGE_PATH must be an absolute path")
    normalized = pathlib.Path(os.path.abspath(requested))
    if has_symbolic_component(normalized):
        raise SetupError(f"GERRIT_STORAGE_PATH contains a symbolic-link component: {normalized}")
    resolved = normalized.resolve(strict=False)
    unsafe = set(UNSAFE_STORAGE_PATHS) | {home.resolve()}
    if resolved in unsafe:
        raise SetupError(f"Refusing unsafe broad Gerrit storage path: {resolved}")
    if resolved.parent == resolved:
        raise SetupError(f"Refusing unsafe Gerrit storage path: {resolved}")
    return resolved


def validate_storage_path(
    value: str | None,
    home: pathlib.Path,
    minimum_free_bytes: int = MINIMUM_FREE_BYTES,
) -> tuple[pathlib.Path, dict[str, object]]:
    """Safely verify storage permissions, locking, mount state, and capacity."""

    site = resolve_storage_path(value, home)
    parent = site.parent
    if not parent.is_dir():
        raise SetupError(f"Gerrit storage parent does not exist: {parent}")
    if site.exists() and not site.is_dir():
        raise SetupError(f"Gerrit storage path exists but is not a directory: {site}")
    if (site / "etc" / "gerrit.config").is_file():
        raise SetupError(
            f"Existing Gerrit site detected at {site}; storage write probes were not run there"
        )
    if site.exists() and site.stat().st_uid != os.geteuid():
        raise SetupError(f"Gerrit storage directory is not owned by the current user: {site}")
    mount = storage_mount(parent)
    filesystem_type = str(mount.get("fstype", "unknown"))
    if filesystem_type in {"tmpfs", "ramfs", "overlay"}:
        raise SetupError(
            f"Gerrit storage is not confirmed persistent: {filesystem_type} at "
            f"{mount.get('target', parent)}"
        )
    options = str(mount.get("options", "")).split(",")
    if "ro" in options:
        raise SetupError(f"Gerrit storage filesystem is read-only: {mount.get('target', parent)}")
    probe_root = site if site.exists() else parent
    usage = shutil.disk_usage(probe_root)
    if usage.free < minimum_free_bytes:
        required_gib = minimum_free_bytes / 1024**3
        raise SetupError(
            f"Gerrit storage has insufficient free space: {usage.free} bytes; "
            f"at least {required_gib:.0f} GiB is required"
        )
    temporary: pathlib.Path | None = None
    try:
        temporary = pathlib.Path(tempfile.mkdtemp(prefix=".gerrit-storage-check-", dir=probe_root))
        if temporary.stat().st_uid != os.geteuid():
            raise SetupError("Temporary Gerrit storage directory has unexpected ownership")
        temporary.chmod(0o700)
        if stat.S_IMODE(temporary.stat().st_mode) != 0o700:
            raise SetupError("Gerrit storage does not preserve Linux directory permissions")
        child = temporary / "directory"
        child.mkdir(mode=0o700)
        probe = temporary / "lock-test"
        probe.write_text("write-test\n", encoding="utf-8")
        if probe.read_text(encoding="utf-8") != "write-test\n":
            raise SetupError("Gerrit storage read-after-write validation failed")
        probe.write_text("modify-test\n", encoding="utf-8")
        probe.chmod(0o600)
        if stat.S_IMODE(probe.stat().st_mode) != 0o600:
            raise SetupError("Gerrit storage does not preserve Linux file permissions")
        with probe.open("r+", encoding="utf-8") as lock_file:
            fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
            fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)
        probe.unlink()
        child.rmdir()
    except (OSError, PermissionError) as error:
        raise SetupError(
            f"Gerrit storage validation failed for {site}: {error}. Use {home / 'gerrit-site'} "
            "or ask the administrator to correct the shared-disk ownership and permissions."
        ) from error
    finally:
        if temporary is not None:
            shutil.rmtree(temporary, ignore_errors=True)
    return site, mount


def optional_command_output(arguments: list[str], unavailable: str) -> str:
    """Return optional command output or a stable unavailable description."""

    if command_path(arguments[0]) is None:
        return unavailable
    return run(arguments, check=False).stdout.strip()


def session_assessment() -> dict[str, object]:
    """Collect process-management and optional session information."""

    return {
        "quota": optional_command_output(["quota", "-s"], "quota command unavailable"),
        "user_systemd": optional_command_output(
            ["systemctl", "--user", "is-system-running"],
            "user systemd unavailable",
        ),
        "linger": optional_command_output(
            ["loginctl", "show-user", str(os.getuid()), "-p", "Linger"],
            "loginctl unavailable",
        ),
        "process_tools": {name: command_path(name) for name in ("nohup", "screen", "tmux")},
    }


def assessment(home: pathlib.Path) -> dict[str, object]:
    """Collect the complete read-only prerequisite assessment."""

    java = command_path("java")
    usage = shutil.disk_usage(home)
    persistent, mount_evidence = persistent_storage(home)
    report = {
        "user": os.environ.get("USER") or run(["id", "-un"]).stdout.strip(),
        "home": str(home),
        "hostname": socket.gethostname(),
        "architecture": run(["uname", "-m"]).stdout.strip(),
        "kernel": run(["uname", "-sr"]).stdout.strip(),
        "os_release": pathlib.Path("/etc/os-release").read_text(encoding="utf-8").strip(),
        "memory": run(["free", "-h"]).stdout.strip(),
        "disk_free_bytes": usage.free,
        "disk_total_bytes": usage.total,
        "storage_persistent": persistent,
        "mount": mount_evidence,
        "programs": {name: command_path(name) for name in ASSESSMENT_PROGRAMS},
        "java": java,
        "java_major": java_major(java) if java else None,
        "addresses": network_addresses(),
        "listeners": run(["ss", "-lnt"]).stdout.strip(),
    }
    report.update(session_assessment())
    return report


def print_assessment(report: dict[str, object]) -> None:
    """Print assessment JSON followed by the decisions still requiring input."""

    print(json.dumps(report, indent=2, sort_keys=True))
    print("\nRequired before install:")
    print("- select an assigned IP proven reachable from eduVPN")
    print("- choose a private password for the initial joxy administrator account")
    print("- accept and distribute the generated internal self-signed certificate safely")
    print("- provide a published Gerrit SHA-256 checksum")
    print("- test HTTPS and SSH from two separate computers after installation")


def require_safe_home(home: pathlib.Path) -> None:
    """Reject root, relative, missing, or symlinked home directories."""

    if os.geteuid() == 0:
        raise SetupError("Refusing to install Gerrit as root")
    if not home.is_absolute() or not home.is_dir() or home.is_symlink():
        raise SetupError(f"Unsafe home directory: {home}")


def validate_authentication(args: argparse.Namespace) -> None:
    """Require safe inputs for administrator-managed individual authentication."""

    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]{0,63}", args.admin_username):
        raise SetupError("--admin-username must be a safe Gerrit username")
    if not args.caddy_url or not args.caddy_sha256:
        raise SetupError("Local authentication requires --caddy-url and --caddy-sha256")
    password = os.environ.get("GERRIT_ADMIN_PASSWORD")
    if not password or len(password) < 12:
        raise SetupError("GERRIT_ADMIN_PASSWORD must be set and contain at least 12 characters")
    if any(character in password for character in ("\r", "\n", "\0")):
        raise SetupError("GERRIT_ADMIN_PASSWORD cannot contain control characters")


def split_listen_address(value: str, option: str) -> tuple[str, int]:
    """Parse one host-and-port listen address without accepting wildcards."""

    if value.count(":") != 1:
        raise SetupError(f"{option} must use HOST:PORT syntax")
    host, port_text = value.rsplit(":", 1)
    if not host or host in {"*", "0.0.0.0"} or not port_text.isdigit():
        raise SetupError(f"{option} must use a specific HOST:PORT endpoint")
    port = int(port_text)
    if port <= 1024 or port > 65535:
        raise SetupError(f"{option} must use an unprivileged valid port")
    return host, port


def validate_endpoint_configuration(args: argparse.Namespace, addresses: list[dict[str, object]]) -> None:
    """Validate canonical, internal HTTP, and explicit Gerrit SSH endpoints."""

    canonical = urllib.parse.urlsplit(args.canonical_web_url)
    if canonical.scheme != "https" or not canonical.hostname or canonical.username:
        raise SetupError("GERRIT_CANONICAL_WEB_URL must be an HTTPS URL without credentials")
    if not args.canonical_web_url.endswith("/"):
        raise SetupError("GERRIT_CANONICAL_WEB_URL must end with /")
    if canonical.hostname != args.server_ip:
        raise SetupError(
            "GERRIT_CANONICAL_WEB_URL host must match GERRIT_SERVER_HOST so the generated "
            "certificate and bound HTTPS endpoint remain valid"
        )
    expected_http = f"proxy-https://127.0.0.1:{args.http_port}/"
    if args.http_listen_url != expected_http:
        raise SetupError(
            "GERRIT_HTTP_LISTEN_URL must keep the authenticated Gerrit backend on "
            f"{expected_http}"
        )
    ssh_host, ssh_port = split_listen_address(
        args.ssh_listen_address, "GERRIT_SSH_LISTEN_ADDRESS"
    )
    if ssh_port != args.ssh_port:
        raise SetupError("GERRIT_SSH_LISTEN_ADDRESS port must match GERRIT_SSH_PORT")
    if ssh_host != "127.0.0.1":
        validate_server_ip(ssh_host, addresses)


def select_caddy_artifact(args: argparse.Namespace, architecture: str) -> None:
    """Select the pinned verified Caddy artifact unless an explicit pair is supplied."""

    if bool(args.caddy_url) != bool(args.caddy_sha256):
        raise SetupError("Provide both --caddy-url and --caddy-sha256, or neither")
    if args.caddy_url:
        return
    artifact = CADDY_ARTIFACTS.get(architecture)
    if artifact is None:
        raise SetupError(
            f"No pinned Caddy artifact for architecture {architecture}; provide a verified URL and SHA-256"
        )
    args.caddy_url, args.caddy_sha256 = artifact


def download_verified(url: str, expected_digest: str, destination: pathlib.Path) -> None:
    """Download one artifact and atomically retain it only after verification."""

    if not url.startswith("https://"):
        raise SetupError("Downloads require an https:// URL")
    if len(expected_digest) != 64 or any(
        character not in "0123456789abcdefABCDEF" for character in expected_digest
    ):
        raise SetupError("A complete hexadecimal SHA-256 checksum is required")
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_suffix(destination.suffix + ".partial")
    try:
        with urllib.request.urlopen(url, timeout=60) as response, temporary.open("wb") as output:
            if not response.geturl().startswith("https://"):
                raise SetupError(f"Download redirected to an insecure URL: {response.geturl()}")
            shutil.copyfileobj(response, output)
        actual = sha256(temporary)
        if actual.lower() != expected_digest.lower():
            raise SetupError(f"Checksum mismatch for {url}: expected {expected_digest}, got {actual}")
        temporary.replace(destination)
    finally:
        if temporary.exists():
            temporary.unlink()


def safe_archive_members(source: tarfile.TarFile) -> list[tarfile.TarInfo]:
    """Return archive members only when every path stays below its destination."""

    members = source.getmembers()
    unsafe = any(
        member.name.startswith("/") or ".." in pathlib.PurePosixPath(member.name).parts
        for member in members
    )
    if not members or unsafe:
        raise SetupError("Unsafe portable archive paths")
    return members


def extract_java(archive: pathlib.Path, extract_root: pathlib.Path) -> pathlib.Path:
    """Extract one verified Java archive and return its versioned home."""

    with tarfile.open(archive, "r:*") as source:
        members = safe_archive_members(source)
        top_levels = {pathlib.PurePosixPath(member.name).parts[0] for member in members}
        if len(top_levels) != 1:
            raise SetupError("Portable Java archive must contain one top-level directory")
        source.extractall(extract_root, filter="data")
    return extract_root / top_levels.pop()


def install_portable_java(args: argparse.Namespace, home: pathlib.Path) -> pathlib.Path:
    """Use system Java 21 or install a verified portable Java archive."""

    system_java = command_path("java")
    if system_java and java_major(system_java) == 21:
        return pathlib.Path(system_java).resolve().parent.parent
    if not args.jdk_url or not args.jdk_sha256:
        raise SetupError("Java 21 is unavailable; provide --jdk-url and --jdk-sha256")
    downloads = home / "apps" / "java" / "downloads"
    archive = downloads / pathlib.Path(args.jdk_url).name
    download_verified(args.jdk_url, args.jdk_sha256, archive)
    extract_root = home / "apps" / "java"
    java_home = extract_java(archive, extract_root)
    if java_major(str(java_home / "bin" / "java")) != 21:
        raise SetupError("Downloaded portable Java is not Java 21")
    current = extract_root / "current"
    if current.exists() or current.is_symlink():
        raise SetupError(f"Refusing to replace existing Java link: {current}")
    current.symlink_to(java_home.name)
    return current


def extract_caddy(archive: pathlib.Path, current: pathlib.Path) -> pathlib.Path:
    """Extract the sole Caddy executable from its verified archive."""

    with tarfile.open(archive, "r:*") as source:
        candidates = [
            member for member in safe_archive_members(source)
            if member.isfile() and pathlib.PurePosixPath(member.name).name == "caddy"
        ]
        if len(candidates) != 1:
            raise SetupError("Portable Caddy archive must contain exactly one caddy binary")
        extracted = source.extractfile(candidates[0])
        if extracted is None:
            raise SetupError("Unable to read the Caddy binary from its archive")
        current.mkdir(parents=True)
        binary = current / "caddy"
        with binary.open("wb") as output:
            shutil.copyfileobj(extracted, output)
    binary.chmod(0o700)
    return binary


def caddy_version(binary: pathlib.Path) -> str:
    """Return a supported Caddy semantic version or reject the binary."""

    version = run([str(binary), "version"]).stdout.strip()
    match = re.match(r"v([0-9]+)\.([0-9]+)\.([0-9]+)", version)
    if match is None:
        raise SetupError("Downloaded Caddy binary did not report a supported semantic version")
    if tuple(int(component) for component in match.groups()) < (2, 11, 1):
        raise SetupError("Caddy 2.11.1 or newer is required for signal-based configuration reloads")
    return version


def install_portable_caddy(args: argparse.Namespace, home: pathlib.Path) -> tuple[pathlib.Path, str]:
    """Install one verified portable Caddy archive below the current user's home."""

    downloads = home / "apps" / "caddy" / "downloads"
    archive = downloads / pathlib.Path(args.caddy_url).name
    download_verified(args.caddy_url, args.caddy_sha256, archive)
    current = home / "apps" / "caddy" / "current"
    if current.exists():
        raise SetupError(f"Refusing to replace existing Caddy installation: {current}")
    binary = extract_caddy(archive, current)
    return current, caddy_version(binary)


def git_config(config: pathlib.Path, section_key: str, value: str) -> None:
    """Write one Gerrit Git-config value using Git's parser."""

    run(["git", "config", "--file", str(config), section_key, value])


def git_config_values(config: pathlib.Path, section_key: str, values: tuple[str, ...]) -> None:
    """Replace one Gerrit multi-value setting using Git's parser."""

    run(["git", "config", "--file", str(config), "--unset-all", section_key], check=False)
    for value in values:
        run(["git", "config", "--file", str(config), "--add", section_key, value])


def copy_templates(
    source_root: pathlib.Path,
    home: pathlib.Path,
    replacements: dict[str, str],
    site: pathlib.Path | None = None,
) -> None:
    """Copy reviewed scripts and render documentation placeholders."""

    bin_directory = home / "bin"
    selected_site = site or home / "gerrit-site"
    docs_directory = selected_site / "docs"
    bin_directory.mkdir(parents=True, exist_ok=True)
    docs_directory.mkdir(parents=True, exist_ok=True)
    note_directory = source_root / "DevloperNote" / "Doc" / "note"
    ci_config = source_root / "config" / "XWALK_CI_ENV.example"
    render_templates(source_root / "bin", bin_directory, replacements, 0o700)
    render_templates(note_directory, docs_directory, replacements, 0o600)
    render_file(ci_config, docs_directory / ci_config.name, replacements, 0o600)
    copy_tool_assets(source_root, home, selected_site)


def copy_tool_assets(source_root: pathlib.Path, home: pathlib.Path, site: pathlib.Path) -> None:
    """Install the CI runner and UI plugin into the user-owned Gerrit service."""

    tool_directory = home / "apps" / "gerrit" / "tools"
    tool_directory.mkdir(parents=True, exist_ok=True)
    for name in (
        "xWalkGerritCi.py", "xWalkGerritLogServer.py", "xWalkGerritQuality.py",
    ):
        target = tool_directory / name
        shutil.copyfile(source_root / "py-src" / name, target)
        target.chmod(0o700 if name == "xWalkGerritCi.py" else 0o600)
    architecture_root = site / "gerrit-tools"
    for relative in (
        pathlib.Path("config/multi-repo.conf"),
        pathlib.Path("py-src/xWalkGerritAcl.py"),
        pathlib.Path("shell-script/xwalk-gerrit-common.sh"),
        pathlib.Path("shell-script/gerrit-auto-uplift.sh"),
        pathlib.Path("shell-script/gerrit-github-checkout.sh"),
        pathlib.Path("shell-script/gerrit-github-sync.sh"),
    ):
        target = architecture_root / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source_root / relative, target)
        target.chmod(0o700 if relative.suffix == ".sh" else 0o600)
    plugin = site / "plugins" / "xWalkReviewControls.js"
    plugin.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source_root / "xWalkReviewControls.js", plugin)
    plugin.chmod(0o600)


def render_templates(
    source_directory: pathlib.Path,
    target_directory: pathlib.Path,
    replacements: dict[str, str],
    mode: int,
) -> None:
    """Render one template directory with a consistent target mode."""

    for source in source_directory.iterdir():
        render_file(source, target_directory / source.name, replacements, mode)


def render_file(
    source: pathlib.Path,
    target: pathlib.Path,
    replacements: dict[str, str],
    mode: int,
) -> None:
    """Render one reviewed file with concrete installation values."""

    content = source.read_text(encoding="utf-8")
    for key, value in replacements.items():
        content = content.replace(f"@@{key}@@", value)
    target.write_text(content, encoding="utf-8")
    target.chmod(mode)


def configure_self_signed_certificate(
    args: argparse.Namespace,
    site: pathlib.Path,
) -> tuple[pathlib.Path, pathlib.Path, str]:
    """Generate an IP-bound self-signed certificate for the local Caddy proxy."""

    certificate = site / "etc" / "gerrit-self-signed.crt"
    private_key = site / "etc" / "gerrit-self-signed.key"
    run(
        [
            "openssl", "req", "-x509", "-newkey", "rsa:3072", "-sha256", "-nodes",
            "-days", "397", "-subj", f"/CN={args.server_ip}",
            "-addext", f"subjectAltName=IP:{args.server_ip}",
            "-keyout", str(private_key), "-out", str(certificate),
        ]
    )
    private_key.chmod(0o600)
    certificate.chmod(0o644)
    fingerprint = run(
        ["openssl", "x509", "-in", str(certificate), "-noout", "-fingerprint", "-sha256"]
    ).stdout.strip()
    return certificate, private_key, fingerprint


def hash_local_password(caddy_home: pathlib.Path) -> str:
    """Hash the initial administrator password through Caddy without using an argument."""

    password = os.environ.get("GERRIT_ADMIN_PASSWORD")
    if not password or len(password) < 12:
        raise SetupError("GERRIT_ADMIN_PASSWORD must be set and contain at least 12 characters")
    result = run(
        [str(caddy_home / "caddy"), "hash-password", "--algorithm", "bcrypt"],
        input_text=f"{password}\n",
    )
    password_hash = result.stdout.strip()
    if not re.fullmatch(r"\$2[aby]\$[0-9]{2}\$[./A-Za-z0-9]{53}", password_hash):
        raise SetupError("Caddy did not produce a valid bcrypt password hash")
    return password_hash


def curl_config_value(value: str) -> str:
    """Escape one trusted value for a double-quoted curl configuration field."""

    if any(character in value for character in ("\r", "\n", "\0")):
        raise SetupError("Credentials and paths cannot contain control characters")
    return value.replace("\\", "\\\\").replace('"', '\\"')


def caddy_global_block(site: pathlib.Path) -> str:
    """Build the global Caddy configuration block."""

    runtime_log = json.dumps(str(site / "logs" / "caddy-runtime.log"))
    return (
        "{\n"
        "    admin off\n"
        "    auto_https disable_redirects\n"
        "    servers {\n"
        "        protocols h1 h2\n"
        "    }\n"
        "    log {\n"
        f"        output file {runtime_log}\n"
        "        level INFO\n"
        "    }\n"
        "}\n\n"
    )


def caddy_login_route(users: pathlib.Path, backend_port: int) -> str:
    """Build the password-protected Gerrit login route."""

    return (
        "        @login path /login /login/*\n"
        "        handle @login {\n"
        "            basic_auth {\n"
        f"                import {json.dumps(str(users))}\n"
        "            }\n"
        f"            reverse_proxy 127.0.0.1:{backend_port} {{\n"
        "                header_up -Authorization\n"
        "                header_up -X-Forwarded-For\n"
        "                header_up X-Gerrit-User {http.auth.user.id}\n"
        "            }\n"
        "        }\n"
    )


def caddy_git_route(users: pathlib.Path, backend_port: int) -> str:
    """Build the authenticated Git-over-HTTPS proxy route."""

    return (
        "        @git_http path */info/refs */git-upload-pack */git-receive-pack\n"
        "        handle @git_http {\n"
        "            basic_auth {\n"
        f"                import {json.dumps(str(users))}\n"
        "            }\n"
        f"            reverse_proxy 127.0.0.1:{backend_port} {{\n"
        "                header_up -Authorization\n"
        "                header_up -X-Forwarded-For\n"
        "                header_up X-Gerrit-User {http.auth.user.id}\n"
        "            }\n"
        "        }\n"
    )


def caddy_public_route(backend_port: int) -> str:
    """Build the anonymous review route with identity spoofing removed."""

    return (
        "        handle {\n"
        f"            reverse_proxy 127.0.0.1:{backend_port} {{\n"
        "                header_up -Authorization\n"
        "                header_up -X-Forwarded-For\n"
        "                header_up -X-Gerrit-User\n"
        "            }\n"
        "        }\n"
    )


def caddy_ci_route() -> str:
    """Build the HTTPS route to the loopback-only CI log dashboard."""

    return (
        "        handle_path /ci/* {\n"
        "            reverse_proxy 127.0.0.1:8091\n"
        "        }\n"
    )


def caddy_site_block(
    args: argparse.Namespace,
    certificate: pathlib.Path,
    private_key: pathlib.Path,
    users: pathlib.Path,
    access_log: pathlib.Path,
) -> str:
    """Build the IP-bound Caddy site block."""

    ci_route = caddy_ci_route()
    git_route = caddy_git_route(users, args.http_port)
    login_route = caddy_login_route(users, args.http_port)
    public_route = caddy_public_route(args.http_port)
    return (
        f"https://{args.server_ip}:{args.https_port} {{\n"
        f"    bind {args.server_ip}\n"
        f"    tls {json.dumps(str(certificate))} {json.dumps(str(private_key))}\n"
        "    route {\n"
        f"{git_route}{ci_route}{login_route}{public_route}"
        "    }\n"
        "    log {\n"
        f"        output file {json.dumps(str(access_log))}\n"
        "    }\n"
        "}\n"
    )


def write_caddy_configuration(
    args: argparse.Namespace,
    home: pathlib.Path,
    site: pathlib.Path,
    certificate: pathlib.Path,
    private_key: pathlib.Path,
    password_hash: str,
) -> pathlib.Path:
    """Create the private local-user file and loopback-only Gerrit reverse proxy."""

    configuration_directory = home / "gerrit-proxy"
    configuration_directory.mkdir(mode=0o700, exist_ok=True)
    configuration_directory.chmod(0o700)
    users = configuration_directory / "users.caddy"
    users.write_text(f"{args.admin_username} {password_hash}\n", encoding="utf-8")
    users.chmod(0o600)
    configuration = configuration_directory / "Caddyfile"
    access_log = site / "logs" / "caddy-access.log"
    content = caddy_global_block(site)
    content += caddy_site_block(args, certificate, private_key, users, access_log)
    configuration.write_text(content, encoding="utf-8")
    configuration.chmod(0o600)
    return configuration


def write_environment(
    home: pathlib.Path,
    site: pathlib.Path,
    java_home: pathlib.Path,
    proxy_mode: str,
    caddy_home: pathlib.Path | None,
    process_manager: str = "nohup",
) -> None:
    """Create the stable user environment without storing secrets."""

    target = home / "gerrit-env.sh"
    target.write_text(
        "#!/bin/sh\n"
        f"export JAVA_HOME='{java_home}'\n"
        f"export GERRIT_SITE='{site}'\n"
        f"export GERRIT_PROXY_MODE='{proxy_mode}'\n"
        f"export CADDY_HOME='{caddy_home or ''}'\n"
        f"export CADDY_CONFIG='{home / 'gerrit-proxy' / 'Caddyfile'}'\n"
        f"export GERRIT_PROCESS_MANAGER='{process_manager}'\n"
        f"export XWALK_GERRIT_TOOL_HOME='{home / 'apps' / 'gerrit' / 'tools'}'\n"
        "case \":$PATH:\" in\n"
        "    *\":$JAVA_HOME/bin:\"*) ;;\n"
        "    *) export PATH=\"$JAVA_HOME/bin:$PATH\" ;;\n"
        "esac\n",
        encoding="utf-8",
    )
    target.chmod(0o600)


def validate_ports(args: argparse.Namespace) -> None:
    """Require valid unused addresses for every Gerrit endpoint."""

    for port in (args.https_port, args.ssh_port, args.http_port):
        if port <= 1024 or port > 65535:
            raise SetupError(f"All service ports must be unprivileged and valid: {port}")
    ssh_host, _ = split_listen_address(
        args.ssh_listen_address, "GERRIT_SSH_LISTEN_ADDRESS"
    )
    endpoints = (
        (args.server_ip, args.https_port),
        (ssh_host, args.ssh_port),
        ("127.0.0.1", args.http_port),
    )
    for address, port in endpoints:
        if not port_is_free(address, port):
            raise SetupError(f"Required address is occupied: {address}:{port}")


def validate_install(args: argparse.Namespace, home: pathlib.Path) -> str:
    """Validate host, storage, network, authentication, and branch inputs."""

    require_safe_home(home)
    report = assessment(home)
    if not report["storage_persistent"]:
        raise SetupError(f"Home storage is not confirmed persistent: {report['mount']}")
    missing = [name for name in REQUIRED_PROGRAMS if command_path(name) is None]
    if missing:
        request = ", ".join(missing)
        raise SetupError(
            f"Missing required programs: {request}. Ask the server administrator to make "
            "these commands available in your login environment without privilege escalation."
        )
    interface = validate_server_ip(args.server_ip, report["addresses"])
    validate_endpoint_configuration(args, report["addresses"])
    validate_ports(args)
    select_caddy_artifact(args, str(report["architecture"]))
    validate_authentication(args)
    if args.process_manager == "systemd":
        missing_systemd = [name for name in ("systemctl", "systemd-run") if command_path(name) is None]
        systemd_state = str(report["user_systemd"])
        if missing_systemd or systemd_state in {"", "offline", "failed", "user systemd unavailable"}:
            raise SetupError(
                "User-level systemd is unavailable; set GERRIT_PROCESS_MANAGER=nohup "
                "or ask the administrator to enable the user manager"
            )
    if run(["git", "check-ref-format", "--branch", args.project_branch], check=False).returncode != 0:
        raise SetupError(f"Invalid --project-branch: {args.project_branch}")
    return interface


def process_environment(java_home: pathlib.Path) -> dict[str, str]:
    """Build the child-process environment for the selected Java runtime."""

    environment = os.environ.copy()
    environment["JAVA_HOME"] = str(java_home)
    environment["PATH"] = f"{java_home / 'bin'}:{environment.get('PATH', '')}"
    return environment


def install_runtimes(
    args: argparse.Namespace,
    home: pathlib.Path,
    site: pathlib.Path,
) -> tuple[pathlib.Path, pathlib.Path, str, dict[str, str]]:
    """Install Java and Caddy and write their shared environment."""

    java_home = install_portable_java(args, home)
    caddy_home, version = install_portable_caddy(args, home)
    environment = process_environment(java_home)
    write_environment(home, site, java_home, "local-http", caddy_home, args.process_manager)
    return java_home, caddy_home, version, environment


def install_gerrit_war(
    args: argparse.Namespace,
    home: pathlib.Path,
    java_home: pathlib.Path,
) -> pathlib.Path:
    """Download, verify, and retain the selected Gerrit WAR."""

    app_directory = home / "apps" / "gerrit"
    download = app_directory / "downloads" / f"gerrit-{GERRIT_VERSION}.war"
    download_verified(args.gerrit_url, args.gerrit_sha256, download)
    app_directory.mkdir(parents=True, exist_ok=True)
    war = app_directory / "gerrit.war"
    shutil.copyfile(download, war)
    version = run([str(java_home / "bin" / "java"), "-jar", str(war), "version"]).stdout
    if GERRIT_VERSION not in version:
        raise SetupError("Downloaded WAR did not report the selected Gerrit version")
    return war


def initialize_site(
    args: argparse.Namespace,
    site: pathlib.Path,
    java_home: pathlib.Path,
    war: pathlib.Path,
) -> pathlib.Path:
    """Initialize the Gerrit site with loopback-only temporary endpoints."""

    run(
        [
            str(java_home / "bin" / "java"), "-jar", str(war), "init", "--batch",
            "--no-auto-start", "--skip-all-downloads",
            "--install-plugin=download-commands", "-d", str(site),
        ]
    )
    config = site / "etc" / "gerrit.config"
    git_config(config, "gerrit.basePath", "git")
    git_config(config, "gerrit.canonicalWebUrl", f"http://127.0.0.1:{args.http_port}/")
    git_config(config, "httpd.listenUrl", f"http://127.0.0.1:{args.http_port}/")
    git_config(config, "sshd.listenAddress", f"127.0.0.1:{args.ssh_port}")
    git_config(config, "auth.type", "OPENID")
    return config


def protect_site_permissions(site: pathlib.Path) -> None:
    """Apply owner-only access to Gerrit's site and sensitive top-level data."""

    site.chmod(0o700)
    for name in ("cache", "data", "db", "etc", "git", "index", "logs", "plugins", "tmp"):
        directory = site / name
        if directory.is_dir():
            directory.chmod(0o700)
    for name in ("gerrit.config", "secure.config", "ssh_host_ecdsa_key", "ssh_host_ed25519_key",
                 "ssh_host_rsa_key"):
        sensitive = site / "etc" / name
        if sensitive.is_file():
            sensitive.chmod(0o600)


def validate_loopback_site(
    args: argparse.Namespace,
    site: pathlib.Path,
    environment: dict[str, str],
) -> None:
    """Start, probe, and stop the temporary loopback Gerrit site."""

    control = site / "bin" / "gerrit.sh"
    run([str(control), "start"], environment=environment)
    try:
        run(
            [
                "curl", "--fail", "--silent", "--show-error",
                f"http://127.0.0.1:{args.http_port}/config/server/version",
            ]
        )
    finally:
        run([str(control), "stop"], check=False, environment=environment)


def backup_initial_configuration(home: pathlib.Path, site: pathlib.Path) -> pathlib.Path:
    """Create and verify the pre-network Gerrit configuration backup."""

    backup_root = home / "backups" / "gerrit"
    backup_root.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    backup = backup_root / f"pre-network-config-{timestamp}.tar.gz"
    with tarfile.open(backup, "x:gz") as archive:
        archive.add(site / "etc", arcname="gerrit-site/etc")
    backup.chmod(0o600)
    with tarfile.open(backup, "r:gz") as archive:
        if not archive.getmembers():
            raise SetupError("Initial configuration backup is empty")
    checksum = backup.with_suffix(backup.suffix + ".sha256")
    checksum.write_text(f"{sha256(backup)}  {backup.name}\n", encoding="utf-8")
    checksum.chmod(0o600)
    return backup


def configure_final_site(args: argparse.Namespace, config: pathlib.Path) -> None:
    """Configure public reviews, protected login, and IP-bound SSH."""

    git_config(config, "gerrit.canonicalWebUrl", args.canonical_web_url)
    git_config(config, "sshd.listenAddress", args.ssh_listen_address)
    git_config(config, "httpd.listenUrl", args.http_listen_url)
    git_config(config, "auth.type", "HTTP")
    git_config(config, "auth.httpHeader", "X-Gerrit-User")
    git_config(config, "auth.httpTrustedProxyNetworks", "127.0.0.1/32")
    git_config(config, "auth.userNameToLowerCase", "true")
    git_config(config, "auth.loginUrl", "/login/")
    git_config(config, "auth.loginText", "Sign In")
    git_config(config, "sendemail.enable", "false")
    git_config_values(config, "download.command", ("checkout", "cherry_pick", "pull", "format_patch"))
    git_config_values(config, "download.scheme", ("ssh", "http"))
    git_config_values(config, "download.archive", ("tar", "tgz"))


def runtime_documentation_values(
    args: argparse.Namespace, java_home: pathlib.Path, caddy_version_value: str
) -> dict[str, str]:
    """Build runtime and verified-artifact documentation values."""

    java_version = run([str(java_home / "bin" / "java"), "-version"], check=False)
    return {
        "GERRIT_VERSION": GERRIT_VERSION,
        "JAVA_VERSION": java_version.stderr.splitlines()[0],
        "JAVA_HOME": str(java_home),
        "JAVA_SOURCE": args.jdk_url or "server-provided Java 21 runtime",
        "JAVA_SHA256": args.jdk_sha256 or "not applicable to server-provided Java",
        "GERRIT_SHA256": args.gerrit_sha256.lower(),
        "GERRIT_URL": args.gerrit_url,
        "CADDY_VERSION": caddy_version_value,
        "CADDY_URL": args.caddy_url,
        "CADDY_SHA256": args.caddy_sha256,
    }


def documentation_values(
    args: argparse.Namespace,
    java_home: pathlib.Path,
    caddy_version_value: str,
    interface: str,
    fingerprint: str,
) -> dict[str, str]:
    """Build concrete endpoint and identity values for installed guides."""

    values = runtime_documentation_values(args, java_home, caddy_version_value)
    values.update({
        "SERVER_IP": args.server_ip,
        "HTTPS_PORT": str(args.https_port),
        "SSH_PORT": str(args.ssh_port),
        "HTTP_PORT": str(args.http_port),
        "PROJECT_NAME": args.project_name,
        "PROJECT_BRANCH": args.project_branch,
        "HOME": str(pathlib.Path.home().resolve()),
        "INTERFACE": interface,
        "ADMIN_USERNAME": args.admin_username,
        "ADMIN_NAME": args.admin_full_name,
        "ADMIN_ROLE": args.admin_role,
        "ADMIN_EMAIL": args.admin_email,
        "CERTIFICATE_FINGERPRINT": fingerprint,
        "INSTALL_DATE": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "GERRIT_SITE": str(args.site),
        "CANONICAL_WEB_URL": args.canonical_web_url,
        "SSH_LISTEN_ADDRESS": args.ssh_listen_address,
        "HTTP_LISTEN_URL": args.http_listen_url,
    })
    return values


def authenticated_curl_configuration(args: argparse.Namespace, certificate: pathlib.Path) -> str:
    """Build a private curl configuration for the administrator login probe."""

    credentials = curl_config_value(f'{args.admin_username}:{os.environ["GERRIT_ADMIN_PASSWORD"]}')
    certificate_value = curl_config_value(str(certificate))
    return (
        f'user = "{credentials}"\n'
        f'cacert = "{certificate_value}"\n'
        f'url = "https://{args.server_ip}:{args.https_port}/login/"\n'
        "fail\n"
        "silent\n"
        "show-error\n"
    )


def validate_final_site(
    args: argparse.Namespace,
    home: pathlib.Path,
    site: pathlib.Path,
    certificate: pathlib.Path,
    environment: dict[str, str],
) -> None:
    """Start and probe anonymous web, administrator login, and Gerrit SSH."""

    run([str(site / "bin" / "gerrit.sh"), "start"], environment=environment)
    try:
        run([str(home / "bin" / "gerrit-caddy-control"), "start"])
        public_url = f"https://{args.server_ip}:{args.https_port}/config/server/version"
        run(["curl", "--fail", "--silent", "--show-error", "--cacert", str(certificate), public_url])
        run(["curl", "--config", "-"], input_text=authenticated_curl_configuration(args, certificate))
    except (OSError, SetupError):
        run([str(home / "bin" / "gerrit-caddy-control"), "stop"], check=False)
        run([str(site / "bin" / "gerrit.sh"), "stop"], check=False, environment=environment)
        raise
    run(["ssh-keyscan", "-T", "5", "-p", str(args.ssh_port), args.server_ip])


def print_installation(
    args: argparse.Namespace,
    site: pathlib.Path,
    certificate: pathlib.Path,
    fingerprint: str,
    backup: pathlib.Path,
) -> None:
    """Print installed endpoints, trust material, and remaining remote gates."""

    print(f"Installed Gerrit {GERRIT_VERSION} at {site}")
    print(f"HTTPS: {args.canonical_web_url}")
    print(f"SSH: {args.ssh_listen_address}")
    print(f"Self-signed certificate for client trust: {certificate}")
    print(f"Certificate {fingerprint}")
    print("Internal use only: each user must verify and import this public certificate.")
    print(f"Initial configuration backup: {backup}")
    print("Remote eduVPN tests and separate user onboarding are still required.")


def install(args: argparse.Namespace) -> None:
    """Install, configure, validate, and document the user-owned Gerrit site."""

    home = pathlib.Path.home().resolve()
    os.umask(0o077)
    if args.http_listen_url is None:
        args.http_listen_url = f"proxy-https://127.0.0.1:{args.http_port}/"
    if args.canonical_web_url is None:
        args.canonical_web_url = f"https://{args.server_ip}:{args.https_port}/"
    if args.ssh_listen_address is None:
        args.ssh_listen_address = f"127.0.0.1:{args.ssh_port}"
    requested_site = resolve_storage_path(args.storage_path, home)
    if (requested_site / "etc" / "gerrit.config").is_file():
        raise SetupError(
            f"Existing Gerrit site found at {requested_site}. No files were changed. "
            "Use the documented backup, stop, migration, and validation procedure."
        )
    previous_site = home / "gerrit-site"
    if requested_site != previous_site and previous_site.exists():
        raise SetupError(
            f"Existing home Gerrit site found at {previous_site}. Stop Gerrit and follow the "
            f"documented migration procedure before selecting {requested_site}; "
            "the original was not changed."
        )
    site, mount = validate_storage_path(args.storage_path, home)
    args.site = site
    print(f"Selected Gerrit site: {site}")
    print(f"Storage filesystem: {json.dumps(mount, sort_keys=True)}")
    if site.exists() and any(site.iterdir()):
        raise SetupError(
            f"Non-empty Gerrit destination found at {site}. No files were changed. "
            "Use the documented backup, stop, migration, and validation procedure."
        )
    if site.exists():
        site.chmod(0o700)
    interface = validate_install(args, home)
    java_home, caddy_home, caddy_version_value, environment = install_runtimes(args, home, site)
    war = install_gerrit_war(args, home, java_home)
    config = initialize_site(args, site, java_home, war)
    protect_site_permissions(site)
    validate_loopback_site(args, site, environment)
    backup = backup_initial_configuration(home, site)
    certificate, private_key, fingerprint = configure_self_signed_certificate(args, site)
    write_caddy_configuration(
        args, home, site, certificate, private_key, hash_local_password(caddy_home)
    )
    configure_final_site(args, config)
    protect_site_permissions(site)
    values = documentation_values(args, java_home, caddy_version_value, interface, fingerprint)
    copy_templates(pathlib.Path(__file__).resolve().parents[1], home, values, site)
    validate_final_site(args, home, site, certificate, environment)
    print_installation(args, site, certificate, fingerprint, backup)


def add_install_args(install_parser: argparse.ArgumentParser) -> None:
    """Add the non-root Gerrit installation arguments."""

    install_parser.add_argument(
        "--server-host", "--server-ip",
        dest="server_ip",
        required=True,
        help="assigned college-server IPv4 address proven reachable through eduVPN",
    )
    install_parser.add_argument("--storage-path")
    install_parser.add_argument("--https-port", type=int, default=18443)
    install_parser.add_argument("--ssh-port", type=int, default=29418)
    install_parser.add_argument("--http-port", "--init-http-port", dest="http_port", type=int, default=8080)
    install_parser.add_argument("--http-listen-url")
    install_parser.add_argument("--canonical-web-url")
    install_parser.add_argument("--ssh-listen-address")
    install_parser.add_argument("--gerrit-url", default=GERRIT_URL)
    install_parser.add_argument("--gerrit-sha256", required=True)
    install_parser.add_argument("--jdk-url")
    install_parser.add_argument("--jdk-sha256")
    install_parser.add_argument("--caddy-url")
    install_parser.add_argument("--caddy-sha256")
    install_parser.add_argument("--admin-username", default="joxy")
    install_parser.add_argument("--admin-full-name", default="Joxy John")
    install_parser.add_argument("--admin-role", default="Student")
    install_parser.add_argument("--admin-email", default="joxjoh24@student.hh.se")
    install_parser.add_argument("--process-manager", choices=("nohup", "systemd"), default="nohup")
    install_parser.add_argument("--project-name", required=True)
    install_parser.add_argument("--project-branch", default="master")


def parser() -> argparse.ArgumentParser:
    """Build the command-line interface."""

    argument_parser = argparse.ArgumentParser(description=__doc__)
    subparsers = argument_parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("assess", help="perform the read-only server assessment")
    storage_parser = subparsers.add_parser(
        "validate-storage", help="safely validate a home or shared-disk Gerrit site"
    )
    storage_parser.add_argument("--storage-path")
    install_parser = subparsers.add_parser(
        "install",
        help="install after assessment, local administrator password, and checksum are available",
    )
    add_install_args(install_parser)
    return argument_parser


def main() -> int:
    """Run assessment or installation and provide actionable failure output."""

    arguments = parser().parse_args()
    try:
        if arguments.command == "assess":
            print_assessment(assessment(pathlib.Path.home().resolve()))
        elif arguments.command == "validate-storage":
            site, mount = validate_storage_path(arguments.storage_path, pathlib.Path.home().resolve())
            print(f"Selected Gerrit site: {site}")
            print(f"Storage validation passed: {json.dumps(mount, sort_keys=True)}")
        else:
            install(arguments)
    except (OSError, SetupError, ValueError, json.JSONDecodeError) as error:
        print(f"GERRIT_SETUP: FAILED - {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
