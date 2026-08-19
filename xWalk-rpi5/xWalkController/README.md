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
The `voice-active-car-gpt start|stop` command adapts example 21 as Jarvis with
the `hey jarvis` wake phrase, Gemini `gemini-3.7-flash`, the British male Piper
voice `en_GB-alan-medium`, spoken speaker replies, image input, sensor triggers,
and locally filtered actions.
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
| [`GEMINI_CONFIGURATION.md`](GEMINI_CONFIGURATION.md) | Gemini credential and voice-profile setup |
| `xWalkConfig/picar-x.conf` | Machine-independent manifest template and provider selection |
| `xWalkConfig/picar-x.d/` | Functional settings and separate AI-provider profiles |
| `PICARX_COMMAND_CHEAT_SHEET.md` | Copyable PiCar-X Controller command reference |
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

From the integration checkout, configure and build the Raspberry Pi release,
list hardware tests without running them, record the connected Robot HAT, and
run the bounded Doctor preflight:

Confirm the provisioning values on the target Raspberry Pi:

| Provisioning option | Inspection | Value to observe |
| --- | --- | --- |
| `--config` | `ls -l build-rpi/runtime/picar-x.conf` | Existing writable runtime file |
| `--profile` | Inspect the physical board revision | Confirmed `robot_hat_v4` or `robot_hat_v5` |
| `--gpio-device` | `gpiodetect` | `gpiochip4 [pinctrl-rp1] (54 lines)` |
| `--i2c-device` | `i2cdetect -l` | `/dev/i2c-1` for the intended HAT bus |
| `--spi-device` | `ls -l /dev/spidev*` | `/dev/spidev0.0` for the intended SPI device |
| V5 UUID path | `find /proc/device-tree -path '*hat*/uuid' -type f` | HAT UUID property path |
| V5 UUID value | `tr -d '\000' < UUID_FILE` | Supported Robot HAT v5 UUID |

Robot HAT v5 requires Device Tree UUID
`9daeea78-0000-076e-0032-582369ac3e02`. An absent v5 UUID does not prove that
Robot HAT v4 is connected; confirm the v4 revision physically.

Configure the Raspberry Pi release:

```bash
cd /repo/joxjoh24/xWalkPiCarAI
cmake --fresh -S xWalk-rpi5 --preset rpi-release
```

Build and list the hardware tests:

```bash
cmake --build build-rpi/cmake --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

Provision the connected hardware:

```bash
sudo xWalkTool/shell-agent/deploy-tool/provision-hardware.sh --profile robot_hat_v4 --config "$PWD/build-rpi/runtime/picar-x.conf" --gpio-device /dev/gpiochip4 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0
```

Run Doctor:

```bash
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control doctor
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --trace CTRL.024.enable doctor
```

Confirm the correct Raspberry Pi and Robot HAT before provisioning or running
Doctor. The `ctest -N` command only lists hardware tests; it does not execute
them. Doctor pulses only the configured MCU-reset GPIO and returns `0` when no
check fails or `2` when the report contains a `[FAIL]` result.
