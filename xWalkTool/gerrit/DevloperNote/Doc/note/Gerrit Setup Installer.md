# Gerrit setup installer

## Scope

`shell-script/gerrit-setup.sh` is the supported entry point. It invokes
`py-src/xWalkGerritServerSetup.py`, installs the complete Gerrit service as the
current Linux user, starts it, and validates the resulting HTTPS endpoint. It
never invokes `sudo` or a system package manager.

## Assessment

From the repository root on the college server, run:

```bash
xWalkTool/gerrit/shell-script/gerrit-setup.sh assess
```

Confirm the persistent home filesystem, quota, supported Java runtime,
physical-interface address, and free ports. Do not select loopback, a container
bridge, a virtual-only address, or a guessed address.

## Installation

Obtain the official Gerrit WAR SHA-256 checksum. Set it and the detected server
IP in `xWalkTool/gerrit/config/gerrit-setup.conf`. Do not store the administrator
password in that file.

```bash
xWalkTool/gerrit/shell-script/gerrit-setup.sh install
```

The script prompts without echo for the initial `@@ADMIN_USERNAME@@` password
and exports it only to the installer process. The installer verifies downloads,
initializes Gerrit on loopback, validates the local site, backs up the initial
configuration, creates the internal HTTPS certificate, installs Caddy, binds
final services to `@@SERVER_IP@@`, and installs the management commands, CI
programs, UI plugin, and rendered guides.

Start and validate an existing installation with:

```bash
xWalkTool/gerrit/shell-script/gerrit-setup.sh start
```

## Installed paths

- Gerrit application: `$HOME/apps/gerrit`
- Gerrit site: `$HOME/gerrit-site`
- Authoritative repositories: `$HOME/gerrit-site/git`
- Caddy application: `$HOME/apps/caddy`
- Caddy configuration: `$HOME/gerrit-proxy`
- CI programs: `$HOME/apps/gerrit/tools`
- Commands: `$HOME/bin`
- Rendered guides: `$HOME/gerrit-site/docs`
- Backups: `$HOME/backups/gerrit`

## Service operation

Use `$HOME/bin/gerrit-start`, `$HOME/bin/gerrit-stop`,
`$HOME/bin/gerrit-restart`, `$HOME/bin/gerrit-status`,
`$HOME/bin/gerrit-logs`, and `$HOME/bin/gerrit-check`. Manual startup after a
server reboot is supported. Do not create a system-level service.

Print the installed web address with:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

Continue with the administrator configuration before onboarding users.
