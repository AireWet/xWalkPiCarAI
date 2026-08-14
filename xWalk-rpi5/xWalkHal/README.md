# xWalk HAL aggregate build and test guide

The workspace root CMake project builds the HAL libraries and coordinates the
tests registered by every submodule and the sibling `xWalkIW` interface. The
`xWalkHal` directory has no aggregate CMake file; run the commands in this
document from the repository root.

All aggregate build flags default to `OFF`. Host and Raspberry Pi verification
must use separate build directories because they enable different backends and
tests.

## Architectural groups

The HAL modules are organized by responsibility while retaining their existing
library targets, namespaces, APIs, tests, and trace identifiers:

```text
xWalk-rpi5/xWalkHal/
├── interface/  low-level platform interfaces and common services
├── device/     hardware device abstractions
├── sensor/     sensor and actuator components
└── layer1/     higher-level robot services and features
```

Dependencies flow from `interface` through `device` and `sensor` to `layer1`.
A higher group may use a lower group when its existing contract requires it.
The repository-wide `xWalk-rpi5/xWalkLibrary/common` interface remains outside `xWalkHal`
because HAL, Agent, Controller, IW, and Trace consumers share it.

Each architectural group documents its hardware-independent GoogleTest
interaction suite separately:

- [`interface`](interface/README.md)
- [`device`](device/README.md)
- [`sensor`](sensor/README.md)
- [`layer1`](layer1/README.md)

These suites complement, and do not replace, each module's individual host
tests.

## Aggregate build modes

| Root flag | Default | Result |
|---|---:|---|
| `BUILD_TESTING` | `ON` | Builds deterministic host tests when `XWALK_BUILD_RPI=OFF` |
| `XWALK_BUILD_RPI` | `OFF` | Builds Raspberry Pi backends and hardware-labelled tests |
| `XWALK_ENABLE_PACKAGING` | `OFF` | Enables deployment packaging for an RPI build |

The root configuration derives its internal HAL host/RPI mode and propagates
it to every submodule. Individual options such as
`XWALK_PWM_BUILD_HOST_TESTS` and `XWALK_I2C_BUILD_HOST_TESTS` are not needed for
a workspace build.

## Required tools and source layout

| Requirement | Purpose |
|---|---|
| CMake 3.16 or newer | Configures the aggregate project |
| C++17 compiler | Builds all native libraries and tests |
| ALSA development library | Builds the optional shared PCM and mixer backend |
| Protobuf and gRPC development libraries | Build the xWalkIW interface library |
| GoogleTest development library | Provides the central HAL host-test framework |
| json-c development library | Parses runtime trace configuration files |
| TinyXML2 development library | Validates the central test selection file |
| yaml-cpp development library | Loads board, AI, example, and hardware runtime values |
| `../xWalkLibrary` | Architecture-selected portable dependencies and shared Vosk model |
| `../xWalkIW` | Protobuf and gRPC interface module imported by the aggregate |
| `../xWalkController` | Standalone CLI aggregate included by host and RPI aggregate builds |
| `../../xWalkTool/dtoverlays` | Robot HAT and Servo HAT+ Raspberry Pi boot overlays |
| Linux GPIO, I2C, and SPI UAPI headers | Required when `XWALK_BUILD_RPI=ON` |

On Debian or Ubuntu, install the normal build dependencies with:

```bash
sudo apt-get install build-essential cmake libasound2-dev libcurl4-openssl-dev libgrpc++-dev libprotobuf-dev libgtest-dev libjson-c-dev libtinyxml2-dev libyaml-cpp-dev libsndfile1-dev linux-libc-dev
```

## Build and run every host test

Host mode is deterministic logic simulation. It must not open GPIO, I2C, SPI,
or Audio devices, and it does not require a Raspberry Pi or Robot HAT.

From the repository root, configure the complete host build:

```bash
cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
```

Build every library and host test executable:

```bash
cmake --build build --parallel
```

List every registered host test without running it:

```bash
ctest --test-dir build -N -L host
```

Run every host test from every submodule:

```bash
./build/xGoogleTest
ctest --test-dir build -L host --output-on-failure --parallel 4
```

Plain `ctest --test-dir build --output-on-failure` is equivalent in a
host-only build, but the `host` label makes the intended test class explicit.

