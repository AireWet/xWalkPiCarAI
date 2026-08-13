# xWalk Gerrit review controls

`xWalkReviewControls.js` is a standalone Gerrit Web UI plugin for the local
`xWalkPiCarAI` review service. It replaces Gerrit's transient **Mark As Active**
action with one persistent review control:

- a WIP change shows an enabled **Activate** button;
- activation marks the change ready for review and starts Gerrit CI;
- an active or merged change shows a disabled **Activated** button with a check
  icon, preserving the pressed state;
- **Submit** remains hidden until Gerrit reports the open change as submittable
  and both `Code-Review` and `Verified` requirements are satisfied.

Gerrit's inherited submit requirements remain the authority for submission.
The plugin only controls presentation and invokes the standard `POST
/changes/{change-id}/ready` REST operation.

Run the host-only JavaScript tests:

```bash
node --test xWalkTool/gerrit-ui/xWalkReviewControlsTest.js
```

Install or update the standalone plugin, then reload the browser page:

```bash
sudo install -o gerrit -g gerrit -m 0644 xWalkTool/gerrit-ui/xWalkReviewControls.js /opt/gerrit/plugins/xWalkReviewControls.js
```

Gerrit detects standalone plugin changes without a server restart. Confirm the
plugin is enabled with `ssh gerrit-airewet gerrit plugin ls`.
