# xWalkController

`xWalkController` provides the
`xwalk-picarx-control` Raspberry Pi executable and a hardware-independent parser and command coordinator.

The standalone CLI build imports `xWalkPicarx`, `xWalkLineTracking`, `xWalkSelfDrive`, and
`xWalkBoot` as adjacent Agent dependencies. Their host tests remain owned
by their respective submodules and by the aggregate `xWalkAgent` suite. The CLI
composes line tracking and self-drive only for their corresponding commands.

## Backend boot

`XWalkBootRpi` is the process hardware-composition owner. Its one-shot `run()`
method invokes one application callback and rejects every later attempt through
the same boot object. A failed or throwing callback consumes the attempt so
partially opened hardware cannot be initialized again through that object.

Before claiming hardware, boot loads `config/picar-x.conf`, resolves the
configured Device Tree, I2C, and GPIO device paths, validates the requested HAT
revision, and optionally verifies the exact GPIO chip name and label. It does
not scan `/dev/gpiochip*` or guess from filename order. The default `auto` board
mode fails closed when the supported v5 UUID is absent; legacy v4 deployments
must select `hardware_board = robot_hat_v4` explicitly.

The RPi boot module creates the selected backend graph before invoking the CLI callback.
I2C, GPIO, PWM, servo, ADC, ultrasonic, motor, PiCar-X, and configuration
objects remain alive until command completion. Line tracking, shared ALSA audio,
speech, model, LED, and Music services are initialized only for commands that
select them. Board control and its reset and speaker GPIO objects remain alive
for the command so speaker-enable ownership is preserved during playback.

The complete `xWalkHal` inventory also contains utilities, speaker, and other
optional backends. The CLI composes only the graph selected by the parsed command.
`--help` returns before constructing the boot object, so it does not claim I2C,
GPIO, audio, microphone, or model resources.

`doctor` selects a separate passive boot graph. It reports board-profile and
Device Tree agreement, I2C and firmware response, battery voltage, GPIO chip
metadata and identity, SPI open availability, camera and ALSA metadata,
configuration permissions, executables, Vosk, and configured model resources.
It does not reset the MCU, request a GPIO line, construct PWM, servo, or motor
objects, transfer SPI payloads, enable audio, capture media, or contact Ollama.
Checks prefixed `[FAIL]` produce status 2; `[WARN]` is advisory.

## Complete CLI command and action reference

The following table lists every command group and action name accepted by the CLI:

| Command group | Accepted actions | Additional arguments |
| --- | --- | --- |
| `doctor` | None | Passive deployment report |
| `move` | `forward`, `backward` | Optional `--speed N` and `--duration S` |
| `turn` | `left`, `right` | Optional `--angle N` |
| `cam` | `pan`, `tilt` | Required `--angle N` |
| `sensor` | `distance`, `grayscale` | None |
| `spi` | `transfer` | One contiguous hexadecimal payload |
| `line-track` | `start`, `stop` | None |
| `self-drive` | See the complete preset-action list below | None |
| `sound` | `play`, `volume`, `music`, `stop` | File or volume according to the selected action |
| `voice-chat` | `start`, `stop` | Start runs until SIGINT or SIGTERM |
| `voice-active-car` | `start`, `stop` | Base sensor-aware voice-car behavior |
| `voice-active-car-gpt` | `start`, `stop` | English Buddy profile; configured model |
| `voice-controlled-car` | `start`, `stop` | “Hey robot” movement-command loop |
| `voice-prompt-car` | `start`, `stop` | Spoken four-movement demonstration |
| `calibrate` | No action name | Interactive calibration |

Complete command shapes:

```text
xwalk-picarx-control [--deployment-config ABSOLUTE_PATH] [--resource-directory ABSOLUTE_PATH] <command>
xwalk-picarx-control doctor
xwalk-picarx-control move <forward|backward> [--speed N] [--duration S]
xwalk-picarx-control turn <left|right> [--angle N]
xwalk-picarx-control cam <pan|tilt> --angle N
xwalk-picarx-control sensor <distance|grayscale>
xwalk-picarx-control spi transfer <HEX>
xwalk-picarx-control line-track <start|stop>
xwalk-picarx-control self-drive <action-name>
xwalk-picarx-control sound <play|volume|music|stop> [file|volume] [--volume N]
xwalk-picarx-control voice-chat <start|stop>
xwalk-picarx-control voice-active-car <start|stop>
xwalk-picarx-control voice-active-car-gpt <start|stop>
xwalk-picarx-control voice-controlled-car <start|stop>
xwalk-picarx-control voice-prompt-car <start|stop>
xwalk-picarx-control calibrate
```

Use `xwalk-picarx-control --help` or `xwalk-picarx-control -h` for command descriptions, option ranges, and
examples.

Example commands:

```bash
xwalk-picarx-control doctor
xwalk-picarx-control move forward --speed 40 --duration 2.5
xwalk-picarx-control turn left --angle 20
xwalk-picarx-control cam pan --angle 45
xwalk-picarx-control sensor distance
xwalk-picarx-control spi transfer 9F000000
xwalk-picarx-control line-track start
xwalk-picarx-control line-track stop
xwalk-picarx-control self-drive wave-hands
xwalk-picarx-control sound play sounds/car-double-horn.wav --volume 80
xwalk-picarx-control sound music music/slow-trail-Ahjay_Stelino.mp3 --volume 20
xwalk-picarx-control sound volume 60
xwalk-picarx-control voice-chat start
xwalk-picarx-control voice-chat stop
xwalk-picarx-control voice-active-car start
xwalk-picarx-control voice-active-car-gpt start
xwalk-picarx-control voice-controlled-car start
xwalk-picarx-control voice-prompt-car start
xwalk-picarx-control calibrate
```

Every command that composes the PiCar-X actuator graph owns a scope-bound emergency-stop guard. The guard
attempts both motors independently on normal return and after an escaping backend failure. SIGINT and SIGTERM
only change the application-wide shutdown request; the controlling thread observes it and performs hardware
cleanup. Move, turn, self-drive, line-tracking, and voice-controlled movement share this policy, and bounded
movement waits poll cancellation in slices no longer than 20 milliseconds.

Steering is limited to 30 degrees. `picarx_max_motor_output_percent` limits the final calibrated PWM magnitude.
Until `calibrate` records successful raised-wheel motor-direction, motor-balance, and steering-center checks,
the effective limit cannot exceed 20 percent. Raise the configured limit only after those checks. Grayscale
acquisition performs a warm-up, five-sample filtering, poisoned-ADC signature rejection, automatic reference,
line status, and cliff output.

The `calibrate` workflow clears prior verification before actuator checks. After servo calibration, it requires
the operator to confirm that all wheels are raised, runs the left motor, right motor, and paired motors through
bounded low-output checks, and records verification only when motor direction, balance, and steering center are
all confirmed. It persists the signed `picarx_motor_speed_calibration` correction. It then stops the motors
before sampling, displaying, confirming, and persisting `line_reference` and `cliff_reference`. Cancellation
or a negative actuator response leaves the 20-percent gate active.

`spi transfer` creates only the configured Linux SPI backend and SPI Agent. It
does not detect or reset the Robot HAT and does not claim motors, GPIO, audio,
or camera resources. The payload accepts an optional `0x` prefix and 2 through
512 contiguous hexadecimal digits. Received bytes are printed as uppercase
space-separated hexadecimal text. Configure the device, speed, mode, and word
size in `config/picar-x.conf` before connecting a peripheral.

`line-track start` runs in the foreground and repeatedly executes the Agent
module's bounded `step()` operation. Press Ctrl+C, send SIGINT, or send SIGTERM
to end the loop; the command then stops both motors before returning.
`line-track stop` sends an immediate stop through a freshly composed
line-tracking coordinator and exits. It does not signal another foreground CLI
process, so use its signal-based cancellation for graceful shutdown.

`voice-chat start` dispatches the foreground `XWalkLocalVoiceChatbot` Agent and
uses SIGINT or SIGTERM for graceful cancellation. `voice-chat stop` requests
shutdown through a freshly composed service; like `line-track stop`, it does
not signal another foreground CLI process. The RPi composition supplies Vosk,
Espeak, ALSA, the configured language-model HTTP backend, and speaker-power
ownership from deployment configuration.

The two voice-active-car commands use the common sensor/action coordinator with
base and English Buddy profiles. Each accepts only `start` or `stop`. The RPi graph
adds Music, SelfDrive, the Robot HAT status LED, and still-image capture. Set
`camera_connection` to `csi` for the Raspberry Pi camera connector or `usb` for
a V4L2 webcam. The corresponding `rpicam-still` or `ffmpeg` provider must be installed.

`voice-controlled-car` provides Vosk wake-word recognition; `voice-prompt-car`
provides an Espeak movement demonstration. Both use the shared voice
activation module and accept only `start` or `stop`. The RPi boot graph connects
Vosk to ALSA microphone capture and connects Espeak PCM to shared ALSA playback.
Configure library, model, voice, PCM, camera, capture, and mixer values in
`config/picar-x.conf`. Missing runtime packages or invalid device names produce
an explicit startup error before the voice Agent begins moving the vehicle.

Every `xWalkSelfDrive` action has one canonical shell-friendly CLI command:

```text
xwalk-picarx-control self-drive shake-head
xwalk-picarx-control self-drive nod
xwalk-picarx-control self-drive wave-hands
xwalk-picarx-control self-drive resist
xwalk-picarx-control self-drive act-cute
xwalk-picarx-control self-drive rub-hands
xwalk-picarx-control self-drive think
xwalk-picarx-control self-drive twist-body
xwalk-picarx-control self-drive celebrate
xwalk-picarx-control self-drive depressed
xwalk-picarx-control self-drive forward
xwalk-picarx-control self-drive backward
xwalk-picarx-control self-drive honking
xwalk-picarx-control self-drive start-engine
```

The preset actions perform the following operations:

