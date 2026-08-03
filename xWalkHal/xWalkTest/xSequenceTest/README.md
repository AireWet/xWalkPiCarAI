# xSequenceTest

The Raspberry Pi runner accepts a selector by itself and loads its board and
bounded arguments from `config/xHal_Rpi5CarSequenceTestConfig.yml`:

```sh
./build-rpi/xSequenceTest button-event
./build-rpi/xSequenceTest --config /etc/xwalk/xHal_Rpi5CarSequenceTestConfig.yml button-event
./build-rpi/xSequenceTest --config=/etc/xwalk/xHal_Rpi5CarSequenceTestConfig.yml button-event
```

Explicit positional arguments remain supported as a compatibility override.
The YAML `ai` mapping records speech-provider deployment values without storing
credentials.

Contains bounded HAL sequence and integration tests split into host-testable
core behavior and opt-in physical-hardware composition.

## Button event sequence

The `button-event` selection ports
`robot-hat/tests/button_event_test.py` to the C++ GPIO abstraction. It claims
Robot HAT D0 (GPIO17) as a pull-up input and prints timestamped `Pressed` and
`Released` events through one combined-edge registration with ten-millisecond
debounce.

The Python script installs falling and rising handlers sequentially, causing
the second registration to replace the first. The port uses the script's
commented combined-edge form so both intended events remain observable. Its
infinite sleep loop is replaced by a required duration from 1 to 3600 seconds.

Build it as part of the RPI hardware selector:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DBUILD_TESTING=ON
cmake --build build-rpi --target xSequenceTest --parallel
```

After confirming the Raspberry Pi, Robot HAT, button wiring, and GPIO device,
run the sequence directly for 30 seconds:

```sh
./build-rpi/xSequenceTest button-event 30 /dev/gpiochip0 "" ""
```

Or select it through the central hardware profile:

```sh
./build-rpi/xGoogleTest --test-profile=hardware \
    TEST_SUITE_XWALK_SEQUENCE:ButtonEvent:1
```

Run the deterministic in-memory host version through the same suite and case:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --target xGoogleTest --parallel
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:ButtonEvent:1
```

The target is labelled `hardware;sequence` in CTest. Do not execute it on an
ordinary development host; it opens the GPIO character device and claims
physical GPIO17.

## Initialization-angle sequence

The `init-angles` selection ports `robot-hat/tests/init_angles_test.py`. It
resets the Robot HAT MCU, waits the additional ten milliseconds from the Python
script, and initializes PWM channels 10, 11, and 12 to `10`, `45`, and `-45`
degrees in registration order.

The Python call passes `3` as the positional `db` argument. The C++ port uses an
explicit writable configuration path instead and writes zero calibration
offsets before initialization so the requested physical angles are
deterministic.

Run its in-memory host verification with:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:InitAngles:1
```

After confirming power, mechanical clearance, servo connections, PWM channels,
and the correct Raspberry Pi/Robot HAT, select the physical sequence with:

```sh
./build-rpi/xGoogleTest --test-profile=hardware \
    TEST_SUITE_XWALK_SEQUENCE:InitAngles:1
```

The equivalent direct command is:

```sh
./build-rpi/xSequenceTest init-angles \
    /dev/i2c-1 /dev/gpiochip0 "" "" ./build-rpi/init-angles.config
```

This operation resets the MCU and moves three servos. The hardware XML keeps it
disabled until explicitly selected.

## Robot HAT v5 motor sequence

The `motor-robothat5` selection ports
`robot-hat/tests/motor_robothat5_test.py`. Four dual-PWM motors use channel
pairs `12/13`, `14/15`, `16/17`, and `18/19`. Each bounded cycle applies
`-50%` for one second, `+50%` for one second, and then stops every motor.

The upstream infinite loop is replaced with a required cycle count from 1 to
100. Normal completion and exceptions both make an independent non-throwing
stop attempt on all four motors. Normal completion preserves the final 100 ms
delay from the Python `finally` block.

Run the in-memory host verification with:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:RobotHat5Motor:1
```

After confirming Robot HAT v5, motor wiring, external motor power, mechanical
clearance, and a safe raised-wheel setup, select one physical cycle with:

```sh
./build-rpi/xGoogleTest --test-profile=hardware \
    TEST_SUITE_XWALK_SEQUENCE:RobotHat5Motor:1
```

The equivalent direct command is:

```sh
./build-rpi/xSequenceTest motor-robothat5 1 /dev/i2c-1
```

The hardware XML keeps this motor-moving sequence disabled until explicitly
selected.

## Servo HAT sequence

The `servo-hat` selection ports `robot-hat/tests/servo_hat_test.py`. It resets
the Robot HAT MCU, preserves the script's additional one-second delay, and then
visits PWM channels zero through 15 in order. Each servo moves to `10` degrees,
waits 100 ms, moves to `0` degrees, and waits another 100 ms.

After the sweep, ADC channels zero through four are read and reported in order.
The upstream infinite monitor is replaced by a required sample count from 1 to
3600; every sample retains the original one-second delay.

