# Gerrit Account Setup Guide

This guide describes how a developer or thesis partner receives an account on the xWalk Gerrit server,
registers a separate SSH key, checks out the project, and uploads changes for review. The current project is
`xWalkPiCarAI`. Its Gerrit web interface uses port `8080`, and Git-over-SSH uses port `29418`.

## Before creating an account

The Gerrit server must be running and reachable from the developer's network. A local hostname such as
`aireWet` normally works only on the same network. A developer working from another home or Wi-Fi network
must first connect through an approved campus VPN or a private overlay network such as Tailscale.

Do not expose Gerrit directly to the public internet without access control, firewall rules, HTTPS, backups,
and operating-system security updates.

Each person must use an individual Gerrit account. Do not share the administrator account, the `xwalk-ci`
service account, or an SSH private key.

## Administrator account creation

A Gerrit administrator creates the account from a machine that already has administrative SSH access. Replace
the example name, email address, and username with the developer's real details:

```bash
ssh gerrit-airewet gerrit create-account --full-name "Developer Name" --email "developer@example.com" developer
```

Use a short, stable username without spaces. The username is used in the SSH remote URL and should not be
changed after repositories have been configured.

The administrator then adds the developer to the ordinary reviewer group selected for the project. Grant only
the permissions required to read the repository, upload patch sets, and vote. Do not add ordinary developers
to `Administrators` or `Service Users`.

## Create a dedicated developer SSH key

Run this command on the developer's own computer:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_xwalk_gerrit -C "developer@example.com"
```

Use a passphrase when practical. This creates:

- `~/.ssh/id_ed25519_xwalk_gerrit`: the private key, which must remain private;
- `~/.ssh/id_ed25519_xwalk_gerrit.pub`: the public key, which may be registered in Gerrit.

Never copy the private key into this repository, email, chat, shared storage, a container image, or a thesis
document. Keep its permissions restricted:

```bash
chmod 600 ~/.ssh/id_ed25519_xwalk_gerrit
```

## Register the public key

When the developer can sign in to the web interface, open:

```text
http://aireWet:8080/settings/#SSHKeys
```

Select **Add Key**, paste the complete contents of `id_ed25519_xwalk_gerrit.pub`, and save it. The public key
starts with `ssh-ed25519` and normally ends with the developer's email comment.

If initial web sign-in is unavailable, send only the `.pub` file to the Gerrit administrator. The administrator
can register it through the Gerrit SSH command:

```bash
ssh gerrit-airewet gerrit set-account --add-ssh-key - developer < ~/.ssh/id_ed25519_xwalk_gerrit.pub
```

The path in this administrator command must point to the developer's public key copy, never the private key.

## Configure the SSH alias

Add an entry to the developer's `~/.ssh/config` file:

```sshconfig
Host gerrit-airewet
    HostName aireWet
    User developer
    Port 29418
    IdentityFile ~/.ssh/id_ed25519_xwalk_gerrit
    IdentitiesOnly yes
```

When connecting through a VPN or private overlay, replace `aireWet` with the approved private DNS name or IP
address. Do not put a password or private-key contents in the SSH configuration.

Test the account:

```bash
ssh -T gerrit-airewet
```

Gerrit should display a welcome message and explain that interactive shells are disabled. That response means
SSH authentication succeeded.

## Configure the repository

For a new checkout, clone through the SSH alias:

```bash
git clone ssh://gerrit-airewet/xWalkPiCarAI
```

For an existing GitHub checkout, add Gerrit as a separate remote:

```bash
git remote add gerrit ssh://gerrit-airewet/xWalkPiCarAI
```

If a `gerrit` remote already exists, inspect it before changing anything:

```bash
git remote -v
```

Install Gerrit's `commit-msg` hook from the repository root. The hook adds the `Change-Id` required to update
one review with later patch sets:

```bash
scp -p gerrit-airewet:hooks/commit-msg .git/hooks/commit-msg
chmod 755 .git/hooks/commit-msg
```

## Upload a change for review

Create a branch from the latest Gerrit `master`, make the change, run the relevant host verification, and
commit it:

```bash
git fetch gerrit master
git switch --create feature-name gerrit/master
git add <files>
git commit
```

Confirm the commit message contains a `Change-Id`, then upload it:

```bash
git push gerrit HEAD:refs/for/master
```

Do not push the review commit directly to GitHub before Gerrit review and verification are complete. Uploading
a new commit with the same `Change-Id` creates another patch set in the existing review.

After an approved and verified change is submitted, the Gerrit CI service automatically fast-forwards GitHub
`master` to the submitted Gerrit branch. Developers must not manually repeat that push. The mirror refuses
non-fast-forward updates so it cannot overwrite an independently changed GitHub history.

## Review and verification labels

The project uses these Code-Review meanings:

| Vote | Meaning |
|---:|---|
| `+2` | Ready to submit after required verification passes |
| `+1` | Code looks good, but another reviewer is required |
| `-1` | A bug or concern should be corrected |
| `-2` | Strong review block; the change must not be submitted in its current form |

A `-2` vote does not abandon a change automatically. Use Gerrit's **Abandon** action when the owner and
reviewers decide the change should no longer proceed.

The dedicated `xwalk-ci` account automatically runs the complete configured host build and CTest suite for
each new patch set on `master`:

- `Verified +1` means all configured host tests passed.
- `Verified -1` means configuration, compilation, or at least one host test failed.

Human reviewer accounts do not have permission to set the `Verified` label on normal branches. Verification
is owned exclusively by the CI service. Reviewers use only the `Code-Review` label and submit action.

Raspberry Pi hardware tests, physical movement, live provider calls, and other opt-in tests are not executed by
the automatic runner. They require the correct hardware, a secured robot, and explicit approval.

## Account maintenance

Remove a lost or retired public key immediately in **Settings > SSH Keys**. Generate and register a replacement
key instead of copying another user's key. Ask a Gerrit administrator to deactivate accounts that are no longer
needed and periodically review group membership and project permissions.

The `xwalk-ci` key is a host-mounted service credential used only by the verification container. It must remain
separate from all developer accounts and must never be committed to Git.
