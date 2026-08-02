# xWalkI2c

Hardware-independent I2C interface used by xWalk HAL components.

```text
xWalkI2c/
├── core/
│   ├── include/
│   │   └── xHal_Rpi5CarI2c.h
│   └── src/
│       ├── xHal_Rpi5CarI2c.cpp
│       └── xHal_Rpi5CarI2cLifecycle.cpp
├── test/
│   └── xHal_Rpi5CarI2cTest.cpp
├── hardware/
│   ├── include/
│   │   └── xHal_Rpi5CarI2cLinux.h
│   ├── src/
│   │   ├── xHal_Rpi5CarI2cLinux.cpp
│   │   └── xHal_Rpi5CarI2cLinuxLifecycle.cpp
│   └── test/src/
│       └── xHal_Rpi5CarI2cLinuxHardwareTest.cpp
├── CMakeLists.txt
└── README.md
```

`XWalkI2c` is intentionally independent of Raspberry Pi hardware. It is a
concrete non-virtual object containing C-style callback pointers and a context
pointer. Host tests provide recording callbacks. The optional
`XWalkI2cLinux` backend supplies callback-compatible operations implemented with
Linux `i2c-dev`.

Composition is explicit at the application or test entry point: construct the
backend first, construct a separate `XWalkI2c` with a non-owning pointer to that
backend, then pass the I2C object by reference to consumers. The Linux backend
does not contain or own an `XWalkI2c` object. It also supplies the optional
non-throwing safe-write callback used by actuator cleanup. `tryWriteRegister()`
returns a Boolean status for invalid input, a missing safe callback, or backend
write failure; it does not intercept an exception.

## Build xWalkI2c separately

The module can be configured and built directly without building `xWalkPwm`
and without Raspberry Pi hardware. Its CMake configuration automatically adds
the adjacent `xWalkCommon` dependency.

Run these commands from the repository root:

The host test is a logic simulation. It uses in-memory callbacks and never
opens `/dev/i2c-1`, builds `XWalkI2cLinux`, or requires an RPi.

```bash
cmake -S xWalkI2c -B xWalkI2c/build-host -DXWALK_I2C_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkI2c/build-host --parallel
```

| CMake flag | Default | Purpose |
|---|---:|---|
| `XWALK_I2C_BUILD_HOST_TESTS` | `OFF` | Build callback-based tests that require no hardware |
| `XWALK_I2C_BUILD_HARDWARE_TESTS` | `OFF` | Build the Linux backend and RPi integration test |

The resulting library is:

| Library | Purpose |
|---|---|
| `xWalkI2c/build-host/libxWalkI2c.a` | Hardware-independent I2C callback object |

`xWalkCommon` is a header-only interface target, so it propagates shared types,
math operations, and exception helpers without producing a separate archive.

To build the library without its host test:

```bash
cmake -S xWalkI2c -B xWalkI2c/build-lib -DCMAKE_BUILD_TYPE=Release
cmake --build xWalkI2c/build-lib --parallel
```

## Test xWalkI2c separately

After configuring with `XWALK_I2C_BUILD_HOST_TESTS=ON`, run the test through CTest:

```bash
ctest --test-dir xWalkI2c/build-host --output-on-failure
```

The test executable can also be run directly:

```bash
./xWalkI2c/build-host/xWalkI2cTest
```

The host test verifies callback construction, seven-bit address validation,
address probing, register writes, sequential reads, and register-addressed
reads. It uses in-memory callbacks and does not access `/dev/i2c-*` or require
an RPi.

| Expected result | Value |
|---|---|
| CTest name | `xWalkI2cHostTest` |
| Required hardware | None |
| Required sibling module | `xWalkCommon` |
| Expected passing tests | `1` |

## Test on Raspberry Pi hardware

### Current hardware status

`XWalkI2c` remains the hardware-independent callback object. The prepared
`XWalkI2cLinux` backend owns the `/dev/i2c-1` file descriptor and supplies
Linux callback-compatible operations. The hardware-test entry point creates a
separate `XWalkI2c` object and binds its non-owning context to the backend. The
backend is compiled only when
`XWALK_I2C_BUILD_HARDWARE_TESTS=ON`.

The backend has been compiled on a Linux host but has not yet been validated
against physical Robot HAT hardware.

