# Gerrit troubleshooting

## Service state

Run `$HOME/bin/gerrit-status`, `$HOME/bin/gerrit-check`, and
`$HOME/bin/gerrit-logs`. Confirm Gerrit and Caddy run as the expected non-root
account and listen on the configured addresses.

## Browser access

Print the canonical URL with:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

If local HTTPS works but a remote browser cannot connect, verify eduVPN,
`@@SERVER_IP@@`, ports @@HTTPS_PORT@@ and @@SSH_PORT@@, and the selected network
interface. A college firewall or route requires college IT action.

## Login and permissions

If login fails, confirm the username exists in the private Caddy user file and
restart or reload Caddy with the owner-checked control command. If voting or
Submit is absent, inspect the user's group membership, label range, current
patch-set requirements, and project access inheritance. Submit intentionally
appears only after Code-Review and Verified requirements pass.

## SSH access

Confirm the user uploaded only their public key, is using their Gerrit username
rather than assuming the Linux username, and connects to port @@SSH_PORT@@.
Review the user's registered key fingerprints and revoke obsolete keys.

## CI activation and logs

An active upload triggers CI automatically. A WIP upload triggers only after
**Activate** changes the current patch set to active. Check
`$HOME/bin/gerrit-ci-control status`, `$HOME/bin/gerrit-ci-logs`, the Gerrit
event-stream account permissions, and `https://@@SERVER_IP@@:@@HTTPS_PORT@@/ci/`.

If a quality tool is missing, ask the server administrator to provide the
approved dependency. Do not install system packages without authorization.
