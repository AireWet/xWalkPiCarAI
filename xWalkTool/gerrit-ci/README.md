# xWalk Gerrit CI

This directory defines the dedicated host-verification runner for the local
`xWalkPiCarAI` Gerrit project. The runner listens only for `patchset-created`
events on `master`, fetches the exact Gerrit patch-set ref into an isolated
temporary checkout, runs the original Gerrit host suite and every job from the
GitHub Host quality workflow, and reports one aggregate `Verified +1` or
`Verified -1` on that patch set. After Gerrit submits a verified change, the
same service fast-forwards GitHub `master` to the authoritative Gerrit branch.

The aggregate gate runs all of these jobs before voting:

- strict Gerrit Debug build and all registered CTest tests;
- GCC Debug and Release builds, tests, Agent groups and sequence executables;
- Clang Debug and Release builds, tests, Agent groups and sequence executables;
- AddressSanitizer and UndefinedBehaviorSanitizer tests;
- ThreadSanitizer concurrency tests;
- repeated stress tests;
- Clang-Tidy and Cppcheck analysis;
- enforced host coverage;
- ShellCheck and deployment behavior tests;
- staged release installation, resource, linkage, permission and checksum checks.

Jobs run sequentially in one isolated checkout. A failed job stops its own
remaining steps but does not prevent the other jobs from running. `Verified +1`
is emitted only when every job passes; any failed job produces `Verified -1`.
Physical Raspberry Pi and live-provider tests remain excluded.

The runner uses a dedicated `xwalk-ci` Gerrit account in the `Service Users`
group. Its project-scoped permission is limited to `Verified -1..+1`; it cannot
submit changes. The private SSH key remains on the host and is mounted
read-only into the container. Never add that key to this repository.

GitHub mirroring uses a separate repository-scoped deploy key with write access
only to `jochuuu/xWalkPiCarAI`. The mirror never force-pushes. A divergent
GitHub branch therefore fails safely and remains unchanged.

Store all host-specific runner settings in `~/.xwalk-ci.env`. This file contains
connection values and credential paths, not private-key contents:

```dotenv
XWALK_CI_IMAGE=xwalk-gerrit-ci:local
XWALK_CI_CONTAINER=xwalk-gerrit-ci
XWALK_CI_GERRIT_KEY_SOURCE=/home/jaison/.ssh/id_ed25519_xwalk_ci
XWALK_CI_GITHUB_KEY_SOURCE=/home/jaison/.ssh/id_ed25519_xwalk_github_mirror
XWALK_CI_STATE_VOLUME=xwalk-gerrit-ci-state
XWALK_CI_LOG_VOLUME=xwalk-gerrit-ci-logs
XWALK_CI_SECURITY_OPT=seccomp=unconfined
GERRIT_HOST=127.0.0.1
GERRIT_SSH_PORT=29418
GERRIT_USER=xwalk-ci
GERRIT_PROJECT=xWalkPiCarAI
GERRIT_BRANCH=master
GERRIT_SSH_KEY=/run/secrets/gerrit_ssh_key
GITHUB_HOST=github.com
GITHUB_REPOSITORY=jochuuu/xWalkPiCarAI
GITHUB_WEB_URL=https://github.com/jochuuu/xWalkPiCarAI
GITHUB_SSH_KEY=/run/secrets/github_mirror_ssh_key
XWALK_CI_STATE_DIRECTORY=/var/lib/xwalk-gerrit-ci
XWALK_CI_LOG_DIRECTORY=/var/log/xwalk-gerrit-ci
```

Restrict the file even though it contains no secret values:

```bash
chmod 600 "$HOME/.xwalk-ci.env"
```

Build and start the persistent runner from the repository root:

```bash
set -a
source "$HOME/.xwalk-ci.env"
set +a
docker build --tag "$XWALK_CI_IMAGE" xWalkTool/gerrit-ci
docker volume create "$XWALK_CI_STATE_VOLUME"
docker volume create "$XWALK_CI_LOG_VOLUME"
docker run --detach --name "$XWALK_CI_CONTAINER" --restart unless-stopped --network host --security-opt "$XWALK_CI_SECURITY_OPT" --env-file "$HOME/.xwalk-ci.env" --mount type=bind,source="$XWALK_CI_GERRIT_KEY_SOURCE",target="$GERRIT_SSH_KEY",readonly --mount type=bind,source="$XWALK_CI_GITHUB_KEY_SOURCE",target="$GITHUB_SSH_KEY",readonly --mount type=volume,source="$XWALK_CI_STATE_VOLUME",target="$XWALK_CI_STATE_DIRECTORY" --mount type=volume,source="$XWALK_CI_LOG_VOLUME",target="$XWALK_CI_LOG_DIRECTORY" "$XWALK_CI_IMAGE"
```

The non-root runner uses `seccomp=unconfined` only so `setarch` can disable
address randomization for ThreadSanitizer on the host kernel. TSan otherwise
terminates before executing a test with an unexpected-memory-mapping error.

Inspect service and verification output with:

```bash
docker logs --follow xwalk-gerrit-ci
docker exec xwalk-gerrit-ci find /var/log/xwalk-gerrit-ci -type f -maxdepth 1 -print
```

Uploading a new patch set automatically starts verification. An informational
Gerrit message is posted at start, followed by the aggregate job summary and
final `Verified` vote. Logs remain in the `xwalk-gerrit-ci-logs` Docker volume
across container restarts.
Submitted changes also receive an informational message stating whether the
fast-forward GitHub mirror succeeded. The Gerrit change log records the branch,
retained mirror log, and a clickable link to the exact GitHub commit.
