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
| `xWalkConfig/picar-x.conf` | Writable manifest, provider selection, and calibration overrides |
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

The repository provides a dry-run-first setup tool for required packages,
interfaces, device groups, and exact-node permissions. After installing the
RPi build, preview setup with an explicit board profile and GPIO controller:

```bash
/usr/lib/xwalk/setup-rpi.sh --dry-run --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
```

Review the plan and repeat it with `--apply`. Use `--camera usb` for a USB V4L2
webcam. Installation includes the architecture-selected `xWalkLibrary` Vosk
runtime at `/usr/lib/xwalk/libvosk.so` and its shared model under
`/usr/share/xwalk/models/vosk`. Configure ALSA device names in the active
`/var/lib/xwalk/picar-x.d/voice.conf` before running the command. The manifest
loads resources, vehicle, hardware, voice, vision, connectivity, AI-feature,
and one selected generic AI-provider file in declaration order.
The SPI command additionally requires an enabled `/dev/spidev*` node configured
with the target peripheral's mode, clock speed, and bits per word.

```bash
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

Install the complete deployment layout after a successful build:

```bash
DESTDIR="$PWD/build-host/deploy" cmake --install build-host/cmake
```

See the [deployment guide](../devloper-note/Doc/note/Deployment%20Guide.md) for the installed
paths, package list, Robot HAT revision safeguards, and permission policy.

The last command lists hardware tests without executing them. Do not execute those tests until the correct
Raspberry Pi and Robot HAT are connected and the robot has been placed in a safe test configuration.
