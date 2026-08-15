# Add a User to a Gerrit Repository

This guide explains how a Gerrit administrator gives one person access to an existing xWalk repository.
Access is granted through Gerrit groups and project ACLs. Never give a user direct filesystem access to
`$GERRIT_SITE/git`, and never share the administrator or `xwalk-ci` account.

## Required information

Collect these values before making changes:

| Value | Source |
|---|---|
| `GERRIT_SERVER_HOST` | `xWalkTool/py-agent/gerrit-tool/config/gerrit-setup.conf` |
| `GERRIT_SSH_PORT` | `xWalkTool/py-agent/gerrit-tool/config/gerrit-setup.conf` |
| `GERRIT_ADMIN_USER` | `xWalkTool/py-agent/gerrit-tool/config/gerrit-setup.conf` |
| `PROJECT_NAME` | Gerrit project list or the installer configuration |
| `PROJECT_BRANCH` | Protected review branch, normally `main` |
| `USERNAME` | Unique lowercase login selected for the person |
| `USER_EMAIL` | The person's individual email address |

The administrator must already have a registered SSH key and must be able to run this command:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit version
```

Replace every uppercase token in the examples. Do not paste unresolved tokens into a real command.

## Create the web login

Run the installed xWalkTool account helper on the Gerrit server:

```bash
$HOME/bin/gerrit-user-add USERNAME
```

Enter a unique password of at least 12 characters. The helper stores only a bcrypt hash, reloads Caddy, and
rolls back the user-file change if the reload fails. Send the password to the user through an approved private
channel. Do not put it in Git, email, a command argument, or a remote URL.

Ask the user to sign in once through the configured HTTPS URL. The first successful login creates the Gerrit
account associated with that HTTP identity.

## Set the Gerrit identity

After the first sign-in, assign the person's name and email through the administrator SSH connection:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit set-account --full-name 'USER FULL NAME' --add-email USER_EMAIL USERNAME
```

Never assign two people the same username or email. Each account must remain attributable to one person.

## Add the user to an access group

Gerrit repository access comes from groups. Use the smallest group that provides the required permissions:

| Group | Intended access |
|---|---|
| `Developers` | Read and upload patch sets to `refs/for/PROJECT_BRANCH` |
| `Reviewers` | Read, comment, and vote within the configured Code-Review range |
| `Project-Owners` | Project ACL administration, final review, and Submit |
| `Service Users` | Restricted automation accounts such as `xwalk-ci` |

An ordinary contributor normally needs `Developers`. Add `Reviewers` only when the person is authorized to
vote. Do not add a developer to `Project-Owners`, `Service Users`, or Gerrit's `Administrators` group.

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit set-members --add USERNAME Developers
```

If review voting is also required:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit set-members --add USERNAME Reviewers
```

Verify the resulting membership:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit ls-members Developers
```

Open **Projects → List → PROJECT_NAME → Access** and confirm that the selected group has only the intended
permissions. A normal developer may push to `refs/for/refs/heads/PROJECT_BRANCH`; they must not receive direct
Push, Force Push, Create Reference, Submit, or `Verified` voting permission on the protected branch.

## Register the user's SSH key

The user generates a dedicated key on their own computer:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/id_ed25519_xwalk_gerrit -C "USER_EMAIL"
```

The private key remains on that computer. The user signs in to Gerrit, opens **Settings → SSH Keys**, and adds
only `~/.ssh/id_ed25519_xwalk_gerrit.pub`.

Test the connection from the user's computer:

```bash
ssh -p GERRIT_SSH_PORT USERNAME@GERRIT_SERVER_HOST
```

Gerrit should print its welcome message and state that interactive shells are disabled.

## Clone and upload for review

Clone the repository and install Gerrit's `commit-msg` hook:

```bash
git clone ssh://USERNAME@GERRIT_SERVER_HOST:GERRIT_SSH_PORT/PROJECT_NAME
```

```bash
cd PROJECT_NAME && scp -P GERRIT_SSH_PORT USERNAME@GERRIT_SERVER_HOST:hooks/commit-msg .git/hooks/commit-msg && chmod 755 .git/hooks/commit-msg
```

After making and testing a change, upload it for review:

```bash
git push origin HEAD:refs/for/PROJECT_BRANCH
```

The commit message must contain a `Change-Id`. Component changes are uploaded only to Gerrit `main`; they are
never pushed directly to GitHub.

## Verify and revoke access

Confirm that the user can read the intended project and upload a harmless review change, but cannot push
directly to the protected branch or set `Verified`.

When access is no longer required:

1. Remove the user from every project group in Gerrit.
2. Revoke their SSH keys and deactivate the Gerrit account.
3. Disable the proxy login on the Gerrit server.

```bash
$HOME/bin/gerrit-user-disable USERNAME
```

Disabling the web login alone does not revoke existing Gerrit SSH access. Complete all three steps.

See [Gerrit Account Setup Guide](Gerrit%20Account%20Setup%20Guide.md) for the developer-side SSH configuration,
review workflow, voting meanings, and network-access guidance.
