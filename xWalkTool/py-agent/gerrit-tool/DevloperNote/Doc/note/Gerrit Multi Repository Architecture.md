# Gerrit multi-repository architecture

## Scope and safety state

The target architecture has nine Gerrit-only component repositories and one
private Gerrit integration repository. During migration, `xWalkPiCarAI/master`
is the active integrated Gerrit and GitHub branch; the final integration target
is `xWalk-rpi5/main`. The migration tools stage work in new clones so the source
monorepo remains usable until the administrator validates and submits the
migration. A dry run is a plan, not proof that Gerrit repositories or
permissions exist.

The component repositories are `DevloperNote`, `xWalkAgent`,
`xWalkAudioResources`, `xWalkController`, `xWalkHal`, `xWalkIW`,
`xWalkLibrary`, `xWalkTrace`, and `xWalk-rpi5-sim`. `xWalk-rpi5` records them as
exact submodule gitlinks together with top-level integration files, CMake entry
points, GitHub workflows, and integration history. Reproducible builds must use
`git submodule update --init --recursive`; they must not use uncontrolled
`git submodule update --remote`.

## Configuration and connectivity

Copy and edit `xWalkTool/py-agent/gerrit-tool/config/multi-repo.conf`. Resolve the Gerrit
host and administrator placeholders, retain unprivileged ports, and do not
store passwords, tokens, or private keys in the file. `GERRIT_SSH_PORT` is the
Gerrit SSH service, not the college login SSH port.

```bash
export XWALK_GERRIT_MULTI_REPO_CONFIG="$PWD/xWalkTool/py-agent/gerrit-tool/config/multi-repo.conf"
```

The tools use three configurable groups: `xWalk-Owners`, `xWalk-Partners`, and
`xWalk-CI`. The CI account is not an administrator. Only owners manage ACLs.
The optional permission-only parent is `xWalk-Projects`.

## Access matrix

| Repository | Public | Partner | Owner | CI |
|---|---|---|---|---|
| `DevloperNote` | Read/clone | Develop, review and submit | Full/Owner | Read and Verified |
| `xWalkHal` | Read/clone | Review push | Full/Owner | Read and Verified |
| `xWalkController` | No access | Review push | Full/Owner | Read and Verified |
| `xWalkIW` | No access | Read/clone | Full/Owner | Read and Verified |
| `xWalkAgent` | No access | Read/clone | Full/Owner | Read and Verified |
| `xWalkLibrary` | Read/clone | Review push | Full/Owner | Read and Verified |
| `xWalkTrace` | Read/clone | Review push | Full/Owner | Read and Verified |
| `xWalkAudioResources` | No access | No access | Full/Owner | Read and Verified |
| `xWalk-rpi5-sim` | No access | No access | Full/Owner | Read and Verified |
| `xWalk-rpi5` | No access | No access | Full/Owner | Read, uplift review push and Verified |

Review push means read plus upload to `refs/for/main`; it does not grant direct
branch push, submit, force push, branch deletion, ACL administration, or
ownership. Partner direct push to `DevloperNote/main` remains disabled unless
`GERRIT_PARTNER_DEVNOTE_DIRECT_PUSH=true` is deliberately selected. Private
projects use exclusive read rules and explicit anonymous, registered-user, and
partner denial rules to block unsafe inheritance from `All-Projects`.

## Provisioning and ACL reconciliation

Always review the plan first:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-multi-repo-provision.sh --dry-run
```

Provisioning discovers and preserves existing groups and projects. Apply is
allowed only after an administrator confirms the plan:

```bash
export XWALK_CONFIRM_PROVISION="CREATE_XWALK_REPOSITORIES"; export XWALK_CONFIRM_ACL="APPLY_XWALK_ACL"; xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-multi-repo-provision.sh --apply
```

The ACL tool clones and reviews `refs/meta/config`; it never edits
`$GERRIT_SITE/git`. Each permission receives a separate audit row. Before
apply, back up Gerrit and inspect inherited permissions on `All-Projects`.

## History-preserving migration and initial imports

Install an administrator-approved `git-filter-repo` command before apply. The
split tool validates the fixed component allowlist, requires a clean source,
creates independent clones outside the workspace, rewrites only those clones,
renames their branch to `main`, and leaves the original history unchanged.

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-history-split.sh --dry-run
```

```bash
export XWALK_SPLIT_OUTPUT_DIR="/safe/new/component-splits"; export XWALK_CONFIRM_SPLIT="SPLIT_COMPONENT_HISTORY"; export XWALK_IMPORT_MODE="none"; xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-history-split.sh --apply
```

