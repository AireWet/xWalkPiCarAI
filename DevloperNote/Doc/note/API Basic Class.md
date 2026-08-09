# `XWalkTrace`

`XWalkTrace` provides filtered diagnostic output through an append-only log file
and an injected callback. It replaces implicit console logging with explicit,
testable sinks.

## Public interface

The authoritative declarations are in
[`xWalkTrace`](../../../xWalkTrace/README.md).

The application configures the active severity and supplies the callback
context. The trace object appends accepted records to `log/xWalkTrace.log`,
relative to the process working directory, and does not own the additional
callback destination.
