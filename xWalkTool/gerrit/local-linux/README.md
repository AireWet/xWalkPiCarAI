# Local Linux Gerrit

This module runs the shared non-root Gerrit installer on the current Linux
computer until the college server deployment is available. It reuses the same
installer, management commands, authentication, HTTPS, CI, and UI components;
it does not create a second implementation.

## Assess

Run from the repository root:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh assess
```

Select the assigned IPv4 address of the physical Ethernet or Wi-Fi interface.
Do not use `127.0.0.1`, a container bridge, a virtual-machine-only interface,
or a guessed address. The selected address must appear in the assessment.

## Configure

The checked local configuration records the assessed Wi-Fi address and the
immutable official Gerrit object generation:

```bash
export GERRIT_SERVER_IP="192.168.1.158"
export GERRIT_URL="https://storage.googleapis.com/download/storage/v1/b/gerrit-releases/o/gerrit-3.14.2.war?generation=1783941312319403&alt=media"
export GERRIT_SHA256="3ae33de96f7efb640a0f63b62309330e3981682f448ffe9178763f470ba3ba7a"
```

The SHA-256 was computed after the immutable download matched the official
bucket size and MD5 metadata and reported Gerrit version 3.14.2. Reassess and
update the address if the local DHCP assignment changes. Keep the
administrator password out of this file and out of Git. Local Gerrit SSH uses
`29419` to avoid collision with the system Gerrit service.

## Install and start

Run:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh install
```

The script securely prompts for the initial `joxy` password, installs below
`$HOME`, starts Gerrit and Caddy, and validates HTTPS. It never uses `sudo`.
The local profile runs Caddy as a transient user-level systemd service so it
continues after the installer exits; it does not create a system service.

After a reboot, start the existing installation with:

```bash
xWalkTool/gerrit/local-linux/gerrit-local.sh start
```

Print the browser address with:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

For the assessed host, this prints `https://192.168.1.158:18443/`.

Trust only the generated public certificate after checking its fingerprint.
Never distribute `$HOME/gerrit-site/etc/gerrit-self-signed.key`.

## Local Download button

Sign in to the local Gerrit page, open a change and select **Download**. Choose
SSH or HTTP to copy **Checkout**, **Cherry Pick**, or **Clone**. Use
**Patch File → Zip** to download a zipped patch. Gerrit inserts the selected patch set's
real change ref. Copy the exact value displayed for the selected patch set:

```bash
CHANGE_REF='refs/changes/NN/CHANGE_NUMBER/PATCH_SET'
```

SSH Fetch & Checkout:

```bash
git fetch ssh://joxy@192.168.1.158:29419/xWalkPiCarAI "$CHANGE_REF" && git checkout FETCH_HEAD
```

SSH Fetch & Cherry Pick:

```bash
git fetch ssh://joxy@192.168.1.158:29419/xWalkPiCarAI "$CHANGE_REF" && git cherry-pick FETCH_HEAD
```

SSH Clone:

```bash
git clone ssh://joxy@192.168.1.158:29419/xWalkPiCarAI
```

SSH Push for Review:

```bash
git push ssh://joxy@192.168.1.158:29419/xWalkPiCarAI HEAD:refs/for/master
```

HTTP Fetch & Checkout:

```bash
git fetch https://joxy@192.168.1.158:18443/xWalkPiCarAI "$CHANGE_REF" && git checkout FETCH_HEAD
```

HTTP Fetch & Cherry Pick:

```bash
git fetch https://joxy@192.168.1.158:18443/xWalkPiCarAI "$CHANGE_REF" && git cherry-pick FETCH_HEAD
```

HTTP Clone:

```bash
git clone https://joxy@192.168.1.158:18443/xWalkPiCarAI
```

HTTP Push for Review:

```bash
git push https://joxy@192.168.1.158:18443/xWalkPiCarAI HEAD:refs/for/master
```

HTTP Git prompts for the individual local Gerrit password. Keep the trusted
self-signed certificate enabled and never place the password in the URL. Both
push commands create or update a Gerrit review and cannot bypass protected
`master`.

## Access boundary

The URL is reachable only where the local Linux host and network allow the
configured IP and port. This module does not change a firewall, router, DNS,
or system network configuration. If another local computer cannot connect,
inspect existing network policy or ask its administrator; do not bypass it.

The local installation is not proof that the later college-server deployment
is reachable through eduVPN. Test that deployment separately from another
computer before classifying it as ready.
