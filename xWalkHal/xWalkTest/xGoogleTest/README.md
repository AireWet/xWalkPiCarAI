# xGoogleTest

`xGoogleTest` is the single GoogleTest selector for xWalk HAL host and physical
hardware verification. It compiles host test sources from their owning module
directories and dispatches existing module hardware executables in the RPI
profile; no module test is copied into this directory. The three files under
`src/` only implement configuration, selection, and the central process entry
point.

The pre-existing HAL tests use `assert`-based standalone entry points rather
than `TEST` or `TEST_F` declarations. CMake compiles each entry point in an
object library with a unique symbol name, and `TestRunner` registers those
scenarios dynamically with GoogleTest. Each legacy scenario executes in an
isolated child process so an assertion, signal, or non-zero result is reported
as one failed GoogleTest case without stopping the remaining cases.

## Dependencies and build

The host test build requires GoogleTest, TinyXML2, and yaml-cpp development
packages.
On Debian or Ubuntu:

```sh
sudo apt-get install libgtest-dev libtinyxml2-dev libyaml-cpp-dev
```

From the repository root:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
./build/xGoogleTest
ctest --test-dir build --output-on-failure
```

The executable is always written directly to `${CMAKE_BINARY_DIR}`. CMake
copies both XML configurations and the default hardware runtime YAML file to
`${CMAKE_BINARY_DIR}`. The runner resolves that directory from `/proc/self/exe`,
so execution does not depend on the current working directory or
configuration-path preprocessor definitions. With
`BUILD_TESTING=OFF`, this target and its external test dependencies are not
configured.

## Selection

Every enabled XML entry runs when no selection is supplied:

```sh
./build/xGoogleTest
```

Runtime selections override the XML for that process only:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_I2C:1
./build/xGoogleTest TEST_SUITE_XWALK_I2C:Initialization:1
./build/xGoogleTest TEST_SUITE_XWALK_I2C:0
./build/xGoogleTest TEST_SUITE_XWALK_I2C:Initialization:0
```

Supported custom forms are `<SUITE>:<0|1>` and
`<SUITE>:<CASE>:<0|1>`. If at least one custom entry is enabled, the enabled
custom entries form an allowlist and custom disables are then applied. If all
custom entries are disables, the XML selection is used as the starting set.
Unknown suites, unknown cases, and malformed values produce an error, print the
complete valid inventory, and return a non-zero status.

Standard GoogleTest flags remain available:

```sh
./build/xGoogleTest --gtest_list_tests
./build/xGoogleTest --gtest_filter=TEST_SUITE_XWALK_PWM.Address
./build/xGoogleTest --gtest_repeat=2
```

Filter precedence is custom runtime selection, explicit `--gtest_filter`, then
XML configuration.

## Hardware profile

Physical cases use `config/hardware_test_config.xml`. Every hardware suite and
case is disabled by default. The profile is compiled only by an RPI-enabled
test build and is not registered as the central host CTest entry.

Board and AI runtime values are separate from XML test selection. They are
stored in `config/xHal_Rpi5CarGoogleTestConfig.yml`, copied beside `xGoogleTest`,
and may be overridden without editing the checked-in file:

The generated default uses the target-selected ARM64 or x86-64 Vosk runtime
and the shared small US English model under the root-level `xWalkLibrary/common/models`. Override
`XWALK_VOSK_ARCHITECTURE`, `XWALK_VOSK_LIBRARY_PATH`, or
`XWALK_VOSK_MODEL_PATH` during CMake configuration for another target or
deployment layout.

```sh
./build-rpi/xGoogleTest --test-profile=hardware --runtime-config=/etc/xwalk/xHal_Rpi5CarGoogleTestConfig.yml TEST_SUITE_XWALK_I2C:Probe:1
```

The YAML file supplies formal arguments for configurable hardware executables
and full formal arguments for each `xSequenceTest` dispatch. Consequently, a
custom runtime YAML controls the board paths used by the invoked sequence.
XML continues to control which tests are enabled.

Configure and compile it without executing hardware:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DBUILD_TESTING=ON
cmake --build build-rpi --target xGoogleTest --parallel
```

List all hardware cases without running them:

```sh
./build-rpi/xGoogleTest --test-profile=hardware --gtest_filter='*' --gtest_list_tests
```

Only after confirming the Raspberry Pi, Robot HAT, wiring, power, and
mechanical clearance, run one selected suite or case:

```sh
./build-rpi/xGoogleTest --test-profile=hardware TEST_SUITE_XWALK_I2C:1

