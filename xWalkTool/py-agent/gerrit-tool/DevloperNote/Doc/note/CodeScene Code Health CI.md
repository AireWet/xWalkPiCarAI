# CodeScene Code Health CI

## Supported integration architecture

CodeScene is an additional changed-code quality signal, not a compiler or runtime verifier. The supported flow is:

```text
Developer -> Gerrit patch set -> Gerrit host CI and optional licensed CodeScene CLI delta -> review and submit -> MyPiCarX GitHub mirror -> GitHub Actions and native CodeScene analysis
```

Current CodeScene documentation provides native pull-request integrations for GitHub and for Gerrit. The Gerrit
integration is available for on-premises CodeScene installations and is configured in CodeScene with a Gerrit
service identity. It is not assumed to exist in every licensed edition. The official CodeScene CLI provides the
supported `cs delta` command for local or CI delta analysis after the CLI is licensed and activated. This repository
does not download, license, activate, or guess endpoints for CodeScene.

The repository-owned adapter is `xWalkTool/py-agent/dev-tool/xWalkCodeHealth`. It validates project configuration,
resolves full Git commit IDs, calculates changed files, invokes `cs delta --output-format json BASE REVISION` only
when an executable is configured, and writes reports below `build-host/codescene`. It never invokes an undocumented
API. The adapter resolves an explicit `XWALK_CODESCENE_CLI`, the service `PATH`, and CodeScene's official
`~/.local/bin/cs` installation location in that order. This keeps a non-login CI service independent of interactive
shell path initialization.

Official references:

