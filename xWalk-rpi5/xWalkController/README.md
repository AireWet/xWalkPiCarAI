# xWalk CLI

Controller code qualifies generic shared types through the `ctrl` namespace
exported by `xHal_Rpi5CarTypes.h`. Agent coordinators remain under
`xwalk::agent`, and hardware-specific types remain under `hal`.

`xWalkController` is the standalone command-line application aggregate. It composes the sibling `xWalkAgent`
coordinators with the `xWalkController` parser, command dispatcher, guarded boot, and Raspberry Pi executable.
The `keyboard-control` command ports upstream interactive driving through the reusable
`xWalkKeyboardControl` Agent and line-oriented terminal input.
The `avoid-obstacles` command runs bounded `xWalkObstacleAvoidance` decisions in
the foreground and uses the shared process cancellation and motor safety policy.
The `cliff-detection` command runs the grayscale cliff-response state machine
in the foreground using persisted calibration.
The `computer-vision` command ports interactive photographs, six color modes,
face detection, QR decoding, and object geometry through a camera-only OpenCV
provider that does not claim Robot HAT resources. The `stare-at-you start|stop`
command ports example 8 and centers detected faces with bounded camera pan and
tilt corrections.
The `bull-fight start|stop` command ports example 10 and pursues a detected red
target with bounded camera servos, steering, and 50-percent requested power.
The camera-only `record-video` command ports example 9 with interactive
start, pause, continue, stop, and timestamped AVI persistence.
The `video-car` command ports example 11 with interactive speed, direction,
steering, stopping, and timestamped photo capture.
The `app-control start|stop` command ports example 12 with app telemetry,
joysticks, camera servos, voice movement, assisted driving, horn, and vision.
The interactive `sound-background-music` command ports example 13 with
foreground and background horn playback and toggled background music.
The `voice-prompt-car start|stop` command ports example 14 with an Espeak
greeting and the source's forward, backward, left, and right demonstration.
The `storytelling-robot start|stop` command ports example 15 with its Piper
greeting, two jokes, paired forward legs, farewell, and backward trip home.
The `text-vision-talk start|stop` command ports example 17 with typed prompts,
a fresh 1280-by-720 image per prompt, and a configured local Ollama vision
model.
The `online-llm-test start|stop` command ports example 18 with typed text-only
prompts and OpenAI `gpt-4o`; its credential comes only from `OPENAI_API_KEY`.
The interactive `treasure-hunt` command ports example 20 with six random color
targets, Pico2Wave prompts, bounded keyboard driving, and OpenCV detection.
The `voice-active-car-gpt start|stop` command ports example 21 with the Buddy
wake profile, OpenAI `gpt-4o-mini`, Piper `en_US-ryan-low`, image input, sensor
triggers, and preset actions.
The `voice-active-car start|stop` command ports `voice_active_car.py` with the
Rolly profile, `hey rolly` wake phrase, image input, ultrasonic safety trigger,
OpenAI `gpt-4o-mini`, and `OPENAI_API_KEY` credential boundary.
The `servo-zeroing` command ports `example/servo_zeroing.py`: it resets the
Robot HAT MCU, pulses servo channels 0 through 11 to 10 degrees, returns each
to zero, and remains active until SIGINT or SIGTERM.

The directory contains the handler implementation, standalone applications,
and controller-owned test infrastructure. Agent coordinators remain owned by
the sibling `xWalkAgent` aggregate and are imported through CMake targets.

## Layout

| Path | Responsibility |
| --- | --- |
| `CMakeLists.txt` | CLI aggregate options and Agent dependency composition |
| `xWalkConfig/picar-x.conf` | Machine-independent manifest template and provider selection |
| `xWalkConfig/picar-x.d/` | Functional settings and separate AI-provider profiles |
| `xWalkHandler/` | Controller contract, implementation, and direct in-memory test |
| `xWalkApp/` | Application build, includes, sources, generated help, and executable tests |
| `xWalkTest/xGoogleTest/` | Independent CLI unit runner and strict grouped XML inventory |
| `xWalkTest/xSequenceTest/` | Independent sequence runner and strict grouped XML inventory |

## Host verification

```bash
cmake --fresh --preset sanity
cmake --build --preset sanity --parallel
ctest --preset sanity
```

The seven handler suites are also selectable directly with `ctest --preset sanity -L handler`.

## Raspberry Pi compilation and test discovery

Run the Raspberry Pi configuration from the workspace root. CMake initializes
`build-rpi/runtime/picar-x.conf` and its fragment tree from the repository
templates without overwriting an existing generated runtime. The RPi CLI
compiles that writable generated manifest as its default:

```bash
cmake --fresh -S xWalk-rpi5 --preset rpi-release
cmake --build build-rpi/cmake --parallel
```

List the hardware tests without executing them:

```bash
ctest --test-dir build-rpi/cmake -N -L hardware
```

Only after confirming the correct Raspberry Pi and Robot HAT are connected and
safe, execute the hardware-labelled tests:

```bash
ctest --test-dir build-rpi/cmake -L hardware --output-on-failure
```

Provisioning is a separate opt-in target and is never part of an ordinary
build. Review `XWALK_RPI_PROFILE`, `XWALK_RPI_RUNTIME_USER`,
`XWALK_RPI_GPIO_DEVICE`, `XWALK_RPI_I2C_DEVICE`, `XWALK_RPI_SPI_DEVICE`,
`XWALK_RPI_CAMERA`, `XWALK_RPI_WITH_VOSK`, and `XWALK_RPI_WITH_OLLAMA` in the
CMake cache first. In particular, `/dev/gpiochip4` is a machine-specific
default and must be confirmed from the target Raspberry Pi.

From `xWalk-rpi5`, the provisioning build preset is:

```bash
cmake --build --preset rpi-provision --parallel
```

Because CMake cannot discover `xWalk-rpi5/CMakePresets.json` from its workspace
parent, use the equivalent explicit target from the workspace root:

```bash
cmake --build build-rpi/cmake --target rpi-provision --parallel
```

The target depends on `xwalk-picarx-control`, runs the approved user-local
camera and Ollama setup when enabled, initializes the writable runtime from
repository templates when necessary, installs repository-controlled Vosk
assets when enabled, and invokes `setup-rpi.sh`. The setup script validates the
selected Robot HAT and device identities before modifying the generated runtime.
It never changes ownership, permissions, or values in the tracked templates.

After provisioning, the CLI uses the generated runtime without
`--deployment-config`:

```bash
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control doctor
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --trace CTRL.024.enable doctor
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --help
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --diagnose --no-hardware
```

Doctor is a bounded hardware preflight that pulses only the configured MCU
reset GPIO. It prints its report directly; `CTRL.024` additionally enables the
same lines in trace output. Doctor returns zero when all required checks pass
and two when any `[FAIL]` result exists. The `--diagnose --no-hardware` command
is the device-free check.
See the [deployment guide](../../devloper-note/xwalk-rpi5-note/Doc/note/Deployment%20Guide.md) for installed
paths, package selection, Robot HAT safeguards, and permission policy.