./build-rpi/xGoogleTest --test-profile=hardware TEST_SUITE_XWALK_I2C:Probe:1
```

The override is temporary and does not edit either XML file. Running with only
`--test-profile=hardware` executes no cases because the hardware XML defaults
to disabled. Supplying `--gtest_filter='*'` without `--gtest_list_tests`
explicitly selects every physical case and must be treated as a full hardware
run.

## XML configuration

`config/test_config.xml` contains 26 host suites and 54 host-safe cases.
`config/hardware_test_config.xml` contains 22 hardware suites and 32 physical
cases. A suite and each child case must have an exact registered name and an
`enabled` value of `0` or `1`. The loader rejects malformed XML, missing or
unknown entries, duplicate suites or cases, and invalid attributes before any
test runs.

The suite inventory maps to these existing source locations:

| Suite | Owning test source directory |
| --- | --- |
| `TEST_SUITE_XWALK_I2C` | `xWalkI2c/test` |
| `TEST_SUITE_XWALK_SPI` | `xWalkSpi/test` |
| `TEST_SUITE_XWALK_GPIO` | `xWalkGpio/test` |
| `TEST_SUITE_XWALK_AUDIO` | `xWalkAudio/hardware/test` |
| `TEST_SUITE_XWALK_CONFIG` | `xWalkConfig/test` |
| `TEST_SUITE_XWALK_TRACE` | `xWalkTrace/test` |
| `TEST_SUITE_XWALK_UTILS` | `xWalkUtils/test`, `xWalkUtils/hardware/test` |
| `TEST_SUITE_XWALK_LANGUAGE_MODEL` | `xWalkLanguageModel/test`, `xWalkLanguageModel/hardware/test` |
| `TEST_SUITE_XWALK_MUSIC` | `xWalkMusic/test`, `xWalkMusic/hardware/test` |
| `TEST_SUITE_XWALK_SPEAKER` | `xWalkSpeaker/test`, `xWalkSpeaker/hardware/test` |
| `TEST_SUITE_XWALK_PWM` | `xWalkPwm/test` |
| `TEST_SUITE_XWALK_ADC` | `xWalkAdc/test` |
| `TEST_SUITE_XWALK_SERVO` | `xWalkServo/test` |
| `TEST_SUITE_XWALK_ADXL345` | `xWalkAdxl345/test` |
| `TEST_SUITE_XWALK_LINE_TRACKER` | `xWalkLineTracker/test` |
| `TEST_SUITE_XWALK_ULTRASONIC` | `xWalkUltrasonic/test` |
| `TEST_SUITE_XWALK_MOTOR` | `xWalkMotor/test` |
| `TEST_SUITE_XWALK_LED` | `xWalkLed/test` |
| `TEST_SUITE_XWALK_BUZZER` | `xWalkBuzzer/test` |
| `TEST_SUITE_XWALK_CAMERA` | `xWalkCamera/test` |
| `TEST_SUITE_XWALK_USER_BUTTON` | `xWalkUserButton/test` |
| `TEST_SUITE_XWALK_BOARD_CONTROL` | `xWalkBoardControl/test` |
| `TEST_SUITE_XWALK_ROBOT` | `xWalkRobot/test` |
| `TEST_SUITE_XWALK_GPT` | `xWalkGPT/test`, `xWalkGPT/hardware/test` |
| `TEST_SUITE_XWALK_VOICE_ASSISTANT` | `xWalkVoiceAssistant/test`, `xWalkVoiceAssistant/hardware/test` |
| `TEST_SUITE_XWALK_SEQUENCE` | `xWalkTest/xSequenceTest/core`, `xWalkTest/xSequenceTest/hardware` |

Files below a module's `hardware/test` directory are included only when they
are device-free adapter tests. Tests that open physical GPIO, I2C, SPI, audio,
camera, sensor, or actuator interfaces remain in their module CMake targets and
stay explicitly opt-in. The optional libsndfile decoder test remains governed
by `XWALK_MUSIC_BUILD_SNDFILE_DECODER` and is not part of the normal host suite.

## Adding a host case

Keep the source in the owning module test directory. Add it explicitly to
`CMakeLists.txt`, register its suite and case in `TestRunner.cpp`, and add the
same exact entry to `config/test_config.xml`. Do not create `src/tests/` and do
not add a second unrenamed `main()` to the central executable.