| Callback | Linux backend responsibility |
|---|---|
| `i2cprobecallback` | Select an address and report whether the device responds |
| `i2cwriteregistercallback` | Write a register followed by the supplied data bytes |
| `i2creadcallback` | Read a requested number of bytes from the selected device |
| `i2creadregistercallback` | Atomically select a register and read consecutive bytes |
| `i2ctrywriteregistercallback` | Attempt a validated register write and return status without throwing |

The backend uses Linux `i2c-dev` access without virtual functions and keeps the
Linux file descriptor outside `XWalkI2c`.

### Hardware information

| Item | Expected value |
|---|---|
| Linux I2C device | `/dev/i2c-1` |
| Bus number | `1` |
| Robot HAT addresses | `0x14`, `0x15`, `0x16` |
| Two-data-byte operation | SMBus word write |
| Larger operation | SMBus I2C block write |
| PWM value byte order | High byte followed by low byte |
| Retry count | `5` |

No additional SunFounder repository is required for the C++ Linux backend.
The hardware-specific requirements are the Robot HAT register protocol, its
firmware, and Linux I2C device access.

### Prepare and inspect the RPi

Enable I2C using the configuration method provided by the installed Raspberry
Pi OS, reboot if required, and confirm that bus 1 exists:

```bash
ls -l /dev/i2c-1
```

Scan bus 1 before running a write test:

```bash
i2cdetect -y 1
```

At least one of `14`, `15`, or `16` should appear in the scan. If none appears,
check power, cabling, I2C enablement, permissions, and Robot HAT firmware before
running any PWM test.

### Recommended hardware-test order

Keep the vehicle raised so its wheels cannot move unexpectedly. Begin with
read-only or zero-output operations.

| Order | Hardware test | Expected behavior |
|---:|---|---|
| 1 | Device-file test | `/dev/i2c-1` can be opened |
| 2 | Address-probe test | One expected Robot HAT address responds |
| 3 | Zero-output write | A selected PWM channel is written with value zero |
| 4 | Fixed PWM output | A known frequency and duty cycle are generated |
| 5 | Scope or logic-analyzer check | Frequency, duty cycle, and byte order match the request |
| 6 | Channel mapping | Physical outputs P0 through P19 use the expected timer groups |
| 7 | Repeated-write test | No intermittent I2C errors occur during extended operation |

### Build the prepared Linux backend

Hardware code and tests are disabled by default so normal host builds remain
portable and safe. The CMake interface provides this flag:

```cmake
option(XWALK_I2C_BUILD_HARDWARE_TESTS "Build RPi I2C hardware tests" OFF)
```

The backend can be compile-checked on a Linux host without running it:

```bash
cmake -S xWalkI2c -B xWalkI2c/build-rpi -DXWALK_I2C_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkI2c/build-rpi --parallel
```

This creates `libxWalkI2cLinux.a` and the
`xWalkI2cLinuxHardwareTest` executable. Do not run the executable on a machine
without `/dev/i2c-1` and a connected Robot HAT.

Set `XWALK_I2C_HARDWARE_DEVICE` during CMake configuration when the prepared
target uses another bus path. The selected path is passed to the hardware test;
the test does not scan or guess an I2C bus.

List the hardware-labelled test without running it:

```bash
ctest --test-dir xWalkI2c/build-rpi -N -L hardware
```

On the RPi with hardware connected, run only the hardware-labelled test:

```bash
ctest --test-dir xWalkI2c/build-rpi -L hardware --output-on-failure
```

The registered I2C hardware test is `xWalkI2cLinuxHardwareProbeTest`. It opens
`/dev/i2c-1` and succeeds only when one of `0x14`, `0x15`, or `0x16` responds.
The hardware configuration is rejected on non-Linux systems.

## Clean the xWalkI2c build

Run the following command from the repository root to remove simulated host-test
outputs while retaining the CMake cache:

```bash
cmake --build xWalkI2c/build-host --target clean
```

To remove the simulation configuration and every generated file, delete its
complete build directory through CMake:

```bash
cmake -E remove_directory xWalkI2c/build-host
```

Use separate commands for the hardware and library-only build directories:

```bash
cmake -E remove_directory xWalkI2c/build-rpi
cmake -E remove_directory xWalkI2c/build-lib
```

These commands remove only generated build directories. They do not remove
`core`, `hardware`, `test`, `CMakeLists.txt`, or the workspace module `../../xWalkCommon`
source directory.

Perform a clean rebuild and rerun the host test with:

```bash
cmake -S xWalkI2c -B xWalkI2c/build-host -DXWALK_I2C_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkI2c/build-host --parallel
ctest --test-dir xWalkI2c/build-host --output-on-failure
```
