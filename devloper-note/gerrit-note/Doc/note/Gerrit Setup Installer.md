# Gerrit setup installer

## Scope

`shell-script/gerrit-setup.sh` is the supported entry point. It invokes
`py-src/xWalkGerritServerSetup.py`, installs the complete Gerrit service as the
current Linux user, starts it, and validates the resulting HTTPS endpoint. It
never invokes `sudo` or a system package manager.

## Assessment

From the repository root on the college server, run:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-setup.sh assess
```

Confirm the persistent home filesystem, quota, supported Java runtime,
physical-interface address, and free ports. Do not select loopback, a container
bridge, a virtual-only address, or a guessed address.

## Installation

Obtain the official Gerrit WAR SHA-256 checksum. Set it and the detected server
IP in `xWalkTool/py-agent/gerrit-tool/config/gerrit-setup.conf`. Do not store the administrator
password in that file.

Leave `GERRIT_STORAGE_PATH` empty for `@@HOME@@/gerrit-site`, or set it to the
absolute site path supplied by the server administrator. Validate it first:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-storage-check.sh
```

The installer prints the resolved site before changing anything. It rejects a
missing or read-only mount, unsafe broad paths, symbolic escapes, insufficient
space, missing Linux permissions, or failed file-locking and file-operation
checks.

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-setup.sh install
```

The script prompts without echo for the initial `@@ADMIN_USERNAME@@` password
and exports it only to the installer process. The installer verifies downloads,
initializes Gerrit on loopback, validates the local site, backs up the initial
configuration, creates the internal HTTPS certificate, installs Caddy, binds
final services to `@@SERVER_IP@@`, and installs the management commands, CI
programs, UI plugin, and rendered guides.

Start and validate an existing installation with:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-setup.sh start
```

## Installed paths

- Gerrit application: `$HOME/apps/gerrit`
- Gerrit site: `@@GERRIT_SITE@@`
- Authoritative repositories: `@@GERRIT_SITE@@/git`
- Caddy application: `$HOME/apps/caddy`
- Caddy configuration: `$HOME/gerrit-proxy`
- CI programs: `$HOME/apps/gerrit/tools`
- Commands: `$HOME/bin`
- Rendered guides: `@@GERRIT_SITE@@/docs`
- Backups: `$HOME/backups/gerrit`

## Service operation

Use `$HOME/bin/gerrit-start`, `$HOME/bin/gerrit-stop`,
`$HOME/bin/gerrit-restart`, `$HOME/bin/gerrit-status`,
`$HOME/bin/gerrit-logs`, and `$HOME/bin/gerrit-check`. Manual startup after a
server reboot is supported. When `$HOME/.xwalk-ci.env` exists, these lifecycle
and validation controls include the CI worker. Do not create a system-level
service.

Print the installed web address with:

```bash
git config --file "@@GERRIT_SITE@@/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

Continue with the administrator configuration before onboarding users.

## Process management

The generated commands use Gerrit's own control script and the exact validated
site. `nohup` is the portable default. `tmux` or `screen` may retain an
interactive session after logout but cannot guarantee startup after reboot.
Select user-level systemd only when assessment confirms the user manager and
required commands work. No system-level unit is created.
