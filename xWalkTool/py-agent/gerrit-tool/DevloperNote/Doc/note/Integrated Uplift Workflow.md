# Integrated uplift workflow

## Purpose

`xWalkPiCarAI/master` is the configured integrated Gerrit repository during the current migration. A component
merge creates an integrated review; it never pushes directly to the protected branch or to GitHub.

```text
Component Gerrit merge
    -> locked xWalkPiCarAI uplift review
    -> complete integrated CI
    -> Verified +1
    -> authorized Code-Review +2
    -> guarded Gerrit submit
    -> exact merged-commit GitHub synchronization
```

The final gitlink superproject remains `xWalk-rpi5/master`. Repository and branch names are configuration values so
the migration can complete without rewriting the safety policy.

## Repository mapping

| Source Gerrit repository | Source branch | Integrated path |
|---|---|---|
| `xWalkAgent` | `master` | `xWalk-rpi5/xWalkAgent` |
| `xWalkAudioResources` | `master` | `xWalk-rpi5/xWalkAudioResources` |
| `xWalkController` | `master` | `xWalk-rpi5/xWalkController` |
| `xWalkHal` | `master` | `xWalk-rpi5/xWalkHal` |
| `xWalkIW` | `master` | `xWalk-rpi5/xWalkIW` |
| `xWalkLibrary` | `master` | `xWalk-rpi5/xWalkLibrary` |
| `xWalkTool` | `master` | `xWalkTool` |
| `DevloperNote` | `master` | `xWalk-rpi5/devloper-note` |
| `xWalkTrace` | `master` | `xWalk-rpi5/xWalkTrace` |
| `xWalk-rpi5-sim` | `master` | `xWalk-rpi5-py3` gitlink |

The Python component changes only its explicit Gerrit gitlink. The other nine components replace only their
mapped integrated source trees.

## Event and uplift processing

The CI service listens for `change-merged` events from the ten allowlisted component repositories. The uplift
worker validates the full source SHA and patch-set number. It proves that the SHA is reachable from the component
`master` branch, and copies the exact Git archive into the mapped integrated directory, except for the Python
component's exact gitlink update. Files outside the selected integration path cannot enter the uplift commit.

The generated commit preserves the source subject and body and appends provenance trailers in this form:

```text
[xWalk-123][x86-HOST] Improve trace validation

Original component commit body.

Source-Repository: xWalkTrace
Source-Commit: 0123456789abcdef0123456789abcdef01234567
Source-Change: 91
Source-Patchset: 3
```

The topic is `uplift-xWalkTrace-91`. A deterministic Change-Id based on the integrated project, branch, component,
and source change updates an existing open uplift when a later source patch set is processed. A locked event marker
prevents duplicate processing of the same source commit. The worker rechecks the integrated branch immediately
before upload and retries from a fresh clone if another uplift changed the baseline.

The active `refs/for/master` upload triggers integrated CI. The worker does not run an early submit operation.

## Integrated CI and labels

The mandatory integrated graph contains:

- integrated-source and Gerrit gitlink metadata validation;
- dependency and configuration validation;
- GCC Debug and Release builds and tests;
- Clang Debug and Release builds and tests;
- unit, group, simulation, recorded-media, streaming, and CLI integration tests;
- Clang-Tidy, Cppcheck, Clang Static Analyzer, sanitizer, Valgrind, coverage, stress, and fuzz checks;
- C++ style, four-space indentation, Allman braces, and 120-character maximum line validation;
- ARM64 dependency-audit tests and an ARM64 cross-build when `XWALK_AARCH64_SYSROOT` is configured;
- staged installation, packaging inputs, deployment scripts, and device-free deployment diagnosis;
- CodeScene changed-code analysis under the configured rollout policy.

The final gate applies `Verified +1` only when every mandatory job succeeds. Any failure applies `Verified -1` to
the exact current patch set. A newer patch set does not inherit the worker's acceptance because submission compares
the event revision, current patch-set revision, and approvals again.

The generated `xWalkPiCarAI` project configuration requires:

- `Code-Review +2` from an account allowed by the Gerrit ACL;
- `Verified +1` from the CI service;
- no unresolved review comments.

Gerrit itself additionally rejects a non-current, non-mergeable, conflicted, or otherwise non-submittable change.
The worker requires Gerrit's complete submit record to be `OK` before requesting submission, so repository policy
remains authoritative.

## Accounts, secrets, and permissions

Use three separate identities where automatic review is intentionally enabled:

| Identity | Required permissions | Prohibited permissions |
|---|---|---|
| `xwalk-ci` | Read, review upload, `Verified`, submit | Owner, direct push, `Code-Review` |
| Authorized human reviewer | `Code-Review -2..+2` | CI secrets, administrator credentials |
| Dedicated uplift reviewer | Conditional `Code-Review +2` | Administrator, direct push, `Verified` |

Automatic review is disabled by default. To enable it, set `GERRIT_UPLIFT_AUTO_REVIEW=true` and configure
`GERRIT_REVIEW_USER`. Provide its mode-`0600` key through
`GERRIT_REVIEW_SSH_KEY`. The CI and reviewer usernames must differ. The dedicated reviewer acts only after all
mandatory CI jobs pass.

Store these values in the CI credential system or the mode-`0600` service environment, never in Git:

- `GERRIT_SSH_KEY`: CI account private key;
- `GERRIT_REVIEW_SSH_KEY`: optional dedicated reviewer key;
- `GITHUB_SSH_KEY`: repository-scoped GitHub deploy key with protected-branch write access;
- `CS_ACCESS_TOKEN`: CodeScene token when licensed analysis is enabled;
- pinned Gerrit and GitHub known-host entries.

