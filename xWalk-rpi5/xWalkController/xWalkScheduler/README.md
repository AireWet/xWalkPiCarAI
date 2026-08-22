# xWalkScheduler

`xWalkScheduler` is the planned Controller component for coordinating commands
that must run at a requested time or on a bounded recurring schedule. This
directory currently contains documentation only; no scheduler target, runtime
service, or CLI command has been implemented.

## Intended responsibilities

- Accept validated, typed scheduling requests from the application layer.
- Order pending work using monotonic time for runtime intervals.
- Keep queue capacity, recurrence, and execution duration explicitly bounded.
- Dispatch due work through existing Controller handlers without duplicating
  command behavior.
- Support cancellation and deterministic shutdown before dependencies are
  destroyed.
- Report scheduling outcomes through the Controller trace boundary.

The scheduler must not own hardware or bypass the existing `xWalkApp`,
`xWalkHandler`, Agent, and HAL safety boundaries. Any future command that can
move actuators or enable physical outputs must retain the same configuration,
preflight, cancellation, and emergency-stop requirements as direct execution.

## Proposed layout

| Path | Responsibility |
| --- | --- |
| `README.md` | Scheduler scope, safety boundaries, and planned structure |
| `include/` | Reserved for public scheduling contracts and typed request definitions |
| `src/` | Reserved for queue, timing, dispatch, and cancellation implementation |
| `test/` | Future deterministic host tests using injected time and callbacks |

The tracked `include` and `src` placeholders contain no production code. Add
their first implementation files only when the scheduler design, public
contract, CMake target, and host verification plan are approved.
