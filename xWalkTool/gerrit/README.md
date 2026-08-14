# Gerrit installer

This directory provides the non-root Gerrit installer for the college server.
Run it as the normal Linux user. It installs only below `$HOME` and does not
use `sudo`, a system package manager, a system service, or a network tunnel.

## Installer layout

- `shell-script/gerrit-setup.sh`: installer entry point;
- `py-src/xWalkGerritServerSetup.py`: assessment and installation logic;
- `bin/`: management commands installed into `$HOME/bin`;
- `config/gerrit-setup.conf`: Gerrit installer configuration;
- `local-linux/`: separate configuration and entry point for the current Linux host;
- `DevloperNote/Doc/note/`: configuration guides rendered into the site;
- `py-test/`: installer and component host tests.

## Required configuration

Edit `config/gerrit-setup.conf` before installation:

```bash
export EDUVPN_SERVER_IP="SERVER_IP_FROM_ASSESSMENT"
export GERRIT_SHA256="OFFICIAL_GERRIT_WAR_SHA256"
export GERRIT_ADMIN_USER="joxy"
export GERRIT_ADMIN_NAME="Joxy John"
export GERRIT_ADMIN_ROLE="Student"
export GERRIT_ADMIN_EMAIL="joxjoh24@student.hh.se"
export GERRIT_PROJECT="xWalkPiCarAI"
export GERRIT_BRANCH="master"
export GERRIT_HTTPS_PORT="18443"
export GERRIT_SSH_PORT="29418"
export GERRIT_HTTP_PORT="18080"
```

Replace the first two placeholders with:

- the assigned college-server IPv4 address reachable through eduVPN;
- the official Gerrit WAR SHA-256 checksum.

Change the administrator, project, branch, or ports in the same file when the
deployment requires different values. The installer prompts separately for a
unique administrator password of at least 12 characters.

Do not add the administrator password to the configuration file. The setup
script reads it without echo and exports `GERRIT_ADMIN_PASSWORD` only to the
Python installer process. Do not place passwords, private keys, or tokens in
this repository.

## Assess the server

Run the read-only assessment from the repository root:

```bash
xWalkTool/gerrit/shell-script/gerrit-setup.sh assess
```

Confirm the persistent home filesystem, disk quota, Java compatibility,
physical network interface, selected server IP, and free ports. The default
ports are HTTPS `18443`, Gerrit SSH `29418`, and loopback HTTP `18080`.

## Install and start Gerrit

After updating `config/gerrit-setup.conf`, run:

```bash
xWalkTool/gerrit/shell-script/gerrit-setup.sh install
```

The script prompts for the initial `joxy` password, exports it for the Python
installer, and removes it from its environment afterward. The installer
verifies downloads, initializes Gerrit on loopback, creates an initial backup,
configures HTTPS, binds Gerrit SSH to the selected address, starts Gerrit and
Caddy, and runs the installation checks.

## Start an installed server

After a logout or server reboot, run:

```bash
xWalkTool/gerrit/shell-script/gerrit-setup.sh start
```

This starts Gerrit only when it is stopped and always runs `gerrit-check`.

## Download button

The installer adds Gerrit's official `download-commands` plugin and configures
the change **Download** button for SSH and authenticated HTTP. After signing in,
select the required protocol and copy one of these generated options:

- **Checkout**: `git fetch` followed by `git checkout FETCH_HEAD`;
- **Cherry Pick**: `git fetch` followed by `git cherry-pick FETCH_HEAD`;
- **Clone**: clone the project through SSH or authenticated HTTPS;
- **Patch File → Zip**: download the selected patch set as a zipped patch.

Gerrit replaces the example change ref below with the selected patch set's real
`refs/changes/...` value:

```bash
CHANGE_REF='refs/changes/NN/CHANGE_NUMBER/PATCH_SET'
```

```bash
git fetch ssh://USERNAME@SERVER_IP:SSH_PORT/PROJECT "$CHANGE_REF" && git checkout FETCH_HEAD
```

```bash
git fetch ssh://USERNAME@SERVER_IP:SSH_PORT/PROJECT "$CHANGE_REF" && git cherry-pick FETCH_HEAD
```

```bash
git clone ssh://USERNAME@SERVER_IP:SSH_PORT/PROJECT
```

```bash
git push ssh://USERNAME@SERVER_IP:SSH_PORT/PROJECT HEAD:refs/for/BRANCH
```

```bash
git fetch https://USERNAME@SERVER_IP:HTTPS_PORT/PROJECT "$CHANGE_REF" && git checkout FETCH_HEAD
```

```bash
git fetch https://USERNAME@SERVER_IP:HTTPS_PORT/PROJECT "$CHANGE_REF" && git cherry-pick FETCH_HEAD
```

```bash
git clone https://USERNAME@SERVER_IP:HTTPS_PORT/PROJECT
```

```bash
git push https://USERNAME@SERVER_IP:HTTPS_PORT/PROJECT HEAD:refs/for/BRANCH
```

HTTPS commands prompt for the user's individual password. Never embed the
password in a URL or disable TLS certificate verification. Both push commands
upload a change for review; they do not push directly to the protected branch.

## Installed paths

| Content | Installed path |
|---|---|
| Gerrit application | `$HOME/apps/gerrit` |
| Gerrit site | `$HOME/gerrit-site` |
| Authoritative repositories | `$HOME/gerrit-site/git` |
| Caddy application | `$HOME/apps/caddy` |
| Caddy configuration | `$HOME/gerrit-proxy` |
| Management commands | `$HOME/bin` |
| Rendered guides | `$HOME/gerrit-site/docs` |
| Backups | `$HOME/backups/gerrit` |

Print the installed HTTPS address with:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

Continue with `$HOME/gerrit-site/docs/Gerrit Admin Setup.md`
after the installer completes.

## Local Gerrit

The `local-linux` profile provides a separate Gerrit deployment for the current
Linux computer. It reuses the non-root installer but does not reuse the college
server configuration. Its configuration is stored in
`local-linux/gerrit-local.conf`.

For the assessed local host, use:

- web review: `https://192.168.1.158:18443/`;
- Gerrit SSH: `192.168.1.158:29419`;
- project: `xWalkPiCarAI`;
- review branch: `master`.

Assess, install, and later restart the local instance with:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh assess
```

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh install
```

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh start
```

Reassess the host and update `GERRIT_SERVER_IP` if its Wi-Fi or Ethernet address
changes. The local Gerrit **Download** button provides separate SSH and HTTPS
commands for clone, fetch and checkout, fetch and cherry-pick, and push for
review. It also provides a zipped patch file.

See [`local-linux/README.md`](local-linux/README.md) for the exact local Git
commands and
[`Gerrit Local Linux Setup.md`](DevloperNote/Doc/note/Gerrit%20Local%20Linux%20Setup.md)
for the administrator workflow and deployment limitations.