No script needs an administrator password, administrator SSH key, GitHub personal access token, or embedded URL
credential. Output redaction removes secret assignments, authenticated URLs, and private-key blocks from CI logs.

## Configuration

The service template is `config/XWALK_CI_ENV.example`; shell helpers use `config/multi-repo.conf`. Important values
are:

| Setting | Migration value |
|---|---|
| `GERRIT_INTEGRATION_PROJECT` | `xWalkPiCarAI` |
| `GERRIT_INTEGRATION_BRANCH` | `master` |
| `GERRIT_INTEGRATION_SOURCE_ROOT` | `xWalk-rpi5` |
| `GERRIT_UPLIFT_ENABLED` | `true` after ACL installation |
| `GERRIT_UPLIFT_AUTO_SUBMIT` | `false` until submit permission and policy are verified |
| `GERRIT_UPLIFT_AUTO_REVIEW` | `false` unless the documented dedicated-account policy is approved |
| `GITHUB_SYNC_SOURCE_PROJECT` | `xWalkPiCarAI` |
| `GITHUB_SYNC_SOURCE_BRANCH` | `master` |
| `GITHUB_INTEGRATION_REMOTE` | Required SSH URL for the confirmed GitHub `xWalkPiCarAI` repository |
| `GITHUB_INTEGRATION_BRANCH` | `master` |
| `GITHUB_PUSH_ENABLED` | `false` until a protected-branch synchronization test is approved |

The workspace has an `xWalkPiCarAI` GitHub remote. The deployment owner must confirm its organization and deploy
key before populating `GITHUB_INTEGRATION_REMOTE`. Automation does not guess or create a destination.

## GitHub synchronization

Only a `change-merged` event for the configured integrated project and branch is eligible. The service re-queries
the merged change, requires its exact revision to have the CI account's `Verified +1`, fetches the current Gerrit
branch tip, and requires it to equal the merge result. It then performs a non-force branch-to-branch push and reads
the GitHub branch back to verify the exact SHA.

An already matching GitHub SHA is a successful idempotent no-op. A diverged GitHub branch, missing Gerrit revision,
failed fetch, rejected push, or read-back mismatch stops synchronization. Component repositories and GitHub events
cannot match the component-merge selector, preventing recursive uplift loops.

## Audit changelog and output

Every fetch, uplift, CI, vote, merge, and GitHub-sync result appends one JSON object to
`XWALK_UPLIFT_CHANGELOG`. Each record contains the timestamp, module, operation, source change and patch set,
commit, integrated change, integrated commit, result, explanation, and relevant link. The file is created with mode
`0600`. Uplift jobs share an exclusive lock; JSON records are append-only and contain no credentials.

The merged component change also receives an `xWalk Integration Uplift` change-log row. The row reports the
uplift lifecycle status, linked integration review when one was created, and a safe rejection reason when Gerrit
blocks the upload. Automated uplift commits use the registered `GERRIT_CI_EMAIL`; placeholder commit identities
are not accepted by Gerrit.

An automatic uplift preserves the submitted component commit subject and body. It replaces the component
`Change-Id` with the deterministic integration `Change-Id`, adds source repository, commit, change, patch-set, and
topic trailers, and adds the CI sign-off. This keeps the original intent visible without making the integration
review update the already submitted component review.

Typical successful output is:

```text
Source module: xWalkTrace
Source Gerrit change: 91,3
Integrated Gerrit change: 104
Integrated Gerrit URL: https://gerrit.example/c/xWalkPiCarAI/+/104
CI status: all mandatory jobs passed
Gerrit labels: Verified +1, Code-Review +2
Merge status: submitted
GitHub synchronization status: exact merged SHA confirmed
```

A blocked operation prints the exact reason, for example:

```text
Integrated submission blocked: Approval or CI result belongs to a superseded patch set
```

## Retry, rollback, and recovery

Temporary Git, Gerrit SSH, network, and GitHub failures use bounded retries. Unreachable module commits and
commits, unexpected paths, CI failures, missing labels, submit-policy failures, and merge conflicts are permanent
for that event and are never converted to success.

Before submission, rollback means abandoning the open uplift change; the protected branch and GitHub are unchanged.
After submission, use a normal reviewed Gerrit revert rather than rewriting history. GitHub synchronization is
fast-forward only and can be retried after fixing credentials or connectivity.

For manual recovery:

1. Read the last JSON changelog entry and the linked Gerrit CI log.
2. Confirm the component SHA is on its Gerrit `master` branch.
3. Confirm the integrated change's current patch set and submit requirements in Gerrit.
4. Re-deliver the component merge event or rerun the uplift command with the same exact metadata.
5. If Gerrit is merged but GitHub is behind, restart CI. Startup reconciliation checks both branch tips.
6. Never force-push either protected branch and never publish an unmerged patch set to GitHub.

## Administrator activation checklist

Repository code cannot safely perform these deployment operations automatically:

1. Review and apply the generated `xWalkPiCarAI` ACL and submit requirements to `refs/meta/config`.
2. Confirm Gerrit accepts the `-has:unresolved` submit expression on the installed Gerrit version.
3. Add `xwalk-ci` with only read, review-upload, `Verified`, and submit permissions.
4. Keep `Code-Review +2` with authorized humans, or approve and provision a distinct automatic reviewer.
5. Install mode-`0600` SSH keys and pinned host keys through the CI credential store.
6. Confirm the GitHub repository and protected `master` destination, then install its repository-scoped deploy key.
7. Run one safe end-to-end test change before enabling automatic submit or GitHub push.

Until that checklist is completed and tested, automatic approval, automatic submission, and GitHub synchronization
must be reported as configured but not operationally verified.