Inspect every split repository before import. Use `XWALK_IMPORT_MODE=review`
for `refs/for/main`. Use `direct` only for an administrator-authorized empty
repository bootstrap. Neither mode configures or pushes a component GitHub
remote. Do not use force push, `--mirror`, or wildcard refspecs.

## Submodule conversion

The conversion tool creates a new integration clone; it does not remove live
component directories from this workspace. Apply it only after all component
`main` branches contain the verified split commits.

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-submodule-migrate.sh --dry-run
```

```bash
export XWALK_INTEGRATION_OUTPUT_DIR="/safe/new/xWalk-rpi5"; export XWALK_CONFIRM_SUBMODULES="CREATE_INTEGRATION_CLONE"; xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-submodule-migrate.sh --apply
```

The resulting commit must be reviewed before replacing the monorepo baseline.
Validate `.gitmodules`, every mode-`160000` gitlink, recursive initialization,
and the absence of component GitHub remotes.

## Developer workflows

An authorized user clones the private integration project with:

```bash
git clone --recurse-submodules ssh://USER@GERRIT_SERVER_HOST:GERRIT_SSH_PORT/xWalk-rpi5
```

Existing clones update safely with:

```bash
git pull --ff-only && git submodule sync --recursive && git submodule update --init --recursive
```

A module change is committed and reviewed inside that module:

```bash
cd xWalk-rpi5/xWalkHal && git switch main && git pull --ff-only origin main && git add . && git commit -s -m "Update HAL implementation" && git push origin HEAD:refs/for/main
```

Coordinated API work uses a Gerrit topic:

```bash
git push origin HEAD:refs/for/main%topic=TOPIC_NAME
```

After every related change is submitted, the topic-uplift entry point accepts
one repository, full commit, and Gerrit change triple per component. It updates
all listed gitlinks in one commit and runs integration validation once:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-topic-uplift.sh --dry-run TOPIC_NAME xWalkLibrary LIBRARY_COMMIT LIBRARY_CHANGE xWalkHal HAL_COMMIT HAL_CHANGE
```

The partner documentation workflow is:

```bash
git clone ssh://PARTNER@GERRIT_SERVER_HOST:GERRIT_SSH_PORT/DevloperNote && cd DevloperNote && git add . && git commit -s -m "Update documentation" && git push origin HEAD:refs/for/main
```

Users generate their own SSH keys on their own computers and upload only the
public key to their individual Gerrit accounts. Private keys are never shared.

## Standalone and compatibility validation

Each migrated code repository must provide a top-level standalone CMake entry
point before its ACL allows normal submission. The standard validation is:

```bash
cmake --fresh -S MODULE -B build-host/MODULE -G Ninja -DCMAKE_BUILD_TYPE=Debug -DXWALK_STANDALONE_BUILD=ON
```

```bash
cmake --build build-host/MODULE --parallel
```

```bash
ctest --test-dir build-host/MODULE --output-on-failure --no-tests=error
```

Dependencies come from exact `xWalkLibrary`, `xWalkTrace`, and
`xWalkAudioResources` revisions or approved artifacts. `xWalkLibrary` and
`xWalkTrace` changes test direct consumers; audio-resource changes validate
resources and audio consumers; documentation changes validate formatting and
links. Cross-repository topics test all participating revisions. Until every
split repository has a self-contained standalone entry point, trusted Gerrit
CI must validate the patch in an exact `xWalk-rpi5` integration checkout and the
migration must not be declared complete.

## Automatic uplift and recovery

After a submitted module commit receives `Verified +1`, invoke the uplift tool
with the full commit, source Gerrit change, and optional topic:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-auto-uplift.sh --dry-run xWalkHal FULL_COMMIT_ID GERRIT_CHANGE TOPIC
```

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-auto-uplift.sh --apply xWalkHal FULL_COMMIT_ID GERRIT_CHANGE TOPIC
```

Apply acquires a lock, clones clean `xWalk-rpi5`, proves the component commit is
reachable from its Gerrit `main`, changes only that gitlink, creates a
signed-off uplift commit, runs the complete host integration build, tests, and
`--no-hardware` diagnostic, then uploads `refs/for/main`. Automatic submission
is disabled by default. A failure leaves Gerrit `xWalk-rpi5/main` and the last
verified baseline unchanged. Retry after correcting the module, dependency, or
concurrent-uplift conflict; never force-push. The topic-uplift entry point
batches related submitted revisions into one signed-off integration commit,
one complete validation, and one Gerrit review.

