# Gerrit CI configuration

## Active architecture

The local xWalk Gerrit service uses the repository-owned Python worker, not Zuul. `xWalkGerritCi.py` maintains a
noninteractive SSH `gerrit stream-events` connection, selects active `patchset-created` and WIP-to-active events,
fetches the exact patch-set ref into a temporary checkout, and invokes `xWalkGerritQuality.py`. The runner calls
`xWalkTool/shell-agent/gerrit-tool/run-host-ci-job.sh`, which is also used by GitHub Actions.

`xWalkGerritLogServer.py` serves the existing read-only dashboard through Caddy at
`/ci/changes/<change>/<patch-set>`. Each run retains:

- one complete redacted log named `change-<change>-<patch-set>-<timestamp>.log`;
- one atomic structured-state sidecar with module/check status, dependencies, timestamps, and durations;
- stable module routes below `/ci/changes/<change>/<patch-set>/jobs/<module>`.

The page and dependency connectors are server-rendered HTML and CSS. No JavaScript, client graph library, external
CDN, or separate dashboard service is used. Historical logs without a structured sidecar retain raw and complete
log access.

The repository also retains `.zuul.yaml` and example playbooks for a possible administrator-managed Zuul
deployment. They are not used by the current worker and do not install or activate Zuul. Never run Zuul and the
Python worker against the same project because competing services can race to vote on one patch set.

## Module dependency graph

GitHub Actions controls the appearance of its native rectangular job nodes. The Gerrit page renders the equivalent
three-stage graph:

```text
xWalk Preparation -> xWalkAgent      -> xWalk Host Quality Gate
                  -> xWalkController ->
                  -> xWalkHal        ->
                  -> xWalkIW         ->
                  -> xWalkLibrary    ->
                  -> xWalkTrace      ->
                  -> xWalk Vision    ->
                  -> xWalk Streaming ->
                  -> xWalk Quality   ->
                  -> xWalk Deployment ->
                  -> MyPiCarX / Code Health ->
```

Preparation runs first. The eleven independent module nodes then run with bounded parallelism. The xWalk Quality node
runs its compiler, sanitizer, static-analysis, runtime-analysis, and coverage checks sequentially so incompatible
instrumentation never shares one build. The gate passes only when every required node passes. A failed Preparation
marks dependent modules `SKIPPED`; failed, cancelled, skipped, missing, or malformed results cannot produce a
passing gate.

The dashboard supports `WAITING`, `PENDING`, `QUEUED`, `RUNNING`, `PASSED`, `FAILED`, `SKIPPED`, `CANCELLED`, and
`UNAVAILABLE`.
Every graph node links to its module details and retained output. The raw full-log link and complete log remain on
the main page.

## Module-to-test mapping

| Module | Owned checks |
|---|---|
| xWalk Preparation | ShellCheck, CI YAML and Ansible metadata validation |
| xWalkAgent | Aggregate tests, functional-group tests, configuration and communication handlers |
| xWalkController | Controller lifecycle, CLI, Controller-to-HAL sequences, `--diagnose --no-hardware` |
| xWalkHal | Unit and host-safe sequences, interface/device/sensor/layer groups, simulations, Robot HAT soak |
| xWalkIW | Schema, serialization, gRPC, signal/payload, and transport-interface coverage |
| xWalkLibrary | Shared utilities, configuration, licence, architecture, and direct-consumer integration |
| xWalkTrace | Formatting, routing, error/warning macros, and error-signal selection |
| xWalk Vision | Recorded media/scenarios, OpenCV, road-user safety, assets, annotations, and checksums |
| xWalk Streaming | Loopback HTTP, MJPEG, limits, lifecycle, timeouts, slow clients, and backpressure |
| xWalk Quality | Compiler builds, sanitizers, analysis, stress, fuzz, Valgrind, and coverage |
| xWalk Deployment | Deployment/provisioning scripts, staged install, resources, linkage, permissions, checksums |
| MyPiCarX / Code Health | Exact-revision CodeScene CLI delta when available, changed-file component mapping, and rollout policy |

The xWalk Quality node keeps the graph module-oriented without reducing coverage. Every prior compiler, sanitizer,
runtime-analysis, coverage, fuzz, stress, analyzer, and timeout selection remains in the shared dispatcher and is
reported as an individual test result on the node's detail page.

## Original-job mapping

| Previous flat job or GitHub node | Current module or quality group |
|---|---|
| `gerrit-host` | Product modules plus the xWalk Quality GCC Debug check |
| `gcc Debug`, `gcc Release`, `clang Debug`, `clang Release` | xWalk Quality |
| `asan-ubsan`, `leak-sanitizer`, `sanitizers`, `thread-sanitizer` | xWalk Quality |
| `stress-tests`, `fuzz-smoke`, `valgrind` | xWalk Quality |
| `static-analysis`, `clang-static-analyzer` | xWalk Quality |
| `coverage` | xWalk Quality |
| `recorded-scenarios` | xWalk Vision |
| `streaming` | xWalk Streaming |
| `soak-smoke` | xWalkHal |
| `shellcheck` | xWalk Preparation |
| `deployment-scripts`, `staged-install` | xWalk Deployment |

