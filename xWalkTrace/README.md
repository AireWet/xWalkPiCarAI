# xWalkTrace

`xWalkTrace` is the C++17 trace service shared by HAL and Controller code. It
provides build-validated identifiers, runtime XML filtering, automatic source
locations, UTC timestamps, monotonic elapsed time, and synchronized append-only
output to `log/xWalkTrace.log`.

## Tagged trace API

HAL traces use `RPI.<number>` and Controller traces use `CTRL.<number>`:

```cpp
XWALK_HAL_TRACE_UID0(
    RPI.1001,
    "Critical HAL trace");

XWALK_HAL_TRACE_UID1(
    RPI.1002,
    "HAL value: %u",
    value);

XWALK_HAL_TRACE_UID2(
    RPI.1003,
    "HAL state: %s",
    state);

XWALK_HAL_TRACE_UID3(
    RPI.1004,
    "HAL diagnostic");

XWALK_CTRL_TRACE_UID0(
    CTRL.2001,
    "Critical CTRL trace");

XWALK_CTRL_TRACE_UID1(
    CTRL.2002,
    "CTRL value: %u",
    value);

XWALK_CTRL_TRACE_UID2(
    CTRL.2003,
    "CTRL state: %s",
    state);

XWALK_CTRL_TRACE_UID3(
    CTRL.2004,
    "CTRL diagnostic");
```

The identifier grammar is `^(RPI|CTRL)\.[0-9]+$`. Leading zeros are preserved
and are significant, so `RPI.001`, `RPI.01`, and `RPI.1` are three different
valid UIDs. This convention supports the boot option's fixed-width UID form.

The complete UID must be unique across every scanned source file, macro, and
priority. `RPI.1001` and `CTRL.1001` are different complete UIDs and may both
exist. HAL macros reject `CTRL` tags and Controller macros reject `RPI` tags.

The macro suffix is the priority. Priority `0` is highest and priority `3` is
lowest. The runtime uses four independent priority flags rather than a
threshold. A tagged record is emitted only when its priority flag and its UID
flag are both enabled. Freshly generated XML disables every priority and UID
by default. Missing UIDs are disabled. Because the macro performs
the enable check before calling the formatter, arguments of disabled traces are
not evaluated.

## Warnings, errors, and assertions

Warnings and errors have no UID and bypass priority and UID filtering:

```cpp
XWALK_HAL_WARNINGS("HAL warning: %d", warningCode);
XWALK_HAL_ERROR("HAL error: %d", errorCode);
XWALK_HAL_ASSERT(100);

XWALK_CTRL_WARNINGS("CTRL warning: %d", warningCode);
XWALK_CTRL_ERROR("CTRL error: %d", errorCode);
XWALK_CTRL_ASSERT(200);
```

An assertion signal must have an integral C++ type. The macro rejects string
and nonnumeric values with a compile-time assertion where the compiler can
form the expression. Assertion signals are diagnostic numbers: the current
module records `signal=<number>` and does not raise an operating-system signal
or terminate the process. There was no earlier xWalkTrace assertion callback or
termination behavior to preserve.

## Always-on verbose trace

`XWALK_VERBOSE` writes unconditionally and has no UID or priority filter:

```cpp
XWALK_VERBOSE("Camera frame: %u", frameNumber);
XWALK_VERBOSE("Controller boot completed");
```

It produces a `[TRACE] [VERBOSE]` record even when every XML priority and UID
is disabled or the XML configuration is missing. Like the other public macros,
it captures the caller's filename and line and includes UTC wall time and
monotonic elapsed time. Because it is never filtered, its formatting arguments
are always evaluated.

## Source location and log format

Every public macro captures `__FILE__` and `__LINE__`. Multiline calls therefore
record the line containing the public macro name. Production log output keeps
only the source basename so it cannot expose an absolute build-machine path.
Generated XML keeps a project-relative source path. For tagged traces, the
generated filename and invocation line are canonical at runtime. This keeps
multiline macro locations identical across GCC and Clang; compiler-provided
macro locations remain the fallback for older XML without metadata.

One tagged record has this form:

```text
2026-08-09 12:45:10.120Z [T+12.345678s] [HAL] [P0] [RPI.1001] [camera.cpp:42] Camera ready
```

Warning, error, and assertion categories replace the priority and omit the UID:

```text
2026-08-09 12:45:11.100Z [T+13.325487s] [HAL] [WARNING] [camera.cpp:87] Camera delayed
2026-08-09 12:45:11.200Z [T+13.425612s] [CTRL] [ERROR] [controller.cpp:102] State failed
2026-08-09 12:45:11.300Z [T+13.525719s] [HAL] [ASSERT] [camera.cpp:120] signal=100
```