The HAL scenarios are registered inside the single `xGoogleTest` CTest entry.
Their sources remain in each owning module. See
[`xWalkTest/xGoogleTest/README.md`](xWalkTest/xGoogleTest/README.md) for the separate host
and hardware XML profiles, the source inventory, and runtime selection.

## Run tests from one submodule

Use the central runtime suite name to select one HAL module. For example:

```bash
./build/xGoogleTest TEST_SUITE_XWALK_PWM:1
./build/xGoogleTest TEST_SUITE_XWALK_PWM:Address:1
```

| Submodule | Central suite | Test scope |
|---|---|---|
| xWalkAdc | `TEST_SUITE_XWALK_ADC` | ADC conversion, in-memory I2C simulation, and trace persistence |
| xWalkAdxl345 | `TEST_SUITE_XWALK_ADXL345` | Accelerometer conversion, safe simulation, and trace persistence |
| xWalkAudio | `TEST_SUITE_XWALK_AUDIO` | Injected ALSA ownership, host simulation, and recovery behavior |
| xWalkBoardControl | `TEST_SUITE_XWALK_BOARD_CONTROL` | Board services, safe simulation, and trace persistence |
| xWalkBuzzer | `TEST_SUITE_XWALK_BUZZER` | Active/passive behavior, safe simulation, and trace persistence |
| xWalkCamera | `TEST_SUITE_XWALK_CAMERA` | Capture validation, safe simulation, and trace persistence |
| xWalkConfig | `TEST_SUITE_XWALK_CONFIG` | Section/store persistence, trace selectors, and safe simulation |
| xWalkGpio | `TEST_SUITE_XWALK_GPIO` | Linux-backend GPIO simulation, pin mapping, polarity, and interrupts |
| xWalkGPT | `TEST_SUITE_XWALK_GPT` | Speech coordination, safe simulation, and trace persistence |
| xWalkI2c | `TEST_SUITE_XWALK_I2C` | Callback-based I2C behavior |
| xWalkLanguageModel | `TEST_SUITE_XWALK_LANGUAGE_MODEL` | Coordinator, in-memory simulation, trace selectors, and fake-HTTP provider behavior |
| xWalkLed | `TEST_SUITE_XWALK_LED` | Single/RGB LED behavior, safe simulation, and trace persistence |
| xWalkLineTracker | `TEST_SUITE_XWALK_LINE_TRACKER` | Tracking, in-memory simulation, and trace persistence |
| xWalkMotor | `TEST_SUITE_XWALK_MOTOR` | Motor control, safe simulation, and trace persistence |
| xWalkMusic | `TEST_SUITE_XWALK_MUSIC` | Music behavior, silent simulation, trace persistence, and ALSA adapter |
| xWalkPwm | `TEST_SUITE_XWALK_PWM` | Addressing, timers, output, safe simulation, and trace persistence |
| xWalkRobot | `TEST_SUITE_XWALK_ROBOT` | Multi-servo coordination, safe simulation, and trace persistence |
| xWalkServo | `TEST_SUITE_XWALK_SERVO` | Angle, pulse output, in-memory simulation, and trace persistence |
| xWalkSpeaker | `TEST_SUITE_XWALK_SPEAKER` | Speaker tasks, silent simulation, trace persistence, and ALSA adaptation |
| xWalkSpi | `TEST_SUITE_XWALK_SPI` | Linux-backend SPI simulation and bounded transfer behavior |
| xWalkTrace | `TEST_SUITE_XWALK_TRACE` | Trace behavior |
| xWalkUltrasonic | `TEST_SUITE_XWALK_ULTRASONIC` | Distance, in-memory GPIO simulation, and trace persistence |
| xWalkUserButton | `TEST_SUITE_XWALK_USER_BUTTON` | Button events, safe simulation, and trace persistence |
| xWalkUtils | `TEST_SUITE_XWALK_UTILS` | Injected utilities, safe Linux behavior, and host simulation |
| xWalkVoiceAssistant | `TEST_SUITE_XWALK_VOICE_ASSISTANT` | Traced coordinator, persistent simulation, and completed-backend composition |
| xSequenceTest | `TEST_SUITE_XWALK_SEQUENCE` | Button, servo, ADC, motor, speech, and tone flows |
| xExample | Direct `xExample <selector> <arguments>` invocation | Ported hardware and service examples |

Upstream examples are ported one by one under `xWalkTest/xExample`. That module has
separate reusable and Raspberry Pi layers and one central `main.cpp`. It is an
example launcher rather than a test suite, so it has no XML profile or CTest
registration. See [`xWalkTest/xExample/README.md`](xWalkTest/xExample/README.md) for each
selector's YAML configuration and formal argument compatibility form.

