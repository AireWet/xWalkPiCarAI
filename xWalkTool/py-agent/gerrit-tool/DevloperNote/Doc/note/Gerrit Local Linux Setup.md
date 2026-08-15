# Gerrit local Linux setup

## Purpose

The `local-linux` module provides a temporary Gerrit installation on the
current Linux computer. It uses the same non-root Python installer as the
college-server deployment but keeps the local address and ports in a separate
configuration file.

## Configure and install

Run the assessment:

```bash
xWalkTool/py-agent/gerrit-tool/local-linux/gerrit-local.sh assess
```

The local configuration requires an assessed Wi-Fi or Ethernet address, HTTPS
port `18443`, and Gerrit SSH port `29419`. Its Gerrit 3.14.2 URL selects
immutable official object generation `1783941312319403`. The downloaded WAR
matched official bucket size and MD5 metadata before its SHA-256 was recorded.
Reassess and update the IP if DHCP changes it. Do not store the administrator
password in this file.

Install and start Gerrit:

```bash
xWalkTool/py-agent/gerrit-tool/local-linux/gerrit-local.sh install
```

The script prompts for the password, installs below `$HOME`, starts Gerrit and
Caddy, starts CI when `$HOME/.xwalk-ci.env` exists, and validates the
installation. After a reboot, use:

```bash
xWalkTool/py-agent/gerrit-tool/local-linux/gerrit-local.sh start
```

This command recovers Gerrit, Caddy, and the configured CI worker as one
lifecycle. The matching status, stop, and restart controls also include CI.

Caddy and the configured CI worker run as transient user-level systemd
services. This keeps both processes alive after the setup command returns
without installing a system service or requiring root access.

## Browser access

Print the configured HTTPS address:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

The printed browser address contains the assessed host and configured HTTPS port.

Open that address locally after verifying and trusting the generated public
certificate. Access from another LAN computer depends on existing host and
network policy. The module does not modify firewall, router, or DNS settings.

## Local Download and review workflow

Sign in at `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`, open a change, and select
**Download**. Choose SSH or authenticated HTTPS. Gerrit substitutes the selected
patch set's real `refs/changes/...` ref for the example below:

```bash
CHANGE_REF='refs/changes/NN/CHANGE_NUMBER/PATCH_SET'
```

Use SSH to fetch and check out the patch set:

```bash
git fetch ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git checkout FETCH_HEAD
```

Use SSH to fetch and cherry-pick the patch set:

```bash
git fetch ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git cherry-pick FETCH_HEAD
```

Clone or upload a new review through SSH:

```bash
git clone ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@
```

```bash
git push ssh://your_gerrit_username@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@ HEAD:refs/for/@@PROJECT_BRANCH@@
```

The equivalent authenticated HTTPS commands are:

```bash
git fetch https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git checkout FETCH_HEAD
```

```bash
git fetch https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@ "$CHANGE_REF" && git cherry-pick FETCH_HEAD
```

```bash
git clone https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@
```

```bash
git push https://your_gerrit_username@@@SERVER_IP@@:@@HTTPS_PORT@@/@@PROJECT_NAME@@ HEAD:refs/for/@@PROJECT_BRANCH@@
```

HTTPS prompts for the user's individual password. Do not place the password in
the URL or disable certificate verification. Select **Patch File → Zip** when a
Git checkout is not required. The push commands upload to Gerrit review and do
not directly update the protected branch.

## Deployment separation

Do not treat local testing as college-server validation. The later college
installation uses `config/gerrit-setup.conf` and must be tested independently
through eduVPN. Gerrit data under the local `$HOME/gerrit-site` remains the
local authoritative instance until a reviewed backup and restore migration is
performed.
