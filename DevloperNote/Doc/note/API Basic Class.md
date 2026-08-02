# `XWalkTrace`

`XWalkTrace` provides filtered diagnostic output through an injected callback.
It replaces implicit console logging with an explicit application-owned sink.

## Public interface

The authoritative declarations are in
[`xWalkTrace`](../../../xWalkHal/xWalkTrace/README.md).

The application configures the active severity and supplies the callback
context. The trace object does not own the destination and does not write to a
platform stream unless the application callback chooses to do so.
