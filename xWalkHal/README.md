# xWalk HAL aggregate build and test guide

The top-level `xWalkHal` CMake project builds the HAL libraries and coordinates
the tests registered by every submodule. Run the commands in this document from
the `xWalkHal` directory.

All aggregate build flags default to `OFF`. Host and Raspberry Pi verification
must use separate build directories because they enable different backends and
tests.

## Aggregate build modes

| Flag | Default | Result |
|---|---:|---|
| `XWALK_HAL_BUILD_HOST` | `OFF` | Builds host tests, the native interface, and CLI without hardware |
| `XWALK_HAL_BUILD_RPI` | `OFF` | Builds all Raspberry Pi backends and hardware-labelled tests |
| `XWALK_HAL_BUILD_CLI` | `OFF` | Builds the agent CLI without the aggregate suite |

Do not enable `XWALK_HAL_BUILD_HOST` and `XWALK_HAL_BUILD_RPI` together. The
top-level configuration automatically propagates the selected mode to every
submodule, so individual options such as `XWALK_PWM_BUILD_HOST_TESTS` and
`XWALK_I2C_BUILD_HOST_TESTS` are not needed for an aggregate build.

## Required tools and source layout

| Requirement | Purpose |
|---|---|
| CMake 3.16 or newer | Configures the aggregate project |
| C++17 compiler | Builds all native libraries and tests |
| ALSA development library | Builds the optional shared PCM and mixer backend |
| `../xWalkCLI` | Standalone CLI aggregate included by host and RPI aggregate builds |
| `../xWalkTool/dtoverlays` | Robot HAT and Servo HAT+ Raspberry Pi boot overlays |
| Linux GPIO, I2C, and SPI UAPI headers | Required when `XWALK_HAL_BUILD_RPI=ON` |

On Debian or Ubuntu, install the normal build dependencies with:

```bash
sudo apt-get install build-essential cmake libasound2-dev libcurl4-openssl-dev libsndfile1-dev linux-libc-dev
```

## Build and run every host test

Host mode is deterministic logic simulation. It must not open GPIO or I2C
devices, and it does not require a Raspberry Pi or Robot HAT.

Configure the complete host build:

```bash
cmake -S . -B build-host -DXWALK_HAL_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
```

Build every library and host test executable:

```bash
cmake --build build-host --parallel
```

List every registered host test without running it:

```bash
ctest --test-dir build-host -N -L host
```

Run every host test from every submodule:

```bash
ctest --test-dir build-host -L host --output-on-failure --parallel 4
```

Plain `ctest --test-dir build-host --output-on-failure` is equivalent in a
host-only build, but the `host` label makes the intended test class explicit.

## Run tests from one submodule

The aggregate build preserves a CTest directory for each submodule. Use that
directory to list or run only the tests belonging to the selected module.

For example, list and run only PWM tests:

```bash
ctest --test-dir build-host/xWalkPwm -N
ctest --test-dir build-host/xWalkPwm --output-on-failure
```

