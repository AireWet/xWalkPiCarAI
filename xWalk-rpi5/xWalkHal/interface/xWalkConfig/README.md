# xWalkConfig

C++17 embedded-oriented configuration module for section-aware and flat key-value files.

The module retains two focused classes:

- `XWalkConfig` provides section-aware parsing, in-memory editing, explicit
  reload, and persistence while preserving unrelated file content.
- `XWalkConfigStore` provides lightweight string-key persistence used for
  calibration values and Robot servo offsets.

Each class remains in its own header and implementation files while both are
provided by the single `xWalkConfig` static library.

## Directory layout

```text
xWalkConfig/
├── include/
│   ├── xHal_Rpi5CarConfig.h
│   ├── xHal_Rpi5CarConfigStore.h
│   └── xHal_Rpi5CarConfigTypes.h
├── src/
│   ├── xHal_Rpi5CarConfig.cpp
│   ├── xHal_Rpi5CarConfigLifecycle.cpp
│   ├── xHal_Rpi5CarConfigStore.cpp
│   └── xHal_Rpi5CarConfigStoreLifecycle.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarConfigTraceConfig.py
│   ├── include/
│   └── src/
├── test/include/xHal_Rpi5CarConfigTestSupport.h
├── test/src/
│   ├── xHal_Rpi5CarConfigTest.cpp
│   ├── xHal_Rpi5CarConfigTestSupport.cpp
│   └── xHal_Rpi5CarConfigStoreTest.cpp
├── CMakeLists.txt
└── README.md
```

## Composition and ownership

Create configuration objects in `main()` or the application composition root.
Hardware drivers receive parsed values or a required configuration-store
reference and do not create configuration objects internally.

```cpp
XWalkConfig configuration("config/robot.ini", "Robot settings");
configuration.set("motor", "speed", "75");
configuration.write();

XWalkConfigStore offsets("config/servo-offsets.config");
offsets.set("legs", "0, 0, 0, 0");
```

Each object owns its configured filesystem path and serializes operations made
through that object. Separate instances addressing the same file, or external
writers, require application-level synchronization.

## Section-aware configuration behavior

- Blank lines and lines beginning with `#` are not parsed as options.
- Section headers use `[section]`; assignments split at the first `=`.
- Names and values are trimmed when read.
- Missing options are inserted into memory using the supplied default.
- Comments, blank lines, unrelated options, and unrelated text are retained.
- New files may receive a multiline description rendered as `# ` comments.
- Updates use a same-directory replacement file and preserve permission bits.

## String-key configuration-store behavior

- A missing parent directory and configuration file are created.
- Missing keys return the supplied default.
- ASCII spaces are removed from retrieved values to preserve the established file contract.
- The last duplicate key wins during retrieval.
- `include = relative/path.conf` recursively inserts a `.conf` file at that
  position; absolute paths, parent traversal, cycles, and depth above eight are rejected.
- Updates replace every matching duplicate entry or append an absent key.
- Updates affect only the primary file, allowing it to override read-only included defaults.
- Comments and malformed unrelated lines remain unchanged.

The module does not accept ownership and permission arguments or invoke shell commands. Apply
deployment ownership and permission policy through trusted platform provisioning.

## Host build and tests

```bash
cmake -S xWalk-rpi5/xWalkHal/interface/xWalkConfig -B xWalk-rpi5/xWalkHal/interface/xWalkConfig/build-host -DXWALK_CONFIG_BUILD_HOST_TESTS=ON
cmake --build xWalkConfig/build-host --parallel
ctest --test-dir xWalkConfig/build-host --output-on-failure
```

Both tests write only beneath `xWalkConfig/build-host/test-data`. They do not
access physical hardware or deployed configuration paths.

Test diagnostics and enabled operations use trace macros and appear in the
terminal and the target-specific log. Successful selector changes persist in
the generated XML and load automatically on later runs.

## Standalone simulation

The standalone executable writes only below its generated build directory. It
persists and reconstructs one section-aware file and one flat configuration
store without accessing deployed configuration paths.

```bash
cmake -S xWalk-rpi5/xWalkHal/interface/xWalkConfig/simulation -B xWalk-rpi5/xWalkHal/interface/xWalkConfig/simulation/build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkHal/interface/xWalkConfig/simulation/build-host --parallel
xWalk-rpi5/xWalkHal/interface/xWalkConfig/simulation/build-host/xWalkConfigSimulation --trace RPI.enable
```

Selectors accept `RPI.<digits>`, the complete `RPI` tag, `all`, or a JSON update.
The `.enable` and `.disable` operations are stored in XML for the next run. See
[`simulation/README.md`](simulation/README.md) for the complete safe-run contract.

## Target compilation

```bash
cmake -S xWalk-rpi5/xWalkHal/interface/xWalkConfig -B xWalk-rpi5/xWalkHal/interface/xWalkConfig/build-rpi -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkConfig/build-rpi --parallel
```

The module has no physical-hardware backend or hardware test. Target verification
compiles the production filesystem library without executing filesystem operations.
