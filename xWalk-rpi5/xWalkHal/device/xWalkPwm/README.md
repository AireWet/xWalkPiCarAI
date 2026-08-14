# xWalkPwm

C++17 Robot HAT PWM channel and shared-timer control.

The module receives an `XWalkI2c` object instead of opening Raspberry Pi
hardware directly. This permits host testing with callback-driven I2C data and
optional Raspberry Pi access through the prepared Linux `i2c-dev` backend.

The non-virtual `XWalkI2c` type is provided by the sibling `xWalkI2c` module.
An application or test entry point creates the backend, I2C interface, and
shared timer-state objects, then passes the I2C and timer-state objects by
reference to PWM. `XWalkPwm` stores non-owning pointers to both dependencies and
does not own or construct them.

Shared standard-library headers are provided by the sibling header-only
`xWalkLibraryCommon` interface target through `xHal_Rpi5CarCommon.h`.

A hardware composition root follows this pattern:

```cpp
xwalk::hal::XWalkI2cLinux backend;
xwalk::hal::XWalkI2c i2c(
    &backend,
    XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
    XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
    XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux),
    nullptr,
    XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux));
xwalk::hal::XWalkPwmTimerState timerState;
xwalk::hal::XWalkPwm pwm(i2c, 0U, {}, timerState);
```

Declaration order expresses lifetime: `pwm` is destroyed first, followed by
`timerState`, `i2c`, and finally `backend`.

Ordinary output methods keep their exception-based validation and failure
reporting. `trySetPulseWidthPercent()` is the separate non-throwing actuator
cleanup path: it validates without throwing, uses the I2C safe-write callback,
and returns whether the complete output update succeeded.

## Directory layout

```text
xWalkPwm/
├── include/
│   ├── xHal_Rpi5CarPwm.h
│   └── xHal_Rpi5CarPwmTimerState.h
├── src/
│   ├── xHal_Rpi5CarPwm.cpp
│   ├── xHal_Rpi5CarPwmLifecycle.cpp
│   ├── xHal_Rpi5CarPwmTimer.cpp
│   ├── xHal_Rpi5CarPwmTimerState.cpp
│   ├── xHal_Rpi5CarPwmTimerStateLifecycle.cpp
│   └── xHal_Rpi5CarPwmOutput.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarPwmTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── hardware/src/
│   │   └── xHal_Rpi5CarPwmHardwareTest.cpp
│   ├── include/
│   │   ├── xHal_Rpi5CarPwmTestFunctions.h
│   │   └── xHal_Rpi5CarPwmTestI2c.h
│   └── src/
│       ├── xHal_Rpi5CarPwmTestMain.cpp
│       ├── xHal_Rpi5CarPwmTestI2c.cpp
│       ├── xHal_Rpi5CarPwmTestI2cLifecycle.cpp
│       ├── xHal_Rpi5CarPwmTestAddress.cpp
│       ├── xHal_Rpi5CarPwmTestOutput.cpp
│       ├── xHal_Rpi5CarPwmTestTimer.cpp
│       ├── xHal_Rpi5CarPwmTestValidation.cpp
│       └── xHal_Rpi5CarPwmTestTrace.cpp
├── CMakeLists.txt
└── README.md
```

Source responsibilities:

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarPwm.cpp` | Channel parsing, timer mapping, and I2C address selection |
| `xHal_Rpi5CarPwmLifecycle.cpp` | PWM constructors, destructor, and initial 50 Hz setup |
| `xHal_Rpi5CarPwmTimer.cpp` | Frequency search, prescaler, period, and shared timer state |
| `xHal_Rpi5CarPwmTimerState.cpp` | Thread-safe access to private shared timer periods |
| `xHal_Rpi5CarPwmTimerStateLifecycle.cpp` | Timer-state constructor and destructor |
| `xHal_Rpi5CarPwmOutput.cpp` | Pulse width, percentage conversion, and 16-bit register output |

## Build and test on the host

Host mode is a logic simulation. It uses the fake callback-based I2C object and
does not open `/dev/i2c-1`, build `XWalkI2cLinux`, or require an RPi or Robot
HAT. Use a separate directory so CMake cannot reuse hardware-mode cache values.
Run the following commands from the repository root.

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkPwm -B xWalk-rpi5/xWalkHal/device/xWalkPwm/build-host -DXWALK_PWM_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkPwm/build-host --parallel
ctest --test-dir xWalkPwm/build-host --output-on-failure
```

| CMake flag | Default | Purpose |
|---|---:|---|
| `XWALK_PWM_BUILD_HOST_TESTS` | `OFF` | Build the seven hardware-independent PWM tests |
| `XWALK_PWM_BUILD_HARDWARE_TESTS` | `OFF` | Build the Linux I2C backend and RPi PWM test |
| `XWALK_I2C_BUILD_HOST_TESTS` | Inherited | Follows the PWM host-test option |
| `XWALK_I2C_BUILD_HARDWARE_TESTS` | Inherited | Follows the PWM hardware-test option |

The hardware flags are safety gates. Enabling the PWM hardware flag builds the
prepared Linux I2C backend, its address-probe test, and a PWM zero-output test.
The configuration is rejected on non-Linux systems.

When `xWalkPwm` is configured as the top-level module, specify only its test
flags. CMake passes the matching values to `xWalkI2c` automatically. When
`xWalkI2c` is configured directly, its own flags remain independent and default
to `OFF`.

## Build hardware tests without running them