| Submodule | Host CTest directory | Test scope |
|---|---|---|
| xWalkAdc | `build-host/xWalkAdc` | ADC conversion and callback logic |
| xWalkAdxl345 | `build-host/xWalkAdxl345` | Accelerometer decoding and validation |
| xWalkAudio | `build-host/xWalkAudio` | Injected ALSA ownership and recovery behavior |
| xWalkBoardControl | `build-host/xWalkBoardControl` | Board control, device, and firmware information |
| xWalkBuzzer | `build-host/xWalkBuzzer` | Buzzer behavior |
| xWalkCamera | `build-host/xWalkCamera` | Capture validation and injected backend behavior |
| xWalkConfig | `build-host/xWalkConfig` | Configuration and configuration-store behavior |
| xWalkGpio | `build-host/xWalkGpio` | Callback-based GPIO behavior |
| xWalkGPT | `build-host/xWalkGPT` | Speech, ALSA, Vosk, and Espeak adapter behavior |
| xWalkI2c | `build-host/xWalkI2c` | Callback-based I2C behavior |
| xWalkLanguageModel | `build-host/xWalkLanguageModel` | Coordinator and Ollama provider behavior |
| xWalkLed | `build-host/xWalkLed` | Single-color and RGB LED behavior |
| xWalkLineTracker | `build-host/xWalkLineTracker` | Line-tracker interpretation |
| xWalkMotor | `build-host/xWalkMotor` | Motor direction and speed logic |
| xWalkMusic | `build-host/xWalkMusic` | Music behavior and shared-ALSA callback adapter |
| xWalkPwm | `build-host/xWalkPwm` | Addressing, timers, registers, percentages, frequency, and validation |
| xWalkRobot | `build-host/xWalkRobot` | Robot composition behavior |
| xWalkServo | `build-host/xWalkServo` | Initialization, angle, pulse-width, and validation behavior |
| xWalkSpeaker | `build-host/xWalkSpeaker` | Speaker behavior, bounded decoding, and ALSA adaptation |
| xWalkSpi | `build-host/xWalkSpi` | Bounded full-duplex SPI callback behavior |
| xWalkTrace | `build-host/xWalkTrace` | Trace behavior |
| xWalkUltrasonic | `build-host/xWalkUltrasonic` | Distance calculation logic |
| xWalkUserButton | `build-host/xWalkUserButton` | Button behavior |
| xWalkUtils | `build-host/xWalkUtils` | Shared utility behavior |
| xWalkVoiceAssistant | `build-host/xWalkVoiceAssistant` | Coordinator and completed-backend composition |
| xWalkController | `build-host/xWalkCLI/xWalkController` | Terminal parsing and safe dispatch behavior |

The workspace-level `../xWalkCommon` module is a header-only interface library
shared by HAL, agent, and other application layers. It does not currently
register a separate executable test.

## Run one specific test case

First obtain the exact test name:

```bash
ctest --test-dir build-host -N
```

Run one exact test using an anchored regular expression:

```bash
ctest --test-dir build-host -R '^xWalkPwmFrequencyTest$' --output-on-failure
```

Run every test whose name begins with a module prefix:

```bash
ctest --test-dir build-host -R '^xWalkServo' --output-on-failure
```

Run tests by sequence number after checking the current list:

```bash
ctest --test-dir build-host -I 1,1 --output-on-failure
```

Exact names and sequence numbers can change when tests are added, so `ctest -N`
should be treated as the current source of truth.

## Build Raspberry Pi hardware tests

Configure and compile hardware tests on Linux without running them:

```bash
cmake -S . -B build-rpi -DXWALK_HAL_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-rpi --parallel
ctest --test-dir build-rpi -N -L hardware
```

Compilation on an Ubuntu host validates the Linux backend code but does not
validate a physical Robot HAT. Do not run the hardware-labelled tests on a
development host.

Before deploying to a Raspberry Pi, follow the overlay installation and
verification instructions in [`../xWalkTool/README.md`](../xWalkTool/README.md).
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
ctest --test-dir build-rpi/xWalkI2c -L hardware --output-on-failure
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
cmake -S . -B build-host -DXWALK_HAL_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --parallel
ctest --test-dir build-host -L host --output-on-failure --parallel 4
```

## Troubleshooting aggregate configuration

| Message | Cause | Resolution |
|---|---|---|
| Missing libsndfile development files | Native audio headers or library are absent | Install `libsndfile1-dev` |
| `../xWalkCLI` does not exist | The standalone CLI aggregate is missing | Restore `xWalkCLI` |
| Host/RPI modes conflict | Both aggregate flags were enabled | Use separate host and RPI builds |
| Linux UAPI check fails | Linux development headers are missing | Install `linux-libc-dev` |
| `No tests were found` | Verification flags are `OFF` | Enable the appropriate aggregate flag |
