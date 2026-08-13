# Gerrit server setup

## Installed architecture

Gerrit @@GERRIT_VERSION@@ runs as the normal Linux user from
`$HOME/gerrit-site`. Its authoritative repositories are in
`$HOME/gerrit-site/git`, not on either developer laptop. Java is
@@JAVA_VERSION@@. The service binds HTTPS to
`@@SERVER_IP@@:@@HTTPS_PORT@@` on `@@INTERFACE@@` and Gerrit SSH to
`@@SERVER_IP@@:@@SSH_PORT@@`.

- Web URL: `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`
- Authentication: college LDAP through `@@LDAP_SERVER@@`
- TLS mode: `@@TLS_MODE@@`
- Gerrit download: `@@GERRIT_URL@@`
- Gerrit SHA-256: `@@GERRIT_SHA256@@`
- Installation date: `@@INSTALL_DATE@@`
- Java home: `@@JAVA_HOME@@`
- Java source: `@@JAVA_SOURCE@@`
- Java archive SHA-256: `@@JAVA_SHA256@@`
- Server certificate fingerprint: `@@TLS_FINGERPRINT@@`
- Project to create after administrator bootstrap: `@@PROJECT_NAME@@`

No root privilege, system package manager, firewall change, system SSH change,
system proxy, system service, Docker, or development impersonation
authentication is used.

## Operation

Use `$HOME/bin/gerrit-start`, `gerrit-stop`, `gerrit-restart`, `gerrit-status`,
`gerrit-logs`, `gerrit-check`, and `gerrit-backup`. Manual restart after a host
reboot is supported. User systemd may be adopted only if the college enables
persistent user sessions; do not create a system unit.

Logs are under `$HOME/gerrit-site/logs`. Host configuration is in
`$HOME/gerrit-site/etc`, and sensitive files must retain mode `0600`. The
keystore password is stored only in `etc/secure.config`, not in this guide or
Git.

## Find and share the web address

Print the configured Gerrit web address on the server:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

It must print `https://@@SERVER_IP@@:@@HTTPS_PORT@@/`. This is the address to
share with authorised college users. Remote users must not use plain HTTP; the
temporary `http://127.0.0.1:18080/` address was only for local initialization.

The direct page listing changes visible to the signed-in user for this project
is:

```text
https://@@SERVER_IP@@:@@HTTPS_PORT@@/q/project:@@PROJECT_NAME@@
```

Before sharing the address, verify that Gerrit is bound to the selected college
interface with `$HOME/bin/gerrit-check`. Each user must connect to eduVPN,
validate the TLS certificate, and sign in using an individual LDAP account.
The URL being reachable does not itself grant repository access.

On the project's **Access** page, grant **Read** to `Project-Owners`,
`Developers`, and `Reviewers`. Remove inherited **Anonymous Users: Read** when
the project must require authentication. Grant **Registered Users: Read** only
when every authenticated college account is intended to see the project.

An authorised user must test from a different computer connected through
eduVPN:

```bash
nc -vz @@SERVER_IP@@ @@HTTPS_PORT@@
```

They must then open `https://@@SERVER_IP@@:@@HTTPS_PORT@@/` in a browser and
confirm that TLS validation and individual login succeed. Until this test is
complete, the installation is only partially configured for remote access.

## Administrator bootstrap

The first successful approved LDAP login to a new Gerrit site becomes the
initial administrator. Verify this before onboarding anyone else. Create
`Project-Owners`, `Developers`, and `Reviewers` in the Gerrit group UI. Add
named individual accounts only.

Create `@@PROJECT_NAME@@` with initial branch `main`. Grant Developers Read and
push to `refs/for/refs/heads/*`; grant Reviewers Code-Review; grant
Project-Owners project administration and Submit. Do not grant ordinary users
Push, Force Push, Create Reference, or Submit on `refs/heads/main`. Inspect the
effective access page before accepting work.

