# Gerrit CI configuration

## Architecture

xWalk uses two independent Host Quality integrations:

- GitHub Actions reads `.github/workflows/host-quality.yml` for GitHub push and pull-request events.
- Gerrit Code Review delegates patch-set and gate execution to an externally installed Zuul service.
- Zuul reads the repository-owned `.zuul.yaml` and executes the Ansible playbooks under `xWalkTool/shell-agent/env-tool/playbooks/zuul`.
- Both integrations call `xWalkTool/shell-agent/gerrit-tool/run-host-ci-job.sh`, which owns the substantive host-safe commands.

Gerrit does not execute GitHub Actions YAML. Committing `.zuul.yaml` also does not install Zuul, Nodepool, an
executor, a Gerrit connection, or a log server. A Gerrit or Zuul administrator must provide those services.

The previous local Gerrit event worker must not vote alongside Zuul. Disable that worker after Zuul is connected
and verified, otherwise two services can race to report different `Verified` votes for the same patch set.

## Repository-controlled job graph

The `xwalk-shellcheck` and `xwalk-deployment-scripts` jobs run first. The four GCC/Clang Debug/Release build jobs
depend on both. Sanitizer, stress, scenario, streaming, fuzz, soak, analysis, coverage, Valgrind, and installation
jobs depend on all four build jobs. `xwalk-host-quality-gate` has hard `dependencies:` on every preceding job.

Zuul runs a job only after all hard dependencies complete successfully. An upstream failure marks dependent jobs
as skipped, so the final gate cannot run or report success after a required failure. The gate playbook performs a
lightweight repository metadata check; dependency processing is the authoritative aggregation mechanism.
`parent:` is not used for ordering.

Every job is registered in both the `check` and `gate` project pipelines. No repository configuration submits a
change automatically.

## Ubuntu 24.04 node image

The repository defines the `xwalk-ubuntu-24.04` nodeset with the external label
`ubuntu-24.04-xwalk-ci`. The Zuul administrator must map that label to a maintained Ubuntu 24.04 image containing
the same compiler, CMake, Ninja, library, sanitizer, analyzer, coverage, Valgrind, ShellCheck, Python, Protobuf,
gRPC, OpenCV, and Ansible dependencies used by GitHub Host Quality.

Jobs never use interactive or password-based `sudo`. Package installation belongs in the node image or in a
trusted administrator-owned base job. TSan and LSan must be allowed to initialize, and loopback sockets must be
available for host streaming tests. No node may expose Raspberry Pi or Robot HAT devices to these jobs.

The administrator-owned base job should run the standard Zuul `fetch-output` log collection. Repository
post-playbooks stage available coverage, analyzer, Valgrind, soak, and staged-install reports below
`$HOME/zuul-output/artifacts` for that mechanism.

## Service account and Gerrit permissions

Create a dedicated `xwalk-ci` Gerrit service account. Register only its public SSH key and store the private key in
the Zuul connection secret store, outside this repository. Grant only:

- read access to the xWalk project and its patch-set refs;
- event-stream access required by the Gerrit driver;
- `Verified -1..+1` on the project;
- no project ownership, force push, branch deletion, or server administration.

The recommended result mapping is `Verified +1` when every required job succeeds and `Verified -1` when any
required job fails. Configure a Gerrit submit requirement that requires the current patch set to hold a successful
CI `Verified +1` vote before submission.

## Administrator-side Zuul configuration

Pipeline definitions belong in a trusted Zuul config project, not this untrusted product repository. The following
is an administrator-side example; replace `gerrit` with the configured connection name and adapt the gate approval
to the reviewed local policy:

```yaml
- pipeline:
    name: check
    manager: independent
    trigger:
      gerrit:
        - event: patchset-created
        - event: wip-state-changed
    success:
      gerrit:
        Verified: 1
    failure:
      gerrit:
        Verified: -1

- pipeline:
    name: gate
    manager: dependent
    trigger:
      gerrit:
        - event: comment-added
          approval:
            - Code-Review: 2
    success:
      gerrit:
        Verified: 1
    failure:
      gerrit:
        Verified: -1
```

The `patchset-created` event validates each active upload in `check`. If the Gerrit driver filters WIP changes,
configure the WIP-to-active event according to the installed Gerrit and Zuul versions. The reviewed gate approval
event enqueues the change in `gate` before submission. Do not add a submit reporter or automatic submission unless
the project owner separately authorizes that server-side policy.

The administrator must also:

1. Install and operate Zuul, ZooKeeper, an executor, scheduler, web service, and an approved node provider.
2. Configure the Gerrit connection and service-account key without exposing it to job nodes or logs.
3. Add the xWalk Gerrit project to the tenant and allow repository `.zuul.yaml` configuration.
4. Provide the `ubuntu-24.04-xwalk-ci` node label and a base job that copies the speculative checkout to the node.
5. Make that base job collect `$HOME/zuul-output` through Zuul's standard `fetch-output` mechanism.
6. Validate the `check` and `gate` event filters, `Verified` reporting, and submit requirement on a test change.
7. Rotate the service key through the Zuul secret store and Gerrit account without changing repository files.

## Host safety

All jobs operate on the Zuul-provided speculative checkout. They use simulations, recorded fixtures, loopback
networking, and the controller's `--diagnose --no-hardware` mode. They never run physical Raspberry Pi, GPIO,
camera commissioning, motor, servo, sensor, or Robot HAT tests.

GitHub synchronization remains a separate post-submit integration service. Zuul Host Quality does not push to
GitHub and contains no GitHub, Gerrit, SSH, API, or password credentials.
