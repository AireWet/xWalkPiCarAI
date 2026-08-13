# Gerrit local Linux setup

## Purpose

The `local-linux` module provides a temporary Gerrit installation on the
current Linux computer. It uses the same non-root Python installer as the
college-server deployment but keeps the local address and ports in a separate
configuration file.

## Configure and install

Run the assessment:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh assess
```

The local configuration uses assessed Wi-Fi address `192.168.1.158`, HTTPS
port `18443`, and Gerrit SSH port `29419`. Its Gerrit 3.14.2 URL selects
immutable official object generation `1783941312319403`. The downloaded WAR
matched official bucket size and MD5 metadata before its SHA-256 was recorded.
Reassess and update the IP if DHCP changes it. Do not store the administrator
password in this file.

Install and start Gerrit:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh install
```

The script prompts for the password, installs below `$HOME`, starts Gerrit and
Caddy, and validates the installation. After a reboot, use:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh start
```

Caddy runs as a transient user-level systemd service. This keeps the HTTPS
proxy alive after the setup command returns without installing a system
service or requiring root access.

## Browser access

Print the configured HTTPS address:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

For the assessed host, the browser address is
`https://192.168.1.158:18443/`.

Open that address locally after verifying and trusting the generated public
certificate. Access from another LAN computer depends on existing host and
network policy. The module does not modify firewall, router, or DNS settings.

## Deployment separation

Do not treat local testing as college-server validation. The later college
installation uses `config/gerrit-setup.conf` and must be tested independently
through eduVPN. Gerrit data under the local `$HOME/gerrit-site` remains the
local authoritative instance until a reviewed backup and restore migration is
performed.
