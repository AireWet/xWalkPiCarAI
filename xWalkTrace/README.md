# xWalkTrace

`xWalkTrace` is the C++17 trace service shared by HAL, Controller, nested
modules, and libraries. Tagged traces are build-validated, filtered before
format arguments are evaluated, and written synchronously to the terminal and
`log/xWalkTrace.log` with UTC time, monotonic elapsed time, UID, and source
location. Both destinations receive the same completely formatted record.

## Trace declarations

HAL traces use `RPI.<numeric-id>` and Controller traces use
`CTRL.<numeric-id>`:

```cpp
XWALK_HAL_TRACE_UID2(RPI.001, "I2C probe: %u", address);
XWALK_CTRL_TRACE_UID1(CTRL.001, "Controller command execution started");
```

The complete ID must be unique across the repository. `RPI.001` and
`CTRL.001` are distinct and valid; declaring `RPI.001` twice stops the build.
Leading zeros remain significant. Priority suffixes `0` through `3` describe
record priority but do not bypass runtime state.

Warnings, errors, and numeric assertions remain intentionally unconditional:

```cpp
XWALK_HAL_WARNINGS("HAL warning: %d", warningCode);
XWALK_HAL_ERROR("HAL error: %d", errorCode);
XWALK_HAL_ASSERT(100);
XWALK_CTRL_ERROR("Controller error: %d", errorCode);
```

The legacy `XWALK_VERBOSE` name remains as a disabled no-op for source
compatibility. Production normal diagnostics must use a registered UID macro.

## Runtime configuration

All normal traces start disabled. The shared thread-safe registry resolves an
effective state in this order: individual tag, module, then global. Settings
are applied from left to right; a later global setting clears earlier module
and tag overrides, and a later module setting clears earlier tag overrides in
that module. This makes the last applicable setting win.

```bash
xwalk-picarx-control --trace RPI.001.enable
xwalk-picarx-control --trace RPI.001.disable
xwalk-picarx-control --trace CTRL.001.enable
xwalk-picarx-control --trace CTRL.001.disable
xwalk-picarx-control --trace RPI.enable
xwalk-picarx-control --trace RPI.disable
xwalk-picarx-control --trace CTRL.enable
xwalk-picarx-control --trace CTRL.disable
xwalk-picarx-control --trace all.enable
xwalk-picarx-control --trace all.disable
xwalk-picarx-control --trace xWalkController/xWalkConfig/xwalk-traces.json
```

JSON uses one top-level `trace` object. It applies `all`, then module states,
then tag states. State values must be the strings `enable` or `disable`;
Boolean values are rejected. Unknown modules and complete IDs, malformed or
missing files, and invalid selectors produce startup status 2 before hardware
composition. Example files are in `xWalkController/xWalkConfig`.

## Build validation and XML catalogue

`pre-compiler/xHal_Rpi5CarTracePreCompiler.py` tokenizes the structured
`XWALK_HAL_TRACE_UIDn` and `XWALK_CTRL_TRACE_UIDn` declarations recursively
across the repository, including generated project sources and participating
nested repositories. Build trees and third-party/external trees are excluded.
This macro inventory is the sole source for validation, XML generation, runtime
registration, and trace discovery.

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
IDs, text, module changes, and source-line changes regenerate it. Identical
content is not rewritten.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<xwalkTraceCatalogue version="1.0">
  <module name="RPI" defaultState="disable">
    <trace
      id="001"
      fullId="RPI.001"
      defaultState="disable"
      name="I2C probe"
      sourceFile="xWalkHal/xWalkI2c/core/src/xHal_Rpi5CarI2c.cpp"
      sourceLine="73"
      priority="2"
      owningComponent="HAL" />
  </module>
</xwalkTraceCatalogue>
```

XML attributes are escaped by the generator and parsed with real XML parsers
in the automated tests. The runtime loads the catalogue once; it does not parse
XML or JSON for every trace call.

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
