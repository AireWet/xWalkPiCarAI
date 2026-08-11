# xWalkTrace

`xWalkTrace` is the C++17 trace service shared by HAL, Controller, Agent, and
Library sources. Tagged traces are build-validated, filtered before
format arguments are evaluated, and written synchronously to the terminal and
`<build-directory>/log/xWalkTrace.log` with UTC time, monotonic elapsed time,
UID, and source location. Both destinations receive the same completely
formatted record.

## Trace declarations

Each source tree owns a distinct macro and UID family:

| Source tree | Macro family | UID tag |
| ----------- | ------------ | ------- |
| `xWalkHal` | `XWALK_HAL_TRACE_UIDn` | `RPI.<numeric-id>` |
| `xWalkController` | `XWALK_CTRL_TRACE_UIDn` | `CTRL.<numeric-id>` |
| `xWalkAgent` | `XWALK_RPIAGENT_TRACE_UIDn` | `RPIAGENT.<numeric-id>` |
| `xWalkLibrary` | `XWALK_LIB_TRACE_UIDn` | `LIB.<numeric-id>` |

```cpp
XWALK_HAL_TRACE_UID1(RPI.001, "I2C probe: %u", address);
XWALK_CTRL_TRACE_UID0(CTRL.001, "Controller command execution started");
XWALK_RPIAGENT_TRACE_UID0(RPIAGENT.001, "Agent initialized");
XWALK_LIB_TRACE_UID0(LIB.001, "Library operation completed");
```

The numeric value must be unique within each trace tag across the complete
repository, including modules, submodules, and tests. The same numeric value is
valid in different tags. `RPI.001` and `RPI.1` conflict because they represent
the same numeric value. The scanner also rejects a macro family used from the
wrong owned source tree. The `UIDn` suffix is the exact number of formatting
arguments after the UID and format string. Variants `UID0` through `UID5` are
supported. Per-UID record priorities remain separately defined in
`config/xWalkTracePriorities.json` and do not bypass runtime state.

Warnings, errors, and numeric assertions remain intentionally unconditional:

```cpp
XWALK_HAL_WARNING(XWALK_RANGE, "HAL warning: %d", warningCode);
XWALK_HAL_ERROR(XWALK_RUNTIME, "HAL operation failed");
XWALK_HAL_ASSERT(100);
XWALK_CTRL_ERROR(XWALK_INVAL, "Controller input is invalid");
XWALK_RPIAGENT_WARNING(XWALK_SYSTEM, "Agent warning: %d", warningCode);
XWALK_LIB_ERROR(XWALK_RUNTIME, "Library operation returned status %d", errorCode);
```

Each component has one variadic `ERROR` and one variadic `WARNING` macro. Both
accept one selector from `xHal_Rpi5CarErrorSignals.h`, a format string, and
optional printf-style arguments. C++ selectors are
`XWALK_INVAL`, `XWALK_RANGE`, `XWALK_LENGTH`, `XWALK_DOMAIN`, `XWALK_LOGIC`,
`XWALK_RUNTIME`, `XWALK_OVERFLOW`, `XWALK_UNDERFLOW`, `XWALK_SYSTEM`,
`XWALK_ALLOC`, `XWALK_CAST`, `XWALK_TYPEID`, `XWALK_FUNCTION`,
`XWALK_OPTIONAL`, `XWALK_VARIANT`, `XWALK_WEAKPTR`, and `XWALK_EXCEPTION`.
The macros evaluate formatting arguments once and write the selected category,
selector tag, caller location, and formatted message. An `ERROR` with a C++
selector throws that exception after logging. An `ERROR` with an operating-system
signal selector reports the observed condition without raising the signal again.
`XWALK_EXCEPTION` records a generic non-throwing error for callers that return a
status or continue explicitly. Warnings never throw or raise a signal.

Operating-system signal selectors are separate: `XWALK_ABORT`, `XWALK_FLOAT`,
`XWALK_ILL`, `XWALK_SEGV`, `XWALK_TERM`, `XWALK_INT`, `XWALK_PIPE`,
`XWALK_HANG`, and `XWALK_TRAP`. They resolve through namespace-backed signal
constants. Use them with `ERROR` or `WARNING` only when reporting the matching
operating-system signal. `XWALK_PIPE`, `XWALK_HANG`, and `XWALK_TRAP` are
available only when the platform headers provide `SIGPIPE`, `SIGHUP`, and
`SIGTRAP`, respectively.

The legacy `XWALK_VERBOSE` name remains as a disabled no-op for source
compatibility. Production normal diagnostics must use a registered UID macro.

