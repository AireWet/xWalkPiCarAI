# Gerrit storage and migration

## Ownership model

The selected Gerrit site is `@@GERRIT_SITE@@`. Exactly one Gerrit instance,
running as the college account that owns the site, may use it. Partners use
Gerrit web and SSH access. They must never directly modify `git`, `db`, `index`,
`data`, `cache`, `plugins`, or `etc` below the site.

Use `umask 077`. Do not make the site world-writable or group-writable. An
administrator may optionally create a read-limited Unix group for controlled
backup access. The installer neither creates groups nor grants partners
unrestricted internal access.

## Safe migration plan

The installer never copies or removes an existing site. If it detects the
current home site or an existing destination, it stops and reports both paths.

1. Read the active `GERRIT_SITE` from `$HOME/gerrit-env.sh` and check status.
2. Create and verify a backup with `$HOME/bin/gerrit-backup`.
3. Set the proposed `GERRIT_STORAGE_PATH` and run `gerrit-storage-check.sh`.
4. Confirm the destination is empty, private, mounted, persistent, and owned by the Gerrit account.
5. Stop CI and Gerrit cleanly with their exact user-space controls.
6. Confirm no Gerrit or Caddy process still uses the original site.
7. Copy the stopped site while preserving ownership, permissions, timestamps, links, and sparse files.
8. Point `GERRIT_SITE` in `$HOME/gerrit-env.sh` to the resolved destination.
9. Start one migrated instance and validate status, HTTP, SSH, repositories, identities, and review upload.
10. Keep the original stopped site unchanged until the migrated site and backup are validated.

Ask the administrator for an approved preservation-capable copy command when
the filesystems differ. Do not copy a running site, start both copies, or
delete the original automatically.

## Shared bare Git alternative

If network policy cannot permit Gerrit HTTP and SSH, an administrator may
approve a user-writable shared location for a bare repository:

```bash
git init --bare /shared/path/PROJECT_NAME.git
```

This provides Git collaboration only. It does not provide Gerrit changes,
reviews, approvals, comments, audit votes, or `refs/for/master`.