## Gerrit result calculation

After checkout and any component standalone validation, the worker runs the complete module graph. It reports
`Verified +1` only when Preparation, every product module, every quality group, Deployment, Code Health under its
configured enforcement policy, and the final gate
pass. Any checkout, standalone, module, or gate failure reports `Verified -1`. The worker posts the overall
dashboard link when verification starts. After execution, it posts one uniquely tagged Gerrit change-log entry
for Preparation and each module, then posts the Host Quality Gate as the single entry that carries the final
`Verified` vote. By default it does not submit. When the guarded integrated
submit policy is enabled, it re-queries the current `xWalkPiCarAI` patch set and
requests submission only after Gerrit's complete submit record is `OK`.

Grant the `xwalk-ci` service account only:

- read access to the project and patch-set refs;
- permission to consume the Gerrit event stream;
- `Verified -1..+1` on the project.
- integration-review upload and submit permission on `xWalkPiCarAI/master`.

Keep ownership, direct branch push, force-push, `Code-Review`, passwords,
private keys, and API tokens out of the CI identity and repository. Configure
the current-patch-set requirements documented in the
[integrated uplift workflow](Integrated%20Uplift%20Workflow.md).

## Opening results and logs

Open `/ci/changes/<change>/<patch-set>` for the graph, overall status, module details, complete log, and raw-log
link. Select any module node or its **Open module log** link for that module's individual check states and output.
Keyboard focus is visible, status is communicated by text and icon as well as colour, and the graph stacks
vertically on narrow displays.

## Adding a module or check

To add a real validation group:

1. Add a dispatcher selection to `run-host-ci-job.sh` that returns nonzero on failure and remains host-safe.
2. Add the check to the matching `XWalkModulePlan`, or add a new module plan with `needs=("preparation",)`.
3. Add the same GitHub job or named step and dependency in `.github/workflows/host-quality.yml`.
4. Add the module display name to `XWalkGerritLogServer.MODULES` when introducing a new graph node.
5. Add runner, dependency, gate, rendering, escaping, and module-route tests.
6. Update this mapping only for a real module or meaningful validation group; never add an empty display job.

## Installation and operation

The installer copies the three CI Python programs into `$HOME/apps/gerrit/tools`. After the protected
`$HOME/.xwalk-ci.env` file exists, the Gerrit start, stop, status, and restart controls include the CI worker.
Use the CI-specific controls for direct recovery and inspection:

```bash
"$HOME/bin/gerrit-ci-control" start
"$HOME/bin/gerrit-ci-control" status
"$HOME/bin/gerrit-ci-logs"
```

When `GERRIT_PROCESS_MANAGER=systemd`, the control launches a transient
user-level service with failure restart and waits for both the worker process
and its loopback health endpoint. The portable `nohup` mode applies the same
health gate without creating a service.

Verify the HTTPS route:

```bash
curl --fail --show-error --silent --cacert "$HOME/gerrit-site/etc/gerrit-self-signed.crt" "https://SERVER:18443/ci/health"
```

Set `XWALK_CADDY_TEST_BINARY` and `XWALK_CADDY_TEST_ARCHIVE` in `$HOME/.xwalk-ci.env` to the installed pinned
Caddy executable and retained verified release archive. These opt-in inputs enable the real configuration,
lifecycle, and extraction tests; they remain skipped on hosts that do not install the local HTTPS proxy.

After a reviewed runner update is submitted, an administrator can reinstall the approved tool assets through the
normal Gerrit installer workflow. For a controlled in-place update, stop the worker, copy the three reviewed Python
files with their existing modes into `$HOME/apps/gerrit/tools`, and restart it. Do not replace files while the
worker is running.

```bash
"$HOME/bin/gerrit-ci-control" stop
install -m 0700 xWalkTool/py-agent/gerrit-tool/py-src/xWalkGerritCi.py "$HOME/apps/gerrit/tools/xWalkGerritCi.py"
install -m 0600 xWalkTool/py-agent/gerrit-tool/py-src/xWalkGerritQuality.py "$HOME/apps/gerrit/tools/xWalkGerritQuality.py"
install -m 0600 xWalkTool/py-agent/gerrit-tool/py-src/xWalkGerritLogServer.py "$HOME/apps/gerrit/tools/xWalkGerritLogServer.py"
"$HOME/bin/gerrit-ci-control" start
"$HOME/bin/gerrit-ci-control" status
```

Administrator-side work remains necessary for the service account, SSH key permissions, `Verified` label range,
submit requirement, Caddy `/ci` proxy, host dependencies, and credential rotation. Repository changes do not alter
those server settings.

## Host safety

All jobs use simulated backends, recorded fixtures, loopback networking, and `--diagnose --no-hardware`. They never
commission or actuate Raspberry Pi GPIO, I²C, SPI, cameras, audio devices, motors, servos, sensors, or a Robot HAT.
