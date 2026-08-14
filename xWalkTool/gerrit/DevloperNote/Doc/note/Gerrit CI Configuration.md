# Gerrit CI configuration

## Execution model

CI runs directly as the Gerrit server's normal Linux user. It does not use a
container or separate service host. The ThreadSanitizer job uses `unshare` to
create a disposable non-root user, PID, and network namespace. Its private
loopback interface remains available to the streaming host tests, while the
isolated address space prevents host mappings from blocking TSan startup. The
server must provide `unshare` and `ip`; no system configuration is changed.

An active `patchset-created` event starts
verification; a WIP patch set waits until **Activate** emits a
`wip-state-changed` event. The complete quality matrix runs before one
aggregate `Verified +1` or `Verified -1` vote is reported.

A `change-merged` event retains the owner-aware GitHub publication flow. Only
the dedicated service process may use the repository-scoped mirror key.

## Service account

Create an individual `xwalk-ci` Gerrit account, register only its public SSH
key, add it to `Service Users`, and grant only `Verified -1..+1` for
`@@PROJECT_NAME@@`. Do not grant Submit or server administration.

Keep the Gerrit and GitHub private keys on the server outside Git with mode
`0600`. Never reuse a developer key.

## Environment

Copy the rendered environment example and protect it:

```bash
cp "@@GERRIT_SITE@@/docs/XWALK_CI_ENV.example" "$HOME/.xwalk-ci.env" && chmod 600 "$HOME/.xwalk-ci.env"
```

Review every endpoint and replace the two private-key paths if required. The
server must already provide the host build and quality dependencies used by
the repository.

## Operation and logs

Start and inspect CI with:

```bash
$HOME/bin/gerrit-ci-control start
```

```bash
$HOME/bin/gerrit-ci-control status
```

```bash
$HOME/bin/gerrit-ci-logs
```

CI state is retained in `$HOME/gerrit-ci/state`, logs in
`$HOME/gerrit-ci/logs`, and the loopback-only log server is published by Caddy
at `https://@@SERVER_IP@@:@@HTTPS_PORT@@/ci/`.

Validate the dashboard through HTTPS:

```bash
curl --cacert "@@GERRIT_SITE@@/etc/gerrit-self-signed.crt" https://@@SERVER_IP@@:@@HTTPS_PORT@@/ci/health
```
