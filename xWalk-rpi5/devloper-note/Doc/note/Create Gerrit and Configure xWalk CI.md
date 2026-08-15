# Create Gerrit and Configure xWalk CI

This guide explains how to create a new non-root Gerrit service and connect the repository-managed xWalk CI
worker. It applies to a new, empty Gerrit destination. It is not an in-place upgrade or restore procedure.

The supported implementation is under `xWalkTool/py-agent/gerrit-tool`. It installs Gerrit, Caddy, management
commands, CI programs, configuration templates, and rendered administrator guides without `sudo`.

## Safety boundaries

- Run the setup as the dedicated normal Linux account that will own Gerrit.
- Use persistent storage owned by that account; never select `/`, `$HOME`, `/tmp`, or another broad path.
- Do not run two Gerrit processes against the same site.
- Do not reuse an existing non-empty site. Use the documented backup and restore workflow instead.
- Keep passwords, SSH private keys, CodeScene tokens, and GitHub credentials outside the repository.
- Keep automatic hardware tests disabled. The xWalk CI graph is host-only and device-free.

## Prepare the host

Clone the integrated MyPiCarX repository on the future Gerrit server and enter its root. Run the read-only
assessment:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-setup.sh assess
```

Confirm persistent storage, free space, Java support, the physical network address, and availability of the
configured HTTPS, SSH, and loopback HTTP ports.

Edit `xWalkTool/py-agent/gerrit-tool/config/gerrit-setup.conf`. At minimum, replace the server address and the
official Gerrit WAR checksum, then review the administrator identity, project, branch, ports, storage path, and
process manager:

```bash
export EDUVPN_SERVER_IP="SERVER_IP_FROM_ASSESSMENT"
export GERRIT_STORAGE_PATH=""
export GERRIT_SHA256="OFFICIAL_GERRIT_WAR_SHA256"
export GERRIT_PROJECT="xWalk-rpi5"
export GERRIT_BRANCH="main"
```

An empty `GERRIT_STORAGE_PATH` selects `$HOME/gerrit-site`. If an administrator provides another persistent
location, set its absolute path and validate it before installation:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-storage-check.sh
```

## Install Gerrit

Run the installer from the repository root:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-setup.sh install
```

The script requests the initial administrator password without echoing it. It verifies downloaded artifacts,
initializes Gerrit on loopback, creates an initial backup, configures HTTPS through Caddy, binds Gerrit SSH,
installs xWalk management and CI tools, starts the services, and validates the endpoints.

After installation, use only the generated management commands:

```bash
$HOME/bin/gerrit-status
```

```bash
$HOME/bin/gerrit-check
```

```bash
$HOME/bin/gerrit-logs
```

After a logout or reboot, start and validate the existing site with:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-setup.sh start
```

## Initialize the administrator and project

Open the configured HTTPS URL and sign in as `GERRIT_ADMIN_USER`. Add the administrator's individual SSH public
key under **Settings → SSH Keys**, then verify SSH administration:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit version
```

Create these groups if they do not already exist:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit create-group Project-Owners
```

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit create-group Developers
```

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit create-group Reviewers
```

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit create-group "'Service Users'"
```

Add the administrator to `Project-Owners`, then create the configured project and initial branch if the project
does not already exist:

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit set-members --add GERRIT_ADMIN_USER Project-Owners
```

```bash
ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit create-project --branch PROJECT_BRANCH --owner Project-Owners PROJECT_NAME
```

Do not repeat `create-project` against an existing project. Inspect the project list first.

## Configure project review permissions

Open **Projects → List → PROJECT_NAME → Access** and establish this minimum model:

| Reference | Group | Permission |
|---|---|---|
| `refs/*` | `Project-Owners` | Read and Owner |
| `refs/for/refs/heads/PROJECT_BRANCH` | `Developers` | Push |
| `refs/heads/PROJECT_BRANCH` | `Reviewers` | Intended Code-Review range |
| `refs/heads/PROJECT_BRANCH` | `Project-Owners` | Code-Review `-2..+2` and Submit |
| `refs/heads/PROJECT_BRANCH` | `Service Users` | `Verified -1..+1` only |

Do not grant ordinary developers direct Push, Force Push, Create Reference, Submit, or `Verified`. Configure a
`Verified` label with values `-1`, `0`, and `+1`, and add a submit requirement that requires the current patch
set's `Verified +1`. Keep `Code-Review +2` as the human approval requirement. Gerrit label definition and submit
requirements belong in the reviewed `All-Projects` or project `refs/meta/config` configuration.

