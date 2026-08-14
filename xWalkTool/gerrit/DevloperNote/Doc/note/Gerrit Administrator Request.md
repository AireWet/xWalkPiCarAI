# Gerrit administrator request

Please provide or confirm the following without granting root access to the
Gerrit operator:

- an absolute persistent storage path owned and writable by the Gerrit service account;
- at least 10 GiB free and quota growth suitable for repositories, indexes, and backups;
- Linux ownership, restrictive permissions, directories, regular files, and file locking;
- Java 21 and commands reported missing by `gerrit-setup.sh assess`;
- the assigned VPN-facing server host or IPv4 address;
- TCP @@HTTPS_PORT@@ for HTTPS review access from the approved eduVPN subnet;
- TCP @@SSH_PORT@@ for Gerrit SSH from the approved eduVPN subnet;
- confirmation that college login SSH and Gerrit SSH are separate services;
- optional user-level systemd availability when user-service management is desired.

Requested host: `@@SERVER_IP@@`

Requested Gerrit site: `@@GERRIT_SITE@@`

Requested endpoints: `@@CANONICAL_WEB_URL@@` and `@@SSH_LISTEN_ADDRESS@@`

No public exposure, firewall bypass, system user, system service, system
package installation, or ownership change by the installer is requested. If
shared storage is unavailable, the safe fallback is
`@@HOME@@/gerrit-site`.