Project-owned normal, informational, status, progress, success, and debug
diagnostics use the registered UID macro belonging to their source tree.
Warnings use that component's singular `WARNING` macro, errors use its `ERROR`
macro, and numeric assertion signals use `ASSERT` only for genuine invariant
failures. Direct standard-output,
standard-error, C printing, platform logging, and locally defined diagnostic
macros are prohibited for diagnostics.

Functional command output remains separate from tracing. Help, version text,
machine-readable results, protocol responses, interactive prompts, and command
results retain their existing output contract and must not receive trace
metadata that would change that interface.

## Runtime configuration

New normal traces start disabled. At startup, the shared thread-safe registry
loads the saved XML states. It resolves an effective state in this order:
individual tag, module, then global. Settings are applied from left to right; a
later global setting clears earlier module and tag overrides, and a later
module setting clears earlier tag overrides in that module. This makes the
last applicable setting win.

```bash
xwalk-picarx-control --trace RPI.001.enable
xwalk-picarx-control --trace RPI.001.disable
xwalk-picarx-control --trace CTRL.001.enable
xwalk-picarx-control --trace CTRL.001.disable
xwalk-picarx-control --trace RPI.enable
xwalk-picarx-control --trace RPI.disable
xwalk-picarx-control --trace CTRL.enable
xwalk-picarx-control --trace CTRL.disable
xwalk-picarx-control --trace RPIAGENT.enable
xwalk-picarx-control --trace LIB.disable
xwalk-picarx-control --trace all.enable
xwalk-picarx-control --trace all.disable
xwalk-picarx-control --trace xWalkController/xWalkConfig/xwalk-traces.json
```

JSON uses one top-level `trace` object. It applies `all`, then module states,
then tag states. State values must be the strings `enable` or `disable`;
Boolean values are rejected. Unknown modules and complete IDs, malformed or
missing files, and invalid selectors produce startup status 2 before hardware
composition. A successful selector or JSON update atomically replaces the XML
and updates memory. The next run loads that XML automatically, so the selector
does not need to be repeated. Example files are in `xWalkController/xWalkConfig`.

## Build validation and XML catalogue

`pre-compiler/xHal_Rpi5CarTracePreCompiler.py` tokenizes the four registered
`XWALK_<component>_TRACE_UIDn` macro families recursively across the repository,
including generated project sources and participating nested repositories.
Build trees and third-party/external trees are excluded. This macro inventory
is the sole source for validation, ownership enforcement, XML generation,
runtime registration, and trace discovery.

The normal CMake build depends on the scanner. It reports every duplicate ID
and every declaration path and line in one run, then returns non-zero before
the dependent compilation or link completes. For example:

```text
Trace validation error: non-unique trace IDs are used.

Duplicate trace ID: RPI.001
  Declared at: src/rpi/camera.cpp:42 (XWALK_HAL_TRACE_UID1)
  Declared at: src/rpi/motor.cpp:87 (XWALK_HAL_TRACE_UID2)

Compilation stopped because trace IDs must be unique.
```

A successful build atomically creates:

```text
<build-directory>/generated/xwalk-traces.xml
```

The UTF-8 catalogue is deterministic: modules are sorted by canonical name,
traces are sorted numerically and then by preserved ID text, paths are
project-relative, and no timestamps are stored. CMake depends on the scanner
and every recursively discovered project C/C++ source, so additions, removals,
IDs, text, module changes, and source-line changes regenerate it. Regeneration
preserves valid state flags for retained IDs, assigns `disable` to new IDs, and
removes absent IDs. Identical content is not rewritten.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<xwalkTraceCatalogue version="1.0" defaultState="disable">
  <module name="RPI" defaultState="disable">
    <trace
      id="001"
      fullId="RPI.001"
      defaultState="disable"
      name="I2C probe"
      sourceFile="xWalkHal/interface/xWalkI2c/core/src/xHal_Rpi5CarI2c.cpp"
      sourceLine="73"
      priority="2"
      formatArgumentCount="1"
      owningComponent="HAL" />
  </module>
</xwalkTraceCatalogue>
```

XML attributes are escaped by the generator and parsed with real XML parsers
in the automated tests. The runtime loads the catalogue once and writes it only
for a requested configuration update; it does not parse XML or JSON for every
trace call.

## Build and test

The build requires Python 3, TinyXML2, and the `libjson-c-dev` development
package, all resolved locally without network access.

```bash
cmake -S xWalkTrace -B xWalkTrace/build-host -DXWALK_TRACE_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-host --parallel
ctest --test-dir xWalkTrace/build-host --output-on-failure
```

Hardware tests are opt-in and must only be listed during ordinary development:

```bash
cmake -S xWalkTrace -B xWalkTrace/build-rpi -DXWALK_TRACE_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-rpi --parallel
ctest --test-dir xWalkTrace/build-rpi -N -L hardware
```