For the repository's fixed multi-repository architecture, use the reviewed provisioning and ACL tools instead
of recreating the complete matrix manually:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-multi-repo-provision.sh --dry-run
```

Apply that workflow only after reviewing
`xWalkTool/py-agent/gerrit-tool/DevloperNote/Doc/note/Gerrit Multi Repository Architecture.md` and supplying its
explicit confirmation variables.

## Import the initial project history

A newly created Gerrit project is empty. CI cannot verify a patch set until the protected branch contains the
reviewed source history. Perform the initial import once from a clean, verified clone and only with explicit
project-owner authorization.

Add the new Gerrit project as a separate remote and inspect it before pushing:

```bash
git remote add gerrit ssh://GERRIT_ADMIN_USER@GERRIT_SERVER_HOST:GERRIT_SSH_PORT/PROJECT_NAME && git remote -v
```

Push the selected existing source branch to the empty Gerrit branch:

```bash
git push gerrit SOURCE_BRANCH:refs/heads/PROJECT_BRANCH
```

Do not use force, mirror, wildcard, or direct GitHub pushes. After bootstrap, every ordinary change uses
`git push gerrit HEAD:refs/for/PROJECT_BRANCH` and passes review before submission.

## Create the CI service identity

Generate a dedicated server-local SSH key. Do not reuse the administrator or GitHub synchronization key:

```bash
ssh-keygen -t ed25519 -f "$HOME/.ssh/id_ed25519_xwalk_ci" -C "xwalk-ci@GERRIT_SERVER_HOST"
```

Create the service account with only its public key and add it directly to `Service Users`:

```bash
cat "$HOME/.ssh/id_ed25519_xwalk_ci.pub" | ssh -p GERRIT_SSH_PORT GERRIT_ADMIN_USER@GERRIT_SERVER_HOST gerrit create-account --group "'Service Users'" --full-name "'xWalk CI'" --ssh-key - xwalk-ci
```

The service account needs only project read access, patch-set ref access, event-stream access, and
`Verified -1..+1`. It must not receive ownership, Submit, Force Push, or server-administration permission.

## Configure the xWalk CI worker

The installer renders the reviewed environment template with the selected endpoints. Copy it to the protected
runtime location and inspect every value:

```bash
. "$HOME/gerrit-env.sh" && cp "$GERRIT_SITE/docs/XWALK_CI_ENV.example" "$HOME/.xwalk-ci.env" && chmod 600 "$HOME/.xwalk-ci.env"
```

At minimum, verify these settings in `$HOME/.xwalk-ci.env`:

| Setting | Required value |
|---|---|
| `GERRIT_HOST` | Configured Gerrit host or IP |
| `GERRIT_SSH_PORT` | Configured Gerrit SSH port |
| `GERRIT_USER` | `xwalk-ci` |
| `GERRIT_PROJECT` | Project watched by the worker |
| `GERRIT_BRANCH` | Protected review branch |
| `GERRIT_VERIFICATION_TARGETS` | Exact comma-separated `project:branch` allowlist |
| `GERRIT_SSH_KEY` | `$HOME/.ssh/id_ed25519_xwalk_ci` |
| `XWALK_CI_LOG_WEB_URL` | HTTPS `/ci` route installed through Caddy |
| `XWALK_CI_MAX_WORKERS` | Bounded worker count, normally `4` |

Leave `GERRIT_UPLIFT_ENABLED=false` and `GITHUB_PUSH_ENABLED=false` until their separate repository, key,
approval, and fast-forward safeguards have been reviewed. Leave `XWALK_CODESCENE_STRICT=false` until an
installed licensed CodeScene CLI has completed a real analysis.

Restrict the CI private key and test service-account SSH access:

```bash
chmod 600 "$HOME/.ssh/id_ed25519_xwalk_ci" && ssh -i "$HOME/.ssh/id_ed25519_xwalk_ci" -p GERRIT_SSH_PORT xwalk-ci@GERRIT_SERVER_HOST gerrit version
```

## Start and verify CI

Start the repository-owned worker and verify its process state:

```bash
$HOME/bin/gerrit-ci-control start
```

```bash
$HOME/bin/gerrit-ci-control status
```

Follow the service log during the first controlled patch-set upload:

```bash
$HOME/bin/gerrit-ci-logs
```

Verify the HTTPS dashboard health route using the installed trust certificate:

```bash
. "$HOME/gerrit-env.sh" && curl --fail --show-error --silent --cacert "$GERRIT_SITE/etc/gerrit-self-signed.crt" "https://GERRIT_SERVER_HOST:GERRIT_HTTPS_PORT/ci/health"
```

Upload a reviewed test change to `refs/for/PROJECT_BRANCH`. Confirm that the worker receives the
`patchset-created` event, checks out the exact patch set, runs the host-only graph, publishes module results,
and applies exactly one final `Verified +1` or `Verified -1` vote. The worker never submits the change.

## Operational checklist

- Back up Gerrit with `$HOME/bin/gerrit-backup` and verify the generated checksum.
- Review `$HOME/bin/gerrit-check` after configuration or restart changes.
- Keep the CI environment and all private keys at mode `0600`.
- Keep full logs under the configured `$HOME/gerrit-ci/logs` path.
- Apply reviewed CI program updates while the worker is stopped.
- Never run the retained Zuul configuration and the Python worker against the same project.
- Never execute hardware tests from automatic Gerrit CI.

See [Add a User to a Gerrit Repository](Add%20a%20User%20to%20a%20Gerrit%20Repository.md) for contributor
onboarding and [xWalkTool Overview](xWalkTool%20Overview.md) for the complete tooling inventory.

The command forms follow Gerrit's official `create-account`, `set-members`, and `create-project` SSH command
references. Review the documentation matching the installed Gerrit version before changing server policy.