Run the in-memory host verification with:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:ServoHat:1
```

After confirming Robot HAT power, mechanical clearance, safe servo linkages,
all PWM connections, and the correct Raspberry Pi devices, select one physical
ADC sample with:

```sh
./build-rpi/xGoogleTest --test-profile=hardware TEST_SUITE_XWALK_SEQUENCE:ServoHat:1
```

The equivalent direct command is:

```sh
./build-rpi/xSequenceTest servo-hat 1 /dev/i2c-1 /dev/gpiochip0 "" ""
```

The hardware XML keeps this MCU-resetting and servo-moving sequence disabled
until explicitly selected.

## Servo sequence

The `servo` selection ports `robot-hat/tests/servo_test.py`. It creates servos
for PWM channels zero through 11, moves each channel sequentially to `-20`
degrees with a 100 ms delay after every command, and then repeats the ordered
sweep at `+20` degrees.

The upstream infinite loop is replaced with a required complete-cycle count
from 1 to 100. Normal completion leaves all 12 servos at `+20` degrees, matching
the final phase of the original loop.

Run the in-memory host verification with:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:Servo:1
```

After confirming Robot HAT power, PWM connections, mechanical clearance, and
safe servo linkages, select one physical cycle with:

```sh
./build-rpi/xGoogleTest --test-profile=hardware TEST_SUITE_XWALK_SEQUENCE:Servo:1
```

The equivalent direct command is:

```sh
./build-rpi/xSequenceTest servo 1 /dev/i2c-1
```

The hardware XML keeps this servo-moving sequence disabled until explicitly
selected.

## Robot HAT motor sequence

The `motor` selection ports `robot-hat/tests/motor_test.py`. It preserves the
original PWM-plus-direction wiring: the first motor uses P13 with D4, and the
second uses P12 with D5. Each bounded cycle applies `-50%` to both motors for
one second, applies `+50%` for one second, and then stops both motors.

The upstream infinite loop is replaced with a required cycle count from 1 to
100. Normal completion and exceptions both make an independent non-throwing
stop attempt on each motor, followed by the original final 100 ms delay.

Run the in-memory host verification with:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:Motor:1
```

After confirming the correct Robot HAT revision, P13/D4 and P12/D5 wiring,
external motor power, mechanical clearance, and a safe raised-wheel setup,
select one physical cycle with:

```sh
./build-rpi/xGoogleTest --test-profile=hardware \
    TEST_SUITE_XWALK_SEQUENCE:Motor:1
```

The equivalent direct command is:

```sh
./build-rpi/xSequenceTest motor 1 \
    /dev/i2c-1 /dev/gpiochip0 "" ""
```

The hardware XML keeps this motor-moving sequence disabled until explicitly
selected.

## Piper stream comparison

The host-only `PiperStream` case ports `robot-hat/tests/test_piper_stream.py`.
It preserves the `en_US-amy-low` model, exact speech text, status-message order,
and separately timed requests with streaming enabled and disabled. The provider,
monotonic clock, and output are injected so the case runs deterministically
without synthesizing or playing audio.

Run it through the central host selector:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:PiperStream:1
```

The upstream Piper implementation comes from the external Python
`sunfounder_voice_assistant` package. This workspace currently has no C++ Piper
provider exposing both streamed and buffered modes, so no physical
`xSequenceTest` selector or hardware XML entry is registered. The existing
Espeak provider is intentionally not substituted because doing so would not
test Piper or the requested stream-mode distinction.

## Tone sequence

The `Tone` case ports the enabled portion of `robot-hat/tests/tone_test.py`.
It preserves the 72-note order across measures 1 through 17, an 80-percent
volume request, and a tempo of 60 quarter-note beats per minute. The commented
Baby Shark experiment is not executable source behavior and is intentionally
not included.

Run the deterministic in-memory version, which generates and validates PCM but
does not open an audio device, with:

```sh
./build/xGoogleTest TEST_SUITE_XWALK_SEQUENCE:Tone:1
```

The hardware case is disabled in both hardware XML manifests and its direct
CTest entry is disabled. After confirming the correct Raspberry Pi, Robot HAT,
speaker, ALSA devices, and a safe playback environment, it can be selected
explicitly with:

```sh
./build-rpi/xGoogleTest --test-profile=hardware \
    TEST_SUITE_XWALK_SEQUENCE:Tone:1
```

The equivalent direct command is:

```sh
./build-rpi/xSequenceTest tone default default PCM
```

The physical sequence produces approximately 48 seconds of audio at the
source-compatible 80-percent mixer setting. Use different explicit PCM, mixer,
and mixer-element names when the platform does not use ALSA `default` and
`PCM`.

## Source layout