The wall-clock field uses UTC and millisecond precision; `Z` makes the timezone
explicit. `[T+<seconds>.<microseconds>s]` uses `std::chrono::steady_clock` and
measures elapsed time since trace initialization or the latest explicit global
reconfiguration. It does not measure the duration of the calling operation.

Timestamp capture and each append/flush operation occur under synchronization.
All macro threads use the same process-wide start point and cannot interleave
log records. The compatibility callback runs after the file lock is released.

## Build-time scanner and XML

The class-based
`pre-compiler/xHal_Rpi5CarTracePreCompiler.py` runs before `xWalkTrace`
compiles. It scans the
explicit `xWalkTrace`, `xWalkHal`, `xWalkController`, and `xWalkAgent` roots and
ignores build trees, generated code, external code, comments, string literals,
and macro definitions. Its balanced parser supports multiline calls, nested
parentheses, and multiple formatting arguments.

The build fails for malformed identifiers, incorrect component/tag mappings,
unsupported priorities, duplicate complete UIDs, scanner errors, or XML
generation errors. The deterministic output is:

```text
<build-directory>/generated/xWalkTrace.xml
```

Each XML trace contains component, tag, numeric ID, complete UID, priority,
enabled state, format, macro, relative filename, and invocation line. Runtime
timestamps are not stored in XML.

New UIDs default to `enabled="false"`. On regeneration, known UID flags and all
four priority flags are preserved, removed UIDs disappear, and moved traces
receive updated source metadata. Identical XML content is not rewritten.

Example configuration fragment:

```xml
<priorities>
    <priority level="0" enabled="false"/>
    <priority level="1" enabled="false"/>
    <priority level="2" enabled="false"/>
    <priority level="3" enabled="false"/>
</priorities>
<traces>
    <trace
        component="HAL"
        tag="RPI"
        id="1001"
        uid="RPI.1001"
        priority="0"
        enabled="true"
        file="xWalkHal/xWalkCamera/src/camera.cpp"
        line="42"
        format="Camera ready"
        macro="XWALK_HAL_TRACE_UID0"/>
</traces>
```

The Controller optionally applies repeatable `--trace-enable UID` and
`--trace-disable UID` global options during startup, before the backend boot
graph is constructed. Both separated and assignment forms are supported:

```bash
xwalk-picarx-control --trace-enable RPI.001
xwalk-picarx-control --trace-disable=CTRL.2001 doctor
```

A normal command requires no trace argument. A trace-only invocation updates
XML and exits successfully when that UID is
present in the build-generated metadata. The public C++
entry points are `XWalkTrace::enableGlobalTrace(uid)` and
`XWalkTrace::disableGlobalTrace(uid)`. They return a Boolean status, require a
well-formed UID already present in generated XML, save the new flag, and update
the initialized lookup without reparsing XML on each trace call. Unknown UIDs
return `false` instead of creating metadata that was not found by the
pre-compiler. This startup path does not use C++ `try` or `catch` blocks.

Manual XML changes are loaded once when the process-wide trace instance
initializes; they are not parsed for every call. Restart the application for
manual edits to take effect. Tests and application composition code may
deliberately reload a configuration and select a log path with
`XWalkTrace::configureGlobal()`.

All implementation filenames follow the repository's `xHal_Rpi5Car...`
module convention. `CMakeLists.txt` and `README.md` retain their standard build
and documentation names.

Missing or malformed XML produces a clear trace-system warning or error and
keeps all tagged priorities and UIDs disabled. Warnings, errors, and assertions
remain available under those safe defaults.

## Compatibility object API

The existing `XWalkTrace` constructors, typed level selection, `critical()`,
`error()`, `warning()`, `info()`, `debug()`, and synchronous callback remain
available. These compatibility methods retain the original critical-through-
debug threshold and now use the richer timestamped log format. New code should
use the component macros when build metadata and XML filtering are required.

## Build and test

The module requires Python 3 for build-time metadata generation and TinyXML2
for one-time runtime configuration loading.

From the repository root:

```bash
cmake -S xWalkTrace -B xWalkTrace/build-host \
    -DXWALK_TRACE_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-host --parallel
ctest --test-dir xWalkTrace/build-host --output-on-failure
```

The C++ runtime test and Python scanner test are host-only. Hardware compilation
is available without execution:

```bash
cmake -S xWalkTrace -B xWalkTrace/build-rpi \
    -DXWALK_TRACE_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-rpi --parallel
ctest --test-dir xWalkTrace/build-rpi -N -L hardware
```