- [CodeScene CLI documentation](https://docs.enterprise.codescene.io/latest/cli/index.html)
- [CodeScene CLI command reference](https://codescene.io/docs/cli/command-reference.html)
- [CodeScene Gerrit integration](https://docs.enterprise.codescene.io/latest/integrations/pr-integration/gerrit.html)
- [CodeScene CI/CD integration](https://docs.enterprise.codescene.io/versions/7.0.9/guides/pr-integration/integrate-into-ci-cd.html)
- [CodeScene REST API](https://codescene.io/docs/integrations/rest-api.html)

## What is checked

The delta stage evaluates new and modified code between an exact baseline and reviewed revision. When the CLI result
reports a code-health degradation, goal violation, or failing return code, the report marks its quality gate failed.
The summary records the two full commit IDs, eligible changed-file count, mapped architectural components,
degradation result, gate result, analysis link when supplied by CodeScene, and non-secret Gerrit or GitHub correlation
identifiers.

CodeScene does not replace GCC or Clang compilation, CTest, sanitizers, Clang-Tidy, Cppcheck, Clang Static Analyzer,
Valgrind, security checks, architecture validation, fuzzing, stress tests, watchdogs, or test timeouts. It cannot be
treated as a reliable detector of every logical error or infinite loop. Those checks remain independent and continue
to run if CodeScene is unavailable or fails.

## Architectural components and exclusions

`.codescene/project.json` is the repository source of truth. `.codescene/architectural-components.json` contains the
same component payload for an administrator to apply through a supported CodeScene configuration method:

| CodeScene component | Integrated path |
|---|---|
| Agent | `xWalk-rpi5/xWalkAgent/**` |
| Audio Resources | `xWalk-rpi5/xWalkAudioResources/**` |
| Controller | `xWalk-rpi5/xWalkController/**` |
| HAL | `xWalk-rpi5/xWalkHal/**` |
| Interface | `xWalk-rpi5/xWalkIW/**` |
| Library | `xWalk-rpi5/xWalkLibrary/**` |
| Trace | `xWalk-rpi5/xWalkTrace/**` |
| Developer Documentation | `xWalk-rpi5/devloper-note/**` |

`.codescene/analysis-exclusions.txt` lists only build output, downloaded/vendor content, caches, generated content,
and CTest output. Production sources, public headers, `HostTest`, unit tests, integration tests, and configuration
validation tests remain included. These repository manifests do not silently change a CodeScene server project; an
administrator must apply and verify them through the licensed product's supported UI or API.

## GitHub configuration

1. Configure the official CodeScene GitHub App for the single integrated `MyPiCarX` repository. Do not configure
   separate component repositories.
2. In CodeScene, select the integrated `xWalk-rpi5` content and apply the component and exclusion manifests.
3. Enable CodeScene pull-request or delta analysis and its changed-code quality gate in the CodeScene project.
4. Require the native CodeScene check in GitHub branch protection only after the initial non-blocking rollout is
   producing reliable results.
5. If an approved self-hosted runner has an activated CLI, set the repository variable `XWALK_CODESCENE_STRICT` to
   `true` after validation. GitHub-hosted runners are not assumed to contain the licensed CLI.

The workflow keeps `contents: read`, checks out full history for the Code Health job, and references no repository
secret. Forked pull requests therefore receive no repository credential while all normal build and test jobs run.
The native GitHub App owns its webhook and credentials outside workflow YAML. No `CODESCENE_ACCESS_TOKEN`,
`CODESCENE_SERVER_URL`, or `CODESCENE_PROJECT_ID` secret is referenced because the verified architecture does not
require those values in GitHub Actions.

Create and rotate any native integration credential using the current CodeScene administration procedure and the
least-privileged bot/service identity supported by the licensed edition. Store it only in CodeScene's integration
configuration or an approved runner secret store, revoke the previous credential after validation, and never place
its value in this repository, job arguments, or logs.

## Gerrit behavior

The worker fetches and checks out the event's patch-set ref before analysis. It supplies change number, patch-set
number, revision, refspec, project, and branch to the adapter. The delta revision is that exact patch-set commit and
the baseline is its first parent. The Code Health graph node appears from the beginning of the run and posts its own
Gerrit change-log entry containing status, changed-file count, degradation, gate result, and the analysis link when
the CLI supplies one.

No new Gerrit label is created. The existing final Host Quality Gate produces `Verified +1` only when every mandatory
node passes, or `Verified -1` when one fails. During initial rollout `XWALK_CODESCENE_STRICT=false`: a missing CLI,
timeout, malformed result, or unavailable service is clearly `UNAVAILABLE` but does not fail the node. A genuine
CodeScene failed gate is also reported without blocking in rollout mode. Set `XWALK_CODESCENE_STRICT=true` only after
the CLI is licensed, available, and its baseline behavior is confirmed; then unavailable and failed gates return
nonzero and block the existing Verified gate.

Install the official CLI as the operating-system account that runs `gerrit-ci-control`; the official installer
places the executable at `~/.local/bin/cs`. Store its personal access token only as `CS_ACCESS_TOKEN` in the
mode-0600 `~/.xwalk-ci.env` file. For CodeScene Enterprise, also set `CS_ONPREM_URL` to the licensed server URL.
The hosted codescene.io service does not require `CS_ONPREM_URL`. Restart the CI service after changing this
environment, verify `cs version`, and keep strict enforcement disabled until one real delta returns a valid JSON
gate result. Never put the token in this repository, a Gerrit comment, a command argument, or a retained diagnostic.

If the licensed CodeScene edition supports native Gerrit integration, configure it administrator-side using the
official Gerrit integration guide. Otherwise retain the repository CLI delta when available and rely on compilation,
tests, sanitizers, and static analysis before submit; full CodeScene repository analysis occurs after the accepted
integration commit is mirrored to GitHub.

An integrated `xWalk-rpi5` Gerrit patch set can use its exact commit and first parent directly. A patch set uploaded
to an independent component repository is built and tested from an exact overlay, but its commit is not part of the
integrated MyPiCarX Git history. The adapter therefore reports CodeScene `UNAVAILABLE` for that stage instead of
silently analysing the unrelated integration branch head. Native licensed Gerrit analysis or the subsequent reviewed
uplift supplies the supported integrated-history analysis.

## Gerrit-to-GitHub correlation

Only the submitted `xWalk-rpi5/master` integration commit is eligible for GitHub synchronization.
Component repositories
are never pushed independently. The worker first confirms that the submitted revision is the current patch set with
the CI account's `Verified +1`, then performs a non-force fast-forward push to GitHub. Existing guards prevent a
GitHub-to-Gerrit loop.

For a patch set, `summary.json` records `GERRIT_CHANGE_NUMBER`, `GERRIT_PATCHSET_NUMBER`,
`GERRIT_PATCHSET_REVISION`, and `GERRIT_REFSPEC`. For GitHub, it records the base, head, and workflow commit IDs. The
submitted Gerrit revision is the integrated commit; the successful push makes the same object ID the GitHub commit
SHA. A CodeScene-provided analysis link is retained with that revision. Credentials from either system are never
included in this correlation record.

## Local operation

Validate mappings and exclusions:

```bash
xWalkTool/py-agent/dev-tool/xWalkCodeHealth validate-config
```

Run a non-blocking delta with the installed licensed CLI:

```bash
XWALK_CODESCENE_BASE_REVISION=HEAD^ XWALK_CODESCENE_REVISION=HEAD XWALK_CODESCENE_STRICT=false xWalkTool/py-agent/dev-tool/xWalkCodeHealth analyze
```

Run the standard host build and tests independently of CodeScene:

```bash
cmake --fresh -S xWalk-rpi5 -B build-host/codescene-validation -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build-host/codescene-validation --parallel
ctest --test-dir build-host/codescene-validation --output-on-failure --no-tests=error
```

## Troubleshooting

| Symptom | Resolution |
|---|---|
| Missing token | Native App credentials belong in CodeScene, not workflow YAML. For CLI activation, follow the licensed CLI procedure and approved secret store. |
| Invalid project ID | Verify the project selected in the CodeScene UI or documented API; the repository adapter deliberately has no project-ID option. |
| Unreachable server | Inspect `diagnostic.txt`, verify CodeScene availability and runner routing, and keep rollout non-blocking until stable. |
| Missing Git history or shallow clone | Use `fetch-depth: 0`; verify both full revision IDs with `git cat-file -e COMMIT^{commit}`. |
| Wrong Gerrit patch set | Compare the summary revision/refspec with the event and `git rev-parse HEAD`; rerun the current patch set, not branch head. |
| Incorrect GitHub commit | Compare `GITHUB_HEAD_SHA`, `GITHUB_SHA`, and the submitted Gerrit revision; reject any non-fast-forward synchronization. |
| CLI executable unavailable | Install it for the CI OS account; the adapter also checks `~/.local/bin/cs`. |
| CLI activation unavailable | Put `CS_ACCESS_TOKEN` and optional `CS_ONPREM_URL` only in the CI environment. |
| Metadata-only change returns no JSON | The adapter passes when no changed file belongs to a configured architectural component; it does not invoke the CLI for `.gitmodules`-only changes. |
| No analysis link | Local/offline CLI output may not supply a link; use the retained JSON and commit correlation instead of inventing a URL. |

## Rollback

Set `XWALK_CODESCENE_STRICT=false` first so an external outage cannot block review. Remove the Code Health dependency
from the GitHub and Gerrit graph only through a reviewed revert, and disable the native CodeScene GitHub/Gerrit
integration administrator-side. Keep compilation, tests, sanitizers, static analysis, security checks, and
synchronization guards unchanged. Removing `.codescene` manifests alone does not roll back a server-side CodeScene
project configuration; restore that configuration separately using the CodeScene administrator audit history.