Each user signs in through HTTPS and uploads their own locally generated public
SSH key under **Settings → SSH Keys**. Revoke keys individually from that page.
Disable departed accounts through the administrator account API/UI and remove
them from all project groups. Never share accounts or keys.

## Workflow

Clone with
`git clone ssh://USERNAME@@@SERVER_IP@@:@@SSH_PORT@@/@@PROJECT_NAME@@`.
Install the hook with:

```bash
scp -P @@SSH_PORT@@ USERNAME@@@SERVER_IP@@:hooks/commit-msg .git/hooks/commit-msg && chmod +x .git/hooks/commit-msg
```

Upload with `git push origin HEAD:refs/for/main`. Amend while retaining the
`Change-Id` to create a new patch set. Reviewers vote through **Reply**;
authorised owners submit only after requirements pass.

## TLS

For an approved keystore, deploy its full trust chain and renew it before
expiry. For self-signed testing, distribute only
`$HOME/gerrit-site/etc/gerrit-self-signed.crt` over an authenticated channel,
compare its SHA-256 fingerprint out of band, and import it into each user's
trust store. Never distribute the keystore, private key, or password, and never
disable TLS verification.

## Backup and restore

Run `$HOME/bin/gerrit-backup`. It checks free space, stops Gerrit only when
needed, archives repositories, NoteDb configuration, data, indexes, plugins,
host keys, TLS configuration, WAR, and version environment, validates the
archive, emits a SHA-256 file, and restarts a previously running service. It
never removes older backups.

For restore, stop Gerrit and preserve the damaged `$HOME/gerrit-site` by
renaming it to a timestamped path. Verify the archive with `sha256sum -c`, list
it with `tar -tzf`, and extract into a new private temporary directory. Confirm
paths remain below that directory before copying reviewed content into place.
Restore owner-only permissions on `etc/secure.config`, the keystore, and SSH
host private keys. Use the recorded Java and Gerrit versions. Run Gerrit's
documented offline reindex command if indexes are absent or incompatible,
start Gerrit, run `gerrit-check`, and verify repositories and identities. Do
not perform a destructive restore without a separate approval.

## Upgrade and rollback

Create and verify a backup, stop Gerrit, verify the new official WAR checksum,
and follow that release's upgrade notes and plugin compatibility requirements.
Preserve the old WAR. On failure, stop the process and restore the complete
pre-upgrade backup rather than mixing schemas or plugin versions.

## Remote validation

From each computer while independently connected to eduVPN, run
`nc -vz @@SERVER_IP@@ @@HTTPS_PORT@@`,
`nc -vz @@SERVER_IP@@ @@SSH_PORT@@`, a certificate-verifying
`curl -v https://@@SERVER_IP@@:@@HTTPS_PORT@@/`, and
`ssh -v -p @@SSH_PORT@@ USERNAME@@@SERVER_IP@@`. Verify distinct logins, keys,
review attribution, upload to `refs/for/main`, and rejection of direct push to
`refs/heads/main`.

If local access works but eduVPN access fails, ask college IT to permit TCP
@@HTTPS_PORT@@ and @@SSH_PORT@@ to host `@@SERVER_IP@@` only from authorised
eduVPN subnets. Also request an approved internal hostname/certificate and LDAP
details when unavailable. Do not bypass routing, firewall, certificate, or
identity controls.

## Security limitations and uninstall

Readiness cannot be claimed until both computers pass HTTPS and SSH tests
through eduVPN. Embedded Gerrit TLS is suitable for this constrained internal
deployment but has fewer controls than a maintained proxy. Email is disabled
unless the college supplies an approved SMTP relay, so LDAP remains
authoritative for email identity.

To uninstall safely, stop Gerrit, create and verify a final backup, and move
`$HOME/gerrit-site`, `$HOME/apps/gerrit`, `$HOME/gerrit-env.sh`, and the Gerrit
management commands to a dated quarantine directory. Do not delete repositories
or backups automatically.
