# Gerrit admin setup

## Initial administrator

The first successful login as `@@ADMIN_USERNAME@@` on a new site becomes the
initial administrator. Verify this account before creating other logins. Never
share its password, browser session, or SSH key.

| Administrator field | Configured value |
|---|---|
| Gerrit username | `@@ADMIN_USERNAME@@` |
| Full name | `@@ADMIN_NAME@@` |
| Role | `@@ADMIN_ROLE@@` |
| Email | `@@ADMIN_EMAIL@@` |

The role is descriptive documentation; it does not grant a Gerrit permission.
After the initial administrator signs in and registers their SSH public key,
set the confirmed name and email with:

```bash
ssh -p @@SSH_PORT@@ @@ADMIN_USERNAME@@@@@SERVER_IP@@ gerrit set-account --full-name '@@ADMIN_NAME@@' --add-email @@ADMIN_EMAIL@@ @@ADMIN_USERNAME@@
```

## Individual accounts

Create one web login per person:

```bash
$HOME/bin/gerrit-user-add USERNAME
```

After the user signs in once, assign their unique full name and email:

```bash
ssh -p @@SSH_PORT@@ @@ADMIN_USERNAME@@@@@SERVER_IP@@ gerrit set-account --full-name 'USER FULL NAME' --add-email USER_EMAIL USERNAME
```

Do not copy unresolved source fields into the terminal. During installation,
the Python installer replaces them in
`$HOME/gerrit-site/docs/Gerrit Admin Setup.md` with the configured server
values.

Use these sources for every value:

| Command value | Source |
|---|---|
| Gerrit SSH port | `GERRIT_SSH_PORT` in `config/gerrit-setup.conf` |
| Administrator username | `GERRIT_ADMIN_USER` in `config/gerrit-setup.conf` |
| Gerrit server IP | `EDUVPN_SERVER_IP` in `config/gerrit-setup.conf` |
| `USER FULL NAME` | User's individual preferred full name |
| `USER_EMAIL` | User's individual college email address |
| `USERNAME` | Exact login passed to `gerrit-user-add` |

On the server, confirm the effective Gerrit SSH address with:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get sshd.listenAddress
```

It prints `SERVER_IP:SSH_PORT`. The administrator username remains the value
selected during installation. From the repository checkout, load the
non-secret installer configuration and inspect the three values with:

```bash
. xWalkTool/gerrit/config/gerrit-setup.conf && printf 'administrator=%s\nserver=%s\nssh-port=%s\n' "$GERRIT_ADMIN_USER" "$EDUVPN_SERVER_IP" "$GERRIT_SSH_PORT"
```

The administrator must first sign in to Gerrit and register their own SSH
public key under **Settings → SSH Keys**. Verify administrator SSH access before
changing another account:

```bash
ssh -p "$GERRIT_SSH_PORT" "$GERRIT_ADMIN_USER@$EDUVPN_SERVER_IP" gerrit version
```

For example, after creating login `partner1`, ask that user to sign in once and
confirm their own full name and college email. Then run:

```bash
ssh -p "$GERRIT_SSH_PORT" "$GERRIT_ADMIN_USER@$EDUVPN_SERVER_IP" gerrit set-account --full-name 'Partner Full Name' --add-email partner1@student.example partner1
```

Never assign two people the same username or email identity.

To remove web login access, run:

```bash
$HOME/bin/gerrit-user-disable USERNAME
```

Then deactivate the Gerrit account, remove it from groups, and revoke its SSH
keys in the administrator UI. Removing the proxy login alone does not remove
existing Gerrit SSH access.

## Groups

Create these Gerrit groups and add only named individual accounts:

| Group | Responsibility |
|---|---|
| `Project-Owners` | Project administration, review, and Submit |
| `Developers` | Read and upload changes for review |
| `Reviewers` | Read, comment, and vote Code-Review |
| `Service Users` | Restricted CI service identity |

Do not grant project developers `Administrate Server`.

## Project and permissions

Create `@@PROJECT_NAME@@` with initial branch `@@PROJECT_BRANCH@@`. Configure
the project access page as follows:

| Reference | Group | Permission |
|---|---|---|
| `refs/heads/*` | Anonymous Users | Read |
| `refs/for/refs/heads/*` | Developers | Push |
| `refs/heads/*` | Reviewers | Code-Review at the intended range |
| `refs/heads/*` | Project-Owners | Submit and project ownership |
| `refs/heads/*` | Service Users | Verified `-1..+1` only |

Do not grant Anonymous Users or ordinary users direct Push, Force Push, Create
Reference, or Submit on `refs/heads/@@PROJECT_BRANCH@@`. Inspect effective
permissions before accepting work.

## Account maintenance

Periodically review group membership, registered SSH keys, email identities,
and project permissions. Revoke a compromised device key without removing a
user's other valid device keys. Transfer project ownership through group
membership, not by sharing the administrator account.