The project-managed Vosk assets are documented in
[`../xWalkLibrary/README.md`](../xWalkLibrary/README.md). The xExample CMake configuration writes
their absolute paths into its build-local YAML, so Vosk selectors do not depend
on `/usr/share`, the dynamic-linker search path, or the current working
directory. CMake selects the separate Linux ARM64 or x86-64 native library for
the target. Neither retained library supports 32-bit Raspberry Pi OS.

The xWalkIW schema validator and xWalkController tests are separate non-HAL
CTest entries and continue to run through the complete root `ctest` command.

The workspace-level `../xWalkLibrary/common` module is a header-only interface library
shared by HAL, agent, and other application layers. It exports the public
headers stored under `../xWalkLibrary/common` and does not register a separate
executable test.

## Run one specific test case

First obtain the exact GoogleTest suite and case names:

```bash
./build/xGoogleTest --gtest_list_tests
```

Run one exact case using the custom selector:

```bash
./build/xGoogleTest TEST_SUITE_XWALK_PWM:Frequency:1
```

Or use a standard GoogleTest filter:

```bash
./build/xGoogleTest --gtest_filter=TEST_SUITE_XWALK_SERVO.*
```

The XML inventory and `--gtest_list_tests` output are the current sources of
truth when cases are added.

## Build Raspberry Pi hardware tests

Configure and compile hardware tests on Linux without running them:

```bash
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-rpi --parallel
ctest --test-dir build-rpi -N -L hardware
```

Compilation on an Ubuntu host validates the Linux backend code but does not
validate a physical Robot HAT. Do not run the hardware-labelled tests on a
development host.

Before deploying to a Raspberry Pi, follow the overlay installation and
verification instructions in [`../../xWalkTool/README.md`](../../xWalkTool/README.md).
The overlay files are boot resources and are not part of the C++ build.

After deploying the source or compiled build to the intended Raspberry Pi,
confirm the Robot HAT wiring, channel assignments, actuator power, mechanical
clearance, and safe initial state. Then run the complete hardware suite:

```bash
ctest --test-dir build-rpi -L hardware --output-on-failure
```

Run one Raspberry Pi submodule suite with the same directory method, for
example:

```bash
ctest --test-dir build-rpi/xWalkHal/interface/xWalkI2c -L hardware --output-on-failure
```

Hardware tests may move motors or servos, drive GPIO/PWM outputs, sound a
buzzer, illuminate LEDs, or wait for sensor/button activity. Review the
selected submodule README before running its hardware test.

## Useful CTest commands

| Goal | Command |
|---|---|
| List all host tests | `ctest --test-dir build-host -N -L host` |
| Run all host tests | `ctest --test-dir build-host -L host --output-on-failure --parallel 4` |
| Show verbose test output | `ctest --test-dir build-host -L host --verbose` |
| Rerun only previous failures | `ctest --test-dir build-host --rerun-failed --output-on-failure` |
| Run one exact test | `ctest --test-dir build-host -R '^TEST_NAME$' --output-on-failure` |
| Run one submodule | `ctest --test-dir build-host/SUBMODULE --output-on-failure` |
| List hardware tests without running | `ctest --test-dir build-rpi -N -L hardware` |

## Clean and rebuild

Clean compiled outputs while retaining the host CMake configuration:

```bash
cmake --build build-host --target clean
```

Remove the complete host or Raspberry Pi build configuration:

```bash
cmake -E remove_directory build-host
cmake -E remove_directory build-rpi
```

Perform a completely clean host build and test run:

```bash
cmake -E remove_directory build-host
cmake -S . -B build-host -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --parallel
ctest --test-dir build-host -L host --output-on-failure --parallel 4
```

## Troubleshooting aggregate configuration

| Message | Cause | Resolution |
|---|---|---|
| Missing libsndfile development files | Native audio headers or library are absent | Install `libsndfile1-dev` |
| `../xWalkController` does not exist | The standalone CLI aggregate is missing | Restore `xWalkController` |
| Host/RPI modes conflict | Both aggregate flags were enabled | Use separate host and RPI builds |
| Linux UAPI check fails | Linux development headers are missing | Install `linux-libc-dev` |
| `No tests were found` | Verification flags are `OFF` | Enable the appropriate aggregate flag |
