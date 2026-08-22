# xWalk native CBB process scheduler

`xWalkScheduler` is the Controller-owned Linux process supervisor and native
Controller/Agent/HAL signal dispatcher. It is independent of Protobuf, gRPC,
and network-server code. A future transport adapter must convert an external
request to a native payload and call the same CBB send API.

## CBB flow

The send-function name identifies the destination, never the sender:

- `cxx_xWalkCtrlSend_LPP()` sends to Controller.
- `cxx_xWalkAgentSend_LPP()` sends to Agent.
- `cxx_xWalkHalSend_LPP()` sends to HAL.

Consequently, Controller forwards Agent work with
`cxx_xWalkAgentSend_LPP()`, and Agent forwards hardware work with
`cxx_xWalkHalSend_LPP()`.

```text
caller -> Ctrl send -> Controller child -> Agent send -> Agent child
       -> Hal send -> HAL child -> HAL operation
```

Each registered module owns one child PID and one private `AF_UNIX`
`SOCK_SEQPACKET` socket. A child-to-child logical send is returned to the
parent supervisor as a nested native signal and enqueued in the destination
mailbox. Registered native Controller, Agent, and HAL handlers therefore run
through their scheduler mailboxes.

## Native signal interface

`xWalkCbbSignal.h` owns the pointer-free native `XWalkSignal`, plain
`xClientAddress`, fixed binary payload storage, module IDs, and stable signal
registry. Stable `CXX_XWALK_*` constants preserve their existing names and
numeric values. They are identifiers, not callable APIs, so `_LPP` is not
appended to them.

`xWalkCbbApi.h` declares the generic native helpers and the module Handler,
Init, and Shutdown functions. Module-specific Send, GetSignal, SetSignal,
GetClientInfo, SetClientInfo, GetPayload, and SetPayload entry points are thin
macros in `xWalkLibrary/common/include/xControllerSchedulerMacros.h`. They
expand to one generic helper call and evaluate every argument once. Scheduler
lifecycle-only macros remain private to `xWalkScheduler` in
`xControllerSchedulerRuntimeMacros.h`.

Payload setters copy exact binary bytes into fixed 512-byte signal storage.
Getters report the required size, reject insufficient capacity, and preserve
embedded zero bytes. No caller pointer is queued or transmitted. Payload
structures crossing the local fork boundary must be pointer-free and use an
explicit encoding when their representation is not safely copyable.

Module facades are available at:

- `xWalkController/xWalkHandler/include/xControllerCbb.h`
- `xWalkAgent/include/xAgentCbb.h`
- `xWalkHal/include/xHalCbb.h`

## Mailboxes, FIFO, and local indexes

Controller, Agent, and HAL use mailbox IDs `0x2000`, `0x3000`, and `0x1000`.
One mailbox corresponds to one stable process-table slot and one child. Each
mailbox assigns its own increasing nonzero local indexes. A local index
identifies a request; it is never a PID, process slot, or mask bit.

Every mailbox owns a bounded 32-entry FIFO. Only one request may be active for
a mailbox. A validated completion dispatches exactly one next request.
Different mailboxes execute concurrently in different children.

```text
Controller:1 starts
Controller:2 waits
Agent:1 starts in parallel
Controller:1 completes
Controller:2 starts
```

Terminal request records are retained in the fixed 128-entry table. The oldest
terminal record may be evicted with a warning; pending or active records are
never overwritten.

## Responses and process status

Successful request handling maps the exact REQ to its CFM. Failure maps it to
the corresponding REJ and may carry a bounded native `xWalkRejectPayload`.
Mailbox and local index are preserved. Mapping is explicit for exceptional
I2C values and validated for the stable Controller ranges.

Process state and request state are separate. Registered, alive, created,
stopped, starting, running, stopping, exited, and failed masks use the stable
process-table slot as the bit position. Counts are computed from masks;
mailbox IDs, local indexes, requests, and PIDs never select mask bits.

## Lifecycle and cleanup

Start enables dispatch after the child reports Running. Stop pauses dispatch,
lets the active synchronous callback finish, retains queued requests, and then
reports Stopped. Shutdown rejects new work, resolves queued records, asks each
child to stop, polls and reaps during bounded grace periods, and sends
`SIGTERM` only to a validated positive PID owned by the scheduler when needed.
Sockets are closed once, all owned children are reaped, and final alive and
running masks are zero after a successful shutdown.

SIGINT and SIGTERM handlers only set the existing `sig_atomic_t` operation
flag. Cleanup runs in normal application control flow. No broad process-name
termination is used.

## Tracing

Existing Controller trace and error macros cover initialization, registration,
child creation, queue operations, local-index assignment, dispatch, IPC,
completion, lifecycle transitions, reaping, and cleanup. Recoverable stale or
duplicate traffic uses warning severity. Invalid data, handler failures,
system-call failures, forced termination, and cleanup failures use error
severity. Binary payload contents are never logged.

## Build and test

```bash
cmake -S xWalk-rpi5/xWalkController/xWalkScheduler -B build-scheduler -DXWALK_SCHEDULER_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-scheduler --target xWalkSchedulerTest --parallel
ctest --test-dir build-scheduler --output-on-failure
```

The bounded host tests cover stable signal values, binary get/set behavior,
mailbox validation, FIFO execution, parallel children, status masks, cleanup,
and the native Controller-to-Agent-to-HAL chain. They construct no Protobuf
object and require no gRPC server. Physical hardware verification remains
opt-in.
