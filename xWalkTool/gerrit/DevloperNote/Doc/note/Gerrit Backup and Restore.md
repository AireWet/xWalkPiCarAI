# Gerrit backup and restore

## Backup

Run `$HOME/bin/gerrit-backup`. The command checks free space, stops Gerrit and
CI only when required for consistency, creates a timestamped archive below
`$HOME/backups/gerrit`, verifies the archive, writes its SHA-256 checksum, and
restarts services that were previously running. It never deletes older
backups.

The command reads the active site from `$HOME/gerrit-env.sh`, so shared storage
is supported. Backups remain outside the active site under
`$HOME/backups/gerrit`.

The archive includes repositories, NoteDb data, configuration, indexes,
plugins, host keys, HTTPS configuration, Caddy configuration and password
hashes, the Gerrit WAR, installed CI/UI assets, retained CI state and logs, and
recorded version information. User private SSH keys must not exist in Gerrit
data and are not backup content.

## Restore

Do not perform a destructive restore without explicit approval.

1. Stop Gerrit and CI.
2. Preserve the damaged `@@GERRIT_SITE@@` as a timestamped quarantine path.
3. Verify the selected archive with `sha256sum -c`.
4. Inspect it with `tar -tzf`.
5. Extract into a new private temporary directory.
6. Confirm every extracted path remains below that directory.
7. Restore reviewed repositories, configuration, data, plugins, and tools.
8. Restore owner-only permissions on HTTPS and SSH host private keys.
9. Use the recorded Java and Gerrit versions.
10. Run Gerrit's documented offline reindex command when required.
11. Start Gerrit, run `$HOME/bin/gerrit-check`, and validate identities and repositories.

## Upgrade and rollback

Before an upgrade, create and verify a backup, verify the official new WAR
checksum, read the selected release's upgrade notes, and confirm plugin
compatibility. Preserve the old WAR. On failure, restore the complete
pre-upgrade backup instead of mixing database schemas or plugin versions.

## Safe uninstall

Stop services, create a final verified backup, and move the Gerrit site,
applications, proxy configuration, environment, and management commands to a
dated quarantine directory. Never delete repositories or backups
automatically.
