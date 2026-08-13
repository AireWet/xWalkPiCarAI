#!/usr/bin/env python3
"""Assess and install a non-root Gerrit server in the current user's home."""

from __future__ import annotations

import argparse
import datetime
import hashlib
import ipaddress
import json
import os
import pathlib
import shutil
import socket
import subprocess
import sys
import tarfile
import tempfile
import urllib.request


GERRIT_VERSION = "3.14.2"
GERRIT_URL = (
    "https://gerrit-releases.storage.googleapis.com/"
    f"gerrit-{GERRIT_VERSION}.war"
)
REJECTED_INTERFACES = ("docker", "virbr", "vbox", "vmnet")
ASSESSMENT_PROGRAMS = (
    "git", "ssh", "ssh-keygen", "ssh-keyscan", "curl", "wget", "tar",
    "unzip", "openssl", "keytool", "ss", "nc", "nohup", "screen", "tmux",
)
REQUIRED_PROGRAMS = (
    "git", "ssh", "ssh-keygen", "ssh-keyscan", "curl", "tar", "openssl", "ss",
)


class SetupError(RuntimeError):
    """Raised when a safe installation prerequisite is not satisfied."""


def run(
    arguments: list[str],
    *,
    check: bool = True,
    environment: dict[str, str] | None = None,
) -> subprocess.CompletedProcess[str]:
    """Run one command without a shell and return its captured result."""

    result = subprocess.run(
        arguments,
        check=False,
        env=environment,
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


def assessment(home: pathlib.Path) -> dict[str, object]:
    """Collect the complete read-only prerequisite assessment."""

    java = command_path("java")
    usage = shutil.disk_usage(home)
    persistent, mount_evidence = persistent_storage(home)
    return {
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
        "quota": (
            run(["quota", "-s"], check=False).stdout.strip()
            if command_path("quota")
            else "quota command unavailable"
        ),
        "user_systemd": run(["systemctl", "--user", "is-system-running"], check=False).stdout.strip(),
        "linger": run(["loginctl", "show-user", str(os.getuid()), "-p", "Linger"], check=False).stdout.strip(),
        "process_tools": {name: command_path(name) for name in ("nohup", "screen", "tmux")},
        "programs": {name: command_path(name) for name in ASSESSMENT_PROGRAMS},
        "java": java,
        "java_major": java_major(java) if java else None,
        "addresses": network_addresses(),
        "listeners": run(["ss", "-lnt"]).stdout.strip(),
    }


def print_assessment(report: dict[str, object]) -> None:
    """Print assessment JSON followed by the decisions still requiring input."""

    print(json.dumps(report, indent=2, sort_keys=True))
    print("\nRequired before install:")
    print("- select an assigned IP proven reachable from eduVPN")
    print("- provide approved LDAPS configuration")
    print("- provide a published Gerrit SHA-256 checksum")
    print("- provide approved TLS material or explicitly select self-signed testing")
    print("- test HTTPS and SSH from two separate computers after installation")


def require_safe_home(home: pathlib.Path) -> None:
    """Reject root, relative, missing, or symlinked home directories."""

    if os.geteuid() == 0:
        raise SetupError("Refusing to install Gerrit as root")
    if not home.is_absolute() or not home.is_dir() or home.is_symlink():
        raise SetupError(f"Unsafe home directory: {home}")


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
    with tarfile.open(archive, "r:*") as source:
        members = source.getmembers()
        unsafe_member = any(
            member.name.startswith("/")
            or ".." in pathlib.PurePosixPath(member.name).parts
            for member in members
        )
        if not members or unsafe_member:
            raise SetupError("Unsafe portable Java archive paths")
        top_levels = {pathlib.PurePosixPath(member.name).parts[0] for member in members if member.name}
        if len(top_levels) != 1:
            raise SetupError("Portable Java archive must contain one top-level directory")
        source.extractall(extract_root, filter="data")
    java_home = extract_root / top_levels.pop()
    if java_major(str(java_home / "bin" / "java")) != 21:
        raise SetupError("Downloaded portable Java is not Java 21")
    current = extract_root / "current"
    if current.exists() or current.is_symlink():
        raise SetupError(f"Refusing to replace existing Java link: {current}")
    current.symlink_to(java_home.name)
    return current


def git_config(config: pathlib.Path, section_key: str, value: str) -> None:
    """Write one Gerrit Git-config value using Git's parser."""

    run(["git", "config", "--file", str(config), section_key, value])


def copy_templates(source_root: pathlib.Path, home: pathlib.Path, replacements: dict[str, str]) -> None:
    """Copy reviewed scripts and render documentation placeholders."""

    bin_directory = home / "bin"
    docs_directory = home / "gerrit-site" / "docs"
    bin_directory.mkdir(parents=True, exist_ok=True)
    docs_directory.mkdir(parents=True, exist_ok=True)
    for source in (source_root / "templates" / "bin").iterdir():
        target = bin_directory / source.name
        shutil.copyfile(source, target)
        target.chmod(0o700)
    for source in (source_root / "templates" / "docs").iterdir():
        content = source.read_text(encoding="utf-8")
        for key, value in replacements.items():
            content = content.replace(f"@@{key}@@", value)
        target = docs_directory / source.name
        target.write_text(content, encoding="utf-8")
        target.chmod(0o600)


def configure_tls(
    args: argparse.Namespace,
    java_home: pathlib.Path,
    site: pathlib.Path,
) -> tuple[pathlib.Path, pathlib.Path, str]:
    """Import approved TLS material or generate a labelled test certificate."""

    password = os.environ.get("GERRIT_KEYSTORE_PASSWORD")
    if not password or len(password) < 12:
        raise SetupError("GERRIT_KEYSTORE_PASSWORD must be set and contain at least 12 characters")
    keystore = site / "etc" / "keystore"
    if args.tls_mode == "import":
        source = pathlib.Path(args.tls_keystore or "")
        certificate_source = pathlib.Path(args.tls_ca_certificate or "")
        if not source.is_file():
            raise SetupError("--tls-keystore must name an approved PKCS12 keystore")
        if not certificate_source.is_file():
            raise SetupError("--tls-ca-certificate must name the approved certificate chain")
        shutil.copyfile(source, keystore)
        certificate = site / "etc" / "gerrit-ca.crt"
        shutil.copyfile(certificate_source, certificate)
    else:
        run(
            [
                str(java_home / "bin" / "keytool"), "-genkeypair", "-alias", "jetty",
                "-keyalg", "RSA", "-keysize", "3072", "-validity", "397",
                "-dname", f"CN={args.server_ip}", "-ext", f"SAN=ip:{args.server_ip}",
                "-keystore", str(keystore), "-storetype", "PKCS12",
                "-storepass:env", "GERRIT_KEYSTORE_PASSWORD",
                "-keypass:env", "GERRIT_KEYSTORE_PASSWORD",
            ],
            environment=os.environ.copy(),
        )
        certificate = site / "etc" / "gerrit-self-signed.crt"
        run(
            [
                str(java_home / "bin" / "keytool"), "-exportcert", "-rfc",
                "-alias", "jetty", "-keystore", str(keystore),
                "-storepass:env", "GERRIT_KEYSTORE_PASSWORD", "-file", str(certificate),
            ],
            environment=os.environ.copy(),
        )
        certificate.chmod(0o644)
    keystore.chmod(0o600)
    certificate.chmod(0o644)
    server_certificate = site / "etc" / "gerrit-server.crt"
    run(
        [
            str(java_home / "bin" / "keytool"), "-exportcert", "-rfc",
            "-alias", "jetty", "-keystore", str(keystore),
            "-storepass:env", "GERRIT_KEYSTORE_PASSWORD", "-file", str(server_certificate),
        ],
        environment=os.environ.copy(),
    )
    server_certificate.chmod(0o644)
    fingerprint = run(
        ["openssl", "x509", "-in", str(server_certificate), "-noout", "-fingerprint", "-sha256"]
    ).stdout.strip()
    return keystore, certificate, fingerprint


def write_environment(home: pathlib.Path, java_home: pathlib.Path) -> None:
    """Create the stable user environment without storing secrets."""

    target = home / "gerrit-env.sh"
    target.write_text(
        "#!/bin/sh\n"
        f"export JAVA_HOME='{java_home}'\n"
        f"export GERRIT_SITE='{home / 'gerrit-site'}'\n"
        "case \":$PATH:\" in\n"
        "    *\":$JAVA_HOME/bin:\"*) ;;\n"
        "    *) export PATH=\"$JAVA_HOME/bin:$PATH\" ;;\n"
        "esac\n",
        encoding="utf-8",
    )
    target.chmod(0o600)


def install(args: argparse.Namespace) -> None:
    """Install, configure, validate, and document the user-owned Gerrit site."""

    home = pathlib.Path.home().resolve()
    require_safe_home(home)
    os.umask(0o077)
    report = assessment(home)
    if not report["storage_persistent"]:
        raise SetupError(f"Home storage is not confirmed persistent: {report['mount']}")
    if report["disk_free_bytes"] < 10 * 1024**3:
        raise SetupError("At least 10 GiB of free home-filesystem space is required")
    missing = [name for name in REQUIRED_PROGRAMS if command_path(name) is None]
    if missing:
        raise SetupError(f"Missing required programs: {', '.join(missing)}")
    interface = validate_server_ip(args.server_ip, report["addresses"])
    for port in (args.https_port, args.ssh_port, args.init_http_port):
        if port <= 1024 or port > 65535:
            raise SetupError(f"All service ports must be unprivileged and valid: {port}")
    for port in (args.https_port, args.ssh_port, args.init_http_port):
        address = "127.0.0.1" if port == args.init_http_port else args.server_ip
        if not port_is_free(address, port):
            raise SetupError(f"Required address is occupied: {address}:{port}")
    if not args.ldap_server.startswith("ldaps://"):
        raise SetupError("LDAP authentication requires an approved ldaps:// endpoint")

    site = home / "gerrit-site"
    if site.exists():
        raise SetupError(f"Existing Gerrit site found; back it up and use a reviewed upgrade procedure: {site}")
    java_home = install_portable_java(args, home)
    process_environment = os.environ.copy()
    process_environment["JAVA_HOME"] = str(java_home)
    process_environment["PATH"] = f"{java_home / 'bin'}:{process_environment.get('PATH', '')}"
    write_environment(home, java_home)

    app_directory = home / "apps" / "gerrit"
    download = app_directory / "downloads" / f"gerrit-{GERRIT_VERSION}.war"
    download_verified(args.gerrit_url, args.gerrit_sha256, download)
    app_directory.mkdir(parents=True, exist_ok=True)
    war = app_directory / "gerrit.war"
    shutil.copyfile(download, war)
    if GERRIT_VERSION not in run([str(java_home / "bin" / "java"), "-jar", str(war), "version"]).stdout:
        raise SetupError("Downloaded WAR did not report the selected Gerrit version")

    run(
        [
            str(java_home / "bin" / "java"), "-jar", str(war), "init", "--batch",
            "--no-auto-start", "--skip-all-downloads", "-d", str(site),
        ]
    )
    config = site / "etc" / "gerrit.config"
    secure_config = site / "etc" / "secure.config"
    git_config(config, "gerrit.basePath", "git")
    git_config(config, "gerrit.canonicalWebUrl", f"http://127.0.0.1:{args.init_http_port}/")
    git_config(config, "httpd.listenUrl", f"http://127.0.0.1:{args.init_http_port}/")
    git_config(config, "sshd.listenAddress", f"127.0.0.1:{args.ssh_port}")
    git_config(config, "auth.type", "OPENID")
    run([str(site / "bin" / "gerrit.sh"), "start"], environment=process_environment)
    try:
        run(
            [
                "curl", "--fail", "--silent", "--show-error",
                f"http://127.0.0.1:{args.init_http_port}/config/server/version",
            ]
        )
    finally:
        run(
            [str(site / "bin" / "gerrit.sh"), "stop"],
            check=False,
            environment=process_environment,
        )

    backup_root = home / "backups" / "gerrit"
    backup_root.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    initial_backup = backup_root / f"pre-network-config-{timestamp}.tar.gz"
    with tarfile.open(initial_backup, "x:gz") as archive:
        archive.add(site / "etc", arcname="gerrit-site/etc")
    initial_backup.chmod(0o600)
    with tarfile.open(initial_backup, "r:gz") as archive:
        if not archive.getmembers():
            raise SetupError("Initial configuration backup is empty")
    initial_backup_checksum = initial_backup.with_suffix(initial_backup.suffix + ".sha256")
    initial_backup_checksum.write_text(
        f"{sha256(initial_backup)}  {initial_backup.name}\n",
        encoding="utf-8",
    )
    initial_backup_checksum.chmod(0o600)

    keystore, certificate, fingerprint = configure_tls(args, java_home, site)
    git_config(config, "gerrit.canonicalWebUrl", f"https://{args.server_ip}:{args.https_port}/")
    git_config(config, "httpd.listenUrl", f"https://{args.server_ip}:{args.https_port}/")
    git_config(config, "httpd.sslKeyStore", str(keystore))
    git_config(config, "sshd.listenAddress", f"{args.server_ip}:{args.ssh_port}")
    git_config(config, "auth.type", "LDAP")
    git_config(config, "auth.gitBasicAuthPolicy", "HTTP")
    git_config(config, "ldap.server", args.ldap_server)
    git_config(config, "ldap.accountBase", args.ldap_account_base)
    git_config(config, "ldap.accountPattern", args.ldap_account_pattern)
    git_config(config, "ldap.accountFullName", args.ldap_full_name_attribute)
    git_config(config, "ldap.accountEmailAddress", args.ldap_email_attribute)
    git_config(config, "sendemail.enable", "false")
    git_config(secure_config, "httpd.sslKeyPassword", os.environ["GERRIT_KEYSTORE_PASSWORD"])
    secure_config.chmod(0o600)
    (site / "etc").chmod(0o700)

    replacements = {
        "GERRIT_VERSION": GERRIT_VERSION,
        "JAVA_VERSION": run([str(java_home / "bin" / "java"), "-version"], check=False).stderr.splitlines()[0],
        "JAVA_HOME": str(java_home),
        "JAVA_SOURCE": args.jdk_url or "server-provided Java 21 runtime",
        "JAVA_SHA256": args.jdk_sha256 or "not applicable to server-provided Java",
        "SERVER_IP": args.server_ip,
        "HTTPS_PORT": str(args.https_port),
        "SSH_PORT": str(args.ssh_port),
        "PROJECT_NAME": args.project_name,
        "INTERFACE": interface,
        "GERRIT_SHA256": args.gerrit_sha256.lower(),
        "GERRIT_URL": args.gerrit_url,
        "TLS_MODE": args.tls_mode,
        "LDAP_SERVER": args.ldap_server,
        "TLS_FINGERPRINT": fingerprint,
        "INSTALL_DATE": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    }
    copy_templates(pathlib.Path(__file__).resolve().parent, home, replacements)
    run([str(site / "bin" / "gerrit.sh"), "start"], environment=process_environment)
    run(
        [
            "curl", "--fail", "--silent", "--show-error", "--cacert", str(certificate),
            f"https://{args.server_ip}:{args.https_port}/config/server/version",
        ]
    )
    run(["ssh-keyscan", "-T", "5", "-p", str(args.ssh_port), args.server_ip])
    print(f"Installed Gerrit {GERRIT_VERSION} at {site}")
    print(f"HTTPS: https://{args.server_ip}:{args.https_port}/")
    print(f"SSH: {args.server_ip}:{args.ssh_port}")
    print(f"Initial configuration backup: {initial_backup}")
    print("Remote eduVPN tests and separate user onboarding are still required.")


def parser() -> argparse.ArgumentParser:
    """Build the command-line interface."""

    argument_parser = argparse.ArgumentParser(description=__doc__)
    subparsers = argument_parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("assess", help="perform the read-only server assessment")
    install_parser = subparsers.add_parser("install", help="install after assessment inputs are approved")
    install_parser.add_argument("--server-ip", required=True)
    install_parser.add_argument("--https-port", type=int, default=18443)
    install_parser.add_argument("--ssh-port", type=int, default=29418)
    install_parser.add_argument("--init-http-port", type=int, default=18080)
    install_parser.add_argument("--gerrit-url", default=GERRIT_URL)
    install_parser.add_argument("--gerrit-sha256", required=True)
    install_parser.add_argument("--jdk-url")
    install_parser.add_argument("--jdk-sha256")
    install_parser.add_argument("--auth-type", required=True, choices=("ldap",))
    install_parser.add_argument("--ldap-server", required=True)
    install_parser.add_argument("--ldap-account-base", required=True)
    install_parser.add_argument("--ldap-account-pattern", default="(uid=${username})")
    install_parser.add_argument("--ldap-full-name-attribute", default="displayName")
    install_parser.add_argument("--ldap-email-attribute", default="mail")
    install_parser.add_argument("--tls-mode", required=True, choices=("import", "self-signed"))
    install_parser.add_argument("--tls-keystore")
    install_parser.add_argument("--tls-ca-certificate")
    install_parser.add_argument("--project-name", required=True)
    return argument_parser


def main() -> int:
    """Run assessment or installation and provide actionable failure output."""

    arguments = parser().parse_args()
    try:
        if arguments.command == "assess":
            print_assessment(assessment(pathlib.Path.home().resolve()))
        else:
            install(arguments)
    except (OSError, SetupError, ValueError, json.JSONDecodeError) as error:
        print(f"GERRIT_SETUP: FAILED - {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