## GitHub policy and Actions checkout

Only a submitted and integration-verified branch selected by
`GITHUB_SYNC_SOURCE_PROJECT` and `GITHUB_SYNC_SOURCE_BRANCH` may synchronize.
The accepted pairs are the current `xWalkPiCarAI/master` migration branch and
the final `xWalk-rpi5/main` branch. The GitHub repository name and branch must
match the selected source. Component GitHub repositories, component GitHub
remotes, `git push --mirror`, `git push --all`, wildcard refspecs, force push,
and `git submodule foreach git push` are prohibited.

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-github-sync.sh --dry-run
```

The event-driven service additionally requires `GITHUB_PUSH_ENABLED=true`, an
exact integrated-project destination, and the configured CI account's
`Verified +1` on the submitted revision. The standalone final-migration helper
requires `XWALK_INTEGRATION_VERIFIED_COMMIT` equal to the fetched Gerrit `main`
tip. The submitted owner must match the separately configured
`GITHUB_DIRECT_PUSH_OWNER_EMAIL`. The only push refspec is
the configured source branch to its same-named GitHub branch. A change owned by
anyone else is not pushed directly; it must follow the repository owner's
approved pull-request process.

GitHub Actions uses `gerrit-github-checkout.sh` with a repository-scoped,
read-only Gerrit deploy identity stored in GitHub Secrets. The helper scans the
host key, enables strict verification, initializes exact submodule commits,
then removes temporary SSH configuration. If college policy does not permit a
Gerrit key in GitHub Actions, set `XWALK_GITHUB_METADATA_ONLY=true`: GitHub then
validates only `.gitmodules`, gitlinks, top-level metadata, and documentation;
full builds remain in trusted Gerrit CI. Do not make private modules public to
make GitHub Actions pass.

## Complete integration validation

```bash
cmake --fresh -S . -B build-host/integration -G Ninja -DCMAKE_BUILD_TYPE=Debug -DXWALK_ENABLE_STRICT_WARNINGS=ON
```

```bash
cmake --build build-host/integration --parallel
```

```bash
ctest --test-dir build-host/integration --output-on-failure --no-tests=error
```

```bash
build-host/integration/xWalkController/xWalkApp/xwalk-picarx-control --deployment-config="$PWD/xWalkController/xWalkConfig/picar-x.conf" --diagnose --no-hardware
```

Ordinary CI never actuates physical hardware.

## Gerrit change history

Record source changes in the Gerrit commit message and review history. CI votes,
validation results, and synchronization outcomes are posted directly to the
affected Gerrit change. Do not create CSV or Markdown operation logs in the
source repository.

## Permission verification

Plan all forty identity/repository checks with:

```bash
xWalkTool/py-agent/gerrit-tool/shell-script/gerrit-permission-check.sh --dry-run
```

Apply read checks only with dedicated public, partner, owner, and CI test
identities. Missing credentials are recorded as `Skipped`, never as success.
Read checks do not prove review upload, label, submit, force-push, branch
deletion, or ACL administration. Test those negative and positive operations
with disposable changes and the correct individual accounts, then inspect each
`refs/meta/config`. The partner must be unable to list or clone
`xWalkAudioResources` and `xWalk-rpi5`; CI must be unable to administer ACLs or
force-push.

## Adding another module

Adding a module is an architecture change. Update the fixed allowlists, ACL
matrix, provisioning tests, split tests, standalone CI dependency map,
`.gitmodules`, uplift validation, documentation, and operation-log report in
one reviewed topic. Create it only in Gerrit. Do not create a component GitHub
repository.

## Credential rotation

Create the replacement key on the account or CI secret owner, upload its
public half, verify read/vote or owner behavior, rotate the protected secret,
restart only the user-owned CI process, then revoke the old public key. Rotate
GitHub's least-privilege `xWalk-rpi5` deploy credential separately. Never display
or commit private keys, tokens, cookies, authorization headers, or passwords.

## Administrator actions and limitations

The Gerrit administrator must authorize project creation, group membership,
`refs/meta/config` changes, the initial direct imports when needed, CI label
permissions, and network reachability. College IT must allow the configured
Gerrit HTTP/HTTPS and SSH ports from the eduVPN subnet when blocked. A shared
disk does not provide network access.

This repository deliberately does not perform provisioning, imports,
submodule conversion, permission probes, uplifts, or GitHub pushes during a
normal build. Remote readiness requires administrator-authorized apply runs and
independent permission tests with real accounts.
