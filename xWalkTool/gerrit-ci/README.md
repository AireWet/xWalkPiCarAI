# xWalk Gerrit CI

This directory defines the dedicated host-verification runner for the local
`xWalkPiCarAI` Gerrit project. The runner listens only for `patchset-created`
events on `master`, fetches the exact Gerrit patch-set ref into an isolated
temporary checkout, runs the strict Debug host build and complete CTest suite,
and reports `Verified +1` or `Verified -1` on that patch set.

The runner uses a dedicated `xwalk-ci` Gerrit account in the `Service Users`
group. Its project-scoped permission is limited to `Verified -1..+1`; it cannot
submit changes. The private SSH key remains on the host and is mounted
read-only into the container. Never add that key to this repository.

Build and start the persistent runner from the repository root:

```bash
docker build --tag xwalk-gerrit-ci:local xWalkTool/gerrit-ci
docker volume create xwalk-gerrit-ci-state
docker volume create xwalk-gerrit-ci-logs
docker run --detach --name xwalk-gerrit-ci --restart unless-stopped --network host --mount type=bind,source=$HOME/.ssh/id_ed25519_xwalk_ci,target=/run/secrets/gerrit_ssh_key,readonly --mount type=volume,source=xwalk-gerrit-ci-state,target=/var/lib/xwalk-gerrit-ci --mount type=volume,source=xwalk-gerrit-ci-logs,target=/var/log/xwalk-gerrit-ci xwalk-gerrit-ci:local
```

Inspect service and verification output with:

```bash
docker logs --follow xwalk-gerrit-ci
docker exec xwalk-gerrit-ci find /var/log/xwalk-gerrit-ci -type f -maxdepth 1 -print
```

Uploading a new patch set automatically starts verification. An informational
Gerrit message is posted at start, followed by the final `Verified` vote. Logs
remain in the `xwalk-gerrit-ci-logs` Docker volume across container restarts.