| Path | Responsibility |
| --- | --- |
| `core/include/xHal_Rpi5CarButtonEventSequence.h` | Callback-driven sequence API |
| `core/src/xHal_Rpi5CarButtonEventSequence.cpp` | Platform-independent event flow |
| `core/src/xHal_Rpi5CarButtonEventSequenceTest.cpp` | In-memory host verification |
| `core/include/xHal_Rpi5CarInitAnglesSequence.h` | Initialization-angle sequence API |
| `core/src/xHal_Rpi5CarInitAnglesSequence.cpp` | MCU reset and three-servo initialization flow |
| `core/src/xHal_Rpi5CarInitAnglesSequenceTest.cpp` | In-memory initialization-angle verification |
| `core/include/xHal_Rpi5CarRobotHat5MotorSequence.h` | Bounded four-motor sequence API |
| `core/src/xHal_Rpi5CarRobotHat5MotorSequence.cpp` | Direction phases and fail-safe cleanup |
| `core/src/xHal_Rpi5CarRobotHat5MotorSequenceTest.cpp` | In-memory phase and cleanup verification |
| `core/include/xHal_Rpi5CarMotorSequence.h` | Bounded two-motor sequence API |
| `core/src/xHal_Rpi5CarMotorSequence.cpp` | PWM-and-direction phases and fail-safe cleanup |
| `core/src/xHal_Rpi5CarMotorSequenceTest.cpp` | In-memory pin, phase, and cleanup verification |
| `core/include/xHal_Rpi5CarServoHatSequence.h` | Bounded 16-servo and five-ADC sequence API |
| `core/src/xHal_Rpi5CarServoHatSequence.cpp` | Reset, sweep, and bounded sampling flow |
| `core/src/xHal_Rpi5CarServoHatSequenceTest.cpp` | In-memory sweep and ADC verification |
| `core/include/xHal_Rpi5CarServoSequence.h` | Bounded 12-channel servo-sweep API |
| `core/src/xHal_Rpi5CarServoSequence.cpp` | Ordered negative and positive sweep flow |
| `core/src/xHal_Rpi5CarServoSequenceTest.cpp` | In-memory channel-order and angle verification |
| `core/include/xHal_Rpi5CarPiperStreamSequence.h` | Injected Piper comparison contract and source text |
| `core/src/xHal_Rpi5CarPiperStreamSequence.cpp` | Streamed/buffered request and timing order |
| `core/src/xHal_Rpi5CarPiperStreamSequenceTest.cpp` | In-memory mode, timing, and reporting verification |
| `core/include/xHal_Rpi5CarToneSequence.h` | Immutable melody events and sequence API |
| `core/src/xHal_Rpi5CarToneSequence.cpp` | Volume, tempo, measure, and tone playback order |
| `core/src/xHal_Rpi5CarToneSequenceTest.cpp` | In-memory melody and generated-PCM verification |
| `hardware/include/xHal_Rpi5CarButtonEventSequenceLinux.h` | Linux callback adapter API |
| `hardware/src/xHal_Rpi5CarButtonEventSequenceLinux.cpp` | Clock, wait, and console adapters |
| `hardware/include/xHal_Rpi5CarInitAnglesSequenceLinux.h` | Linux init-angle composition API |
| `hardware/src/xHal_Rpi5CarInitAnglesSequenceLinux.cpp` | Physical GPIO, I2C, PWM, and servo composition |
| `hardware/include/xHal_Rpi5CarRobotHat5MotorSequenceLinux.h` | Linux motor composition API |
| `hardware/src/xHal_Rpi5CarRobotHat5MotorSequenceLinux.cpp` | Physical dual-PWM motor composition |
| `hardware/include/xHal_Rpi5CarMotorSequenceLinux.h` | Linux two-motor composition API |
| `hardware/src/xHal_Rpi5CarMotorSequenceLinux.cpp` | Physical PWM-and-direction motor composition |
| `hardware/include/xHal_Rpi5CarServoHatSequenceLinux.h` | Linux servo-sequence composition API |
| `hardware/src/xHal_Rpi5CarServoHatSequenceLinux.cpp` | Physical servo/ADC composition and reporting |
| `hardware/include/xHal_Rpi5CarServoSequenceLinux.h` | Linux 12-channel servo composition API |
| `hardware/src/xHal_Rpi5CarServoSequenceLinux.cpp` | Physical PWM and servo composition |
| `hardware/include/xHal_Rpi5CarToneSequenceLinux.h` | Linux ALSA tone-composition API |
| `hardware/src/xHal_Rpi5CarToneSequenceLinux.cpp` | Physical music/ALSA composition and measure output |
| `hardware/include/xHal_Rpi5CarSequenceTestRunner.h` | Central sequence CLI dispatch contract |
| `hardware/src/xHal_Rpi5CarSequenceTestRunner.cpp` | Usage, validation, selection, and Linux dispatch |
| `main.cpp` | The module's only process entry point |
| `config/xHal_Rpi5CarSequenceTestConfig.yml` | Default board, AI, and formal sequence arguments |
| `config/test_config.xml` | Enabled-by-default host selection manifest |
| `config/hardware_test_config.xml` | Disabled-by-default hardware selection manifest |

The module XML files use the same suite/case schema as `xGoogleTest`. Their
entries are mirrored into the aggregate xGoogleTest host and hardware XML files,
which perform runtime selection and strict validation. CMake also copies the
module manifests and YAML runtime configuration to the sequence-test build
directory.
