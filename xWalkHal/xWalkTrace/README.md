# xWalkTrace

C++17 embedded-oriented filtered diagnostic interface.

The module provides critical, error, warning, info, and debug levels
through a typed interface. It validates numeric and lowercase text selections,
filters records with warning as the default threshold, and synchronously passes
accepted records to an application-provided callback.

## Directory layout

```text
xWalkTrace/
├── include/
│   ├── xHal_Rpi5CarTrace.h
│   └── xHal_Rpi5CarTraceTypes.h
├── src/
│   ├── xHal_Rpi5CarTrace.cpp
│   └── xHal_Rpi5CarTraceLifecycle.cpp
├── test/
│   ├── hardware/src/xHal_Rpi5CarTraceHardwareTest.cpp
│   └── src/xHal_Rpi5CarTraceTest.cpp
├── CMakeLists.txt
└── README.md
```

## Composition and ownership

Create the output backend or state in `main()`, then pass a non-owning context
and callback to `XWalkTrace`. A null context is supported for a stateless
callback, but the callback itself must not be null.

```cpp
void traceOutput(contextpointer context, XWalkTraceLevel level, stringview message);

TraceOutputState outputState;
XWalkTrace trace(&outputState, &traceOutput, XWalkTraceLevel::Warning);
trace.info("Initialization completed");
trace.error("Peripheral initialization failed");
```

The callback runs synchronously and may consume the message only during that
invocation. Any non-null context must outlive the trace object. Calls and level
changes must be externally serialized if several execution contexts share one
trace object.

## Ported behavior

- Levels zero through four use the order critical, error, warning,
  info, and debug.
- The default threshold is warning.
- Numeric levels from zero through four and exact lowercase names are accepted.
- A level change produces a debug record only when the new threshold accepts
  debug output.
- Records with a numeric severity no greater than the configured threshold are
  delivered synchronously.

The module deliberately leaves
backend selection, timestamps, and final formatting to the injected callback.
It introduces no global logger, hidden console dependency, worker, or dynamic
owning pointer.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkTrace -B xWalkTrace/build-host -DXWALK_TRACE_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-host --parallel
ctest --test-dir xWalkTrace/build-host --output-on-failure
```

The host test uses bounded in-memory callback state and performs no hardware or
filesystem access.

## Target compile and test discovery

```bash
cmake -S xWalkTrace -B xWalkTrace/build-rpi -DXWALK_TRACE_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkTrace/build-rpi --parallel
ctest --test-dir xWalkTrace/build-rpi -N -L hardware
```

The hardware-labelled executable only compile-checks target composition. It
does not access a peripheral and is listed, not executed, during verification.