| Action name | Operation |
| --- | --- |
| `shake-head` | Runs the decreasing head-shake gesture |
| `nod` | Runs the repeated camera-tilt nod gesture |
| `wave-hands` | Runs the steering wave gesture |
| `resist` | Runs the steering and camera resistance gesture |
| `act-cute` | Runs the low-speed cute-shaking gesture |
| `rub-hands` | Runs the small steering-angle rubbing gesture |
| `think` | Runs the thinking pose and returns to center |
| `twist-body` | Runs the motor, steering, and camera twist gesture |
| `celebrate` | Runs the mirrored celebration gesture |
| `depressed` | Runs the downward camera-tilt gesture |
| `forward` | Drives forward briefly and stops |
| `backward` | Drives backward briefly and stops |
| `honking` | Plays the horn sound in the background |
| `start-engine` | Plays the engine-start sound in the background |

For compatibility, multi-word actions may also be quoted or passed as separate
shell arguments, such as `self-drive "wave hands"` or `self-drive wave hands`.
The RPi application composes the shared ALSA music backend only for self-drive
and standalone sound commands, preserving audio-device ownership for all other command groups.

The executable detects the Robot HAT revision before composing board-specific resources. MCU reset always uses
GPIO5 through the `MCURST` name. Speaker enable uses GPIO20 for legacy Robot HAT
v4 boards and GPIO12 for Robot HAT v5. A legacy board receives PWM-and-direction
motors on P13/D4 and P12/D5. Robot HAT v5 receives dual-PWM
motors on P12/P13 and P14/P15, without claiming the legacy direction GPIOs. The executable creates fresh ADC
objects after reset and stores calibration in `/var/lib/xwalk/picar-x.conf` by default. Use the global
`--deployment-config` option for another absolute deployment path.

Use `config/picar-x-v4.conf` and `config/picar-x-v5.conf` as explicit board-profile templates. Run
`xWalkTool/shell/provision-hardware.sh --profile <profile> --config <file>` to discover and record one GPIO device,
kernel chip name, and label. Provisioning rejects a v4 selection when the v5 UUID is present and rejects a v5
selection when that UUID is absent. It never interprets failure to detect v5 as proof of v4.

The Robot HAT v5 PWM pairs follow the upstream SunFounder test mapping. Confirm the physical left/right ports
and direction polarity with raised wheels before allowing a movement command to run on a new hardware setup.

The command help is maintained as readable lines in `config/help.json`. CMake generates the private C++ help
constant from that file, keeping the JSON configuration and executable output synchronized.

The RPi executable composes a real shared-ALSA Music backend for the standalone `sound` command. Sound-effect
and music operations play synchronously so the one-shot process retains its audio objects until playback
completes. `sound volume 80` deliberately consumes the documented positional volume value. `sound stop`
applies only to playback owned by the same process; it cannot control an earlier CLI process.
Speaker power is enabled through the board-selected GPIO only for `self-drive` and `sound`, and that GPIO
remains claimed until the selected command and its audio composition are destroyed.

The root-level [`xWalkSounds`](../../xWalkSounds) and [`xWalkMusics`](../../xWalkMusics) directories contain
the packaged sound-effect and background-music files used in CLI examples. Run the executable from `xWalkCLI`
when using the documented relative paths, or provide deployment-specific paths explicitly.
The RPi composition injects the optional libsndfile decoder, which accepts the packaged MP3 as well as the
PCM WAVE sound effects. Decoded audio is bounded to 64 MiB of signed 16-bit PCM per operation. Raspberry Pi
builds therefore require the `libsndfile1-dev` package in addition to the ALSA development package.

## Build and test

```bash
cmake -S xWalkCLI -B xWalkCLI/build-host -DXWALK_CLI_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkCLI/build-host --parallel
ctest --test-dir xWalkCLI/build-host --output-on-failure
```

The RPi build compiles the application and hardware-labelled tests. Do not execute them without a safe robot:

```bash
cmake -S xWalkCLI -B xWalkCLI/build-rpi -DXWALK_CLI_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkCLI/build-rpi --parallel
ctest --test-dir xWalkCLI/build-rpi -N -L hardware
```

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarController.h` | CLI contract and command dispatch |
| `include/xAgent_Rpi5CarControllerHelp.h` | Generated-help selection and source fallback |
| `include/xAgent_Rpi5CarControllerTypes.h` | Platform callback, option, and sound types |
| `config/help.json` | Build-time source for the Linux-style command help text |
| `config/picar-x.conf` | Default writable calibration and grayscale configuration |
| `src/*Lifecycle.cpp` | Dependency binding and callback forwarding |
| `src/*Parsing.cpp` | Named-option parsing, validation, conversion, and formatting |
| `src/*Commands.cpp` | Movement, line tracking, preset action, sensor, sound, and calibration commands |
| `app/*Main.cpp` | Raspberry Pi boot-mode selection and console callbacks |
| `test/src/*Test.cpp` | Deterministic in-memory command tests |

The `xWalkController` target links the three Agent coordinators so a standalone CLI
consumer receives the same dependency set as the aggregate Agent build.
