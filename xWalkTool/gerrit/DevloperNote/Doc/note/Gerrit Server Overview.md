# Gerrit server overview

## Purpose

This directory documents the complete user-owned Gerrit installation for the
college server. Gerrit @@GERRIT_VERSION@@ runs as the normal Linux user from
`$HOME/gerrit-site`; its authoritative repositories are stored in
`$HOME/gerrit-site/git`.

- Web URL: `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`
- Public project changes: `https://@@SERVER_IP@@:@@HTTPS_PORT@@/q/project:@@PROJECT_NAME@@`
- Gerrit SSH: `@@SERVER_IP@@:@@SSH_PORT@@`
- Initial administrator: `@@ADMIN_USERNAME@@`
- Project: `@@PROJECT_NAME@@`
- Target branch: `@@PROJECT_BRANCH@@`
- Interface: `@@INTERFACE@@`
- Authentication: individual local Caddy logins
- Certificate: IP-bound self-signed certificate for internal university use

The setup uses no root privilege, system package manager, system service,
system proxy, SSH tunnel, or development impersonation authentication.

## Documentation pages

- [Gerrit Setup Installer](Gerrit%20Setup%20Installer.md) explains assessment,
  installation, generated paths, and lifecycle commands.
- [Gerrit Local Linux Setup](Gerrit%20Local%20Linux%20Setup.md) explains the
  temporary local-host deployment.
- [Gerrit Admin Setup](Gerrit%20Admin%20Setup.md)
  covers accounts, groups, permissions, project creation, and deactivation.
- [Gerrit User Configuration](Gerrit%20User%20Configuration.md) covers login,
  SSH keys, clone, upload, review, and revised patch sets.
- [Gerrit CI Configuration](Gerrit%20CI%20Configuration.md) covers the direct
  non-root verification process, Gerrit voting, logs, and mirroring.
- [Gerrit Backup and Restore](Gerrit%20Backup%20and%20Restore.md) covers safe
  backup, restore, upgrades, rollback, and uninstall.
- [Gerrit Security and Remote Access](Gerrit%20Security%20and%20Remote%20Access.md)
  covers HTTPS, anonymous viewing, eduVPN validation, and access boundaries.
- [Gerrit Troubleshooting](Gerrit%20Troubleshooting.md) covers service,
  connectivity, authentication, SSH, CI, and college-network failures.

## Installed versions

- Gerrit: `@@GERRIT_VERSION@@`
- Gerrit source: `@@GERRIT_URL@@`
- Gerrit SHA-256: `@@GERRIT_SHA256@@`
- Java: `@@JAVA_VERSION@@`
- Java home: `@@JAVA_HOME@@`
- Java source: `@@JAVA_SOURCE@@`
- Java archive SHA-256: `@@JAVA_SHA256@@`
- Caddy: `@@CADDY_VERSION@@`
- Caddy source: `@@CADDY_URL@@`
- Caddy SHA-256: `@@CADDY_SHA256@@`
- Certificate fingerprint: `@@CERTIFICATE_FINGERPRINT@@`
- Installation date: `@@INSTALL_DATE@@`

## Readiness

The server is not ready for secure collaboration until HTTPS and Gerrit SSH
have both been tested from a second computer connected through the university
network or eduVPN. Local success alone establishes only partial configuration.