The hardware targets can be compile-checked on a Linux host before the RPi and
Robot HAT arrive:

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkPwm -B xWalk-rpi5/xWalkHal/device/xWalkPwm/build-rpi -DXWALK_PWM_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkPwm/build-rpi --parallel
ctest --test-dir xWalkPwm/build-rpi -N -L hardware
```

The PWM flag automatically enables the matching I2C hardware flag. The listed
hardware tests are:

| CTest name | Hardware action |
|---|---|
| `xWalkI2cLinuxHardwareProbeTest` | Open `/dev/i2c-1` and probe `0x14`, `0x15`, and `0x16` |
| `xWalkPwmHardwareZeroOutputTest` | Initialize PWM channel 0 and write a zero pulse width |

Do not run these tests without the Robot HAT. On the RPi with the vehicle safely
raised or motors disconnected, run them with:

```bash
ctest --test-dir xWalkPwm/build-rpi -L hardware --output-on-failure
```

The backend and tests compile successfully on Linux but still require physical
hardware validation.

List the separately registered PWM and I2C tests without running them:

```bash
ctest --test-dir xWalkPwm/build-host -N
```

CTest reports these entries:

| CTest name | PWM functionality |
|---|---|
| `xWalkPwmAddressTest` | Address probing, channel parsing, and timer selection |
| `xWalkPwmTimerMappingTest` | Mapping of all 20 channels to seven timers |
| `xWalkPwmRegisterDataTest` | Big-endian 16-bit register data |
| `xWalkPwmPercentageTest` | Shared period and pulse-width percentage conversion |
| `xWalkPwmFrequencyTest` | Default 50 Hz timer configuration |
| `xWalkPwmValidationTest` | Invalid channel, frequency, and output values |
| `xWalkPwmTraceSelectionTest` | Persistent trace-selector parsing and application |
| `xWalkI2cHostTest` | Callback-based host I2C behavior |

Running `./xWalkPwm/build-host/xWalkPwmTest` without a selector still runs all PWM scenarios
in one process.

## Run a specific test case

Use the exact CTest name with `-R` to run one registered test. Anchoring the
regular expression with `^` and `$` prevents similarly named tests from being
selected accidentally.

For example, run only the PWM frequency test:

```bash
ctest --test-dir xWalkPwm/build-host -R '^xWalkPwmFrequencyTest$' --output-on-failure
```

Run only the I2C host test:

```bash
ctest --test-dir xWalkPwm/build-host -R '^xWalkI2cHostTest$' --output-on-failure
```

The PWM executable also accepts a short selector when invoked directly:

```bash
./xWalkPwm/build-host/xWalkPwmTest frequency
```

| CTest name | Direct executable command |
|---|---|
| `xWalkPwmAddressTest` | `./xWalkPwm/build-host/xWalkPwmTest address` |
| `xWalkPwmTimerMappingTest` | `./xWalkPwm/build-host/xWalkPwmTest mapping` |
| `xWalkPwmRegisterDataTest` | `./xWalkPwm/build-host/xWalkPwmTest register` |
| `xWalkPwmPercentageTest` | `./xWalkPwm/build-host/xWalkPwmTest percentage` |
| `xWalkPwmFrequencyTest` | `./xWalkPwm/build-host/xWalkPwmTest frequency` |
| `xWalkPwmValidationTest` | `./xWalkPwm/build-host/xWalkPwmTest validation` |
| `xWalkPwmTraceSelectionTest` | `./xWalkPwm/build-host/xWalkPwmTest trace` |

## Standalone host simulation and tracing

The standalone simulation composes the public PWM and I2C interfaces with an
in-memory backend. It never opens `/dev/i2c-1` or accesses a Robot HAT.

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkPwm/simulation -B build-pwm-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-pwm-simulation --parallel
./build-pwm-simulation/xWalkPwmSimulation --trace RPI.enable
```

Trace selectors accept `RPI.<digits>.enable`, `RPI.enable`, `all.enable`, their
`.disable` counterparts, or a trace-update JSON path. Successful changes update
the generated XML and load automatically on the next run. Enabled traces appear
in the terminal and `build-pwm-simulation/log/xWalkPwmSimulation.log`.

CTest is preferred for normal use because it reports the selected test name,
execution status, and failure output consistently.

## Clean the xWalkPwm build

Run this command from the repository root to clean simulated host-test outputs
while retaining the configured CMake cache:

```bash
cmake --build xWalkPwm/build-host --target clean
```

For a completely clean simulation configuration, remove its generated build
directory:

```bash
cmake -E remove_directory xWalkPwm/build-host
```

Remove the separate hardware build directory with:

```bash
cmake -E remove_directory xWalkPwm/build-rpi
```

These commands do not remove the `xWalkPwm`, `xWalkI2c`, or `xWalkLibraryCommon`
source directories.

Perform a clean rebuild and rerun all host tests with:

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkPwm -B xWalk-rpi5/xWalkHal/device/xWalkPwm/build-host -DXWALK_PWM_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkPwm/build-host --parallel
ctest --test-dir xWalkPwm/build-host --output-on-failure
```

## Ported behavior

| PWM contract | C++ implementation |
|---|---|
| Channels `0..19` or `P0..P19` | Integer and string constructors |
| Addresses `0x14`, `0x15`, `0x16` | Probed in the same order |
| Seven shared PWM timer periods | `XWalkPwmTimerState` |
| Register groups `0x40/0x44` and `0x50/0x54` | Preserved |
| 72 MHz timer calculation | Preserved |
| Default 50 Hz initialization | Preserved |
| High-byte then low-byte register data | Explicit in `write16()` |

Invalid seven-bit I2C addresses, frequencies, percentages, and register values
are rejected with C++ exceptions. Non-finite floating-point arguments report
`std::invalid_argument`; finite values outside a supported numeric range report
`std::out_of_range`.

During the first physical test, confirm the 16-bit byte order with an I2C logic analyzer capture against the
documented register protocol.
