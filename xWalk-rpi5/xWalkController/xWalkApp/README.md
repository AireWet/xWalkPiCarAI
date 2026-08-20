# xWalkApp

`xWalkApp` owns and builds the `xwalk-picarx-control` host and Raspberry Pi
entry points, generated-help resources, and application GoogleTest. The
sibling `xWalkHandler` directory owns typed command handlers, Controller
lifecycle behavior, and the direct in-memory test.

Controller application APIs use `namespace xwalk::ctrl`. Agent service
objects used by the hardware composition remain in `namespace xwalk::agent`.

Application code is grouped under `cli`, `parse`, `boot`, and `activate`. The
`cli/host` and `cli/hardware` entry behavior stays separate while both use
the same parsing, boot-support, and typed command-activation contracts. Command
dispatch converts CLI text into the typed request structures from
`xWalkControllerConfigTypes.h` before invoking a handler.
The complete command-to-endpoint trace is documented in
[Controller Command Flow](../../../devloper-note/xwalk-rpi5-note/Doc/note/Controller%20Command%20Flow.md),
including the boot
macro, typed request,
handler, Agent or HAL service, and final hardware, file, process, or network boundary.
`xControllerApplicationSupport.cpp` separately owns
signal state and the terminal, timing, and audio callback adapters.
`xControllerBootMode.cpp` maps each top-level command to its minimum Agent boot
graph before any hardware service is claimed. `xControllerRunner.cpp` owns the
command-specific Controller and temporary Agent composition performed while the
selected boot graph retains its services.
`xControllerParsing.cpp` owns free functions that convert command text into
typed requests and format stable Controller output values.

The standalone CLI build imports `xWalkPicarx`, `xWalkLineTracking`, `xWalkSelfDrive`, and
`xWalkBoot` as adjacent Agent dependencies. Their host tests remain owned
by their respective submodules and by the aggregate `xWalkAgent` suite. The CLI
composes line tracking and self-drive only for their corresponding commands.

## Backend boot

`XWalkBootRpi` is the process hardware-composition owner. Its one-shot `run()`
method invokes one application callback and rejects every later attempt through
the same boot object. A failed or throwing callback consumes the attempt so
partially opened hardware cannot be initialized again through that object.

Before claiming hardware, boot loads the manifest at `../xWalkConfig/picar-x.conf`
and its relative `picar-x.d` fragments, resolves the
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

## Device-free configuration diagnostics

The host and Raspberry Pi entry points can validate the layered deployment
manifest before constructing a boot backend. `--validate-config` reports each
known safety and integration invariant. `--print-effective-config` additionally
prints the stable known schema in effective-value order; secret-shaped values
are redacted. `--diagnose` requires `--no-hardware`, which makes the no-device
intent explicit. These actions reject robot commands and never open I2C, GPIO,
SPI, camera, audio, or network resources.

```bash
xwalk-picarx-control --deployment-config=/absolute/path/to/picar-x.conf --validate-config
xwalk-picarx-control --deployment-config=/absolute/path/to/picar-x.conf --print-effective-config
xwalk-picarx-control --deployment-config=/absolute/path/to/picar-x.conf --diagnose --no-hardware
```

This is schema and layering evidence only. A passing report does not prove that
the named device nodes exist, that the Robot HAT revision matches, or that any
physical channel is wired correctly. Use the ordinary `doctor` command later on
the Raspberry Pi for its bounded MCU-reset hardware preflight, then follow the wheels-up
commissioning sequence before enabling actuator commands.

The repeatable global `--trace VALUE` option atomically updates the shared XML
and trace registry before the selected boot graph is constructed. Newly found
normal traces start disabled, while saved states load automatically on later
runs. Values are `all.<state>`, `<module>.<state>`,
`<module>.<numeric-id>.<state>`, or a `.json` file, where state is exactly
`enable` or `disable`. Arguments are applied from left to right and the last
applicable setting wins. JSON applies `all`, module, then tag states. Unknown
modules and IDs, Boolean JSON states, missing files, malformed JSON, and invalid
selectors return status 2 without constructing hardware. Legacy
`--trace-enable UID` and `--trace-disable UID` forms remain accepted.

Every numeric trace value must be unique within its `RPI`, `CTRL`, `RPIAGENT`,
or `LIB` tag across all modules and submodules. Source-tree ownership is also
checked during the build. Equal numeric values in different tags are valid,
while `RPI.001` and `RPI.1` conflict. The validation error reports all
duplicated IDs and every declaration path and line. A successful normal build generates
the deterministic persistent catalogue at
`<build-directory>/generated/xwalk-traces.xml`.

`doctor` selects a separate bounded preflight graph and prints its report directly. It reports board-profile and
Device Tree agreement, I2C and firmware response, battery voltage, GPIO chip
metadata and identity, SPI open availability, camera and ALSA metadata,
configuration permissions, executables, Vosk, and configured model resources.
It validates the configured GPIO identity, drives only `hardware_mcu_reset_pin`
low for 10 ms and then high, and waits the configured settle interval before
I2C inspection. It does not construct PWM, servo, or motor objects, transfer
SPI payloads, enable audio, capture media, or contact Ollama.
For an explicitly provisioned v4 profile, Doctor reports Robot HAT verification
PASS only when the supported v5 UUID is absent, the exact GPIO identity matches,
reset completes, the MCU answers at `0x14` or `0x15`, firmware is read, and the
battery ADC returns a sample. This operational evidence does not turn absence
of the v5 UUID into automatic v4 identification. V5 and `auto` still require
the supported v5 UUID. Safety reports PASS only after the explicit
MCU-reset-only operation invariant succeeds.
Checks prefixed `[FAIL]` produce status 2; `[WARN]` is advisory. Enabling
`CTRL.024` duplicates the report through the trace backend for trace capture.

## Complete CLI command and action reference

The following table lists every command group and action name accepted by the CLI:

| Command group | Accepted actions | Additional arguments |
| --- | --- | --- |
| `doctor` | None | Bounded MCU-reset deployment report |
| `move` | `forward`, `backward`, `demo` | Options apply only to direct movement |
| `turn` | `left`, `right` | Optional `--angle N` |
| `cam` | `pan`, `tilt` | Required `--angle N` |
| `sensor` | `distance`, `grayscale` | None |
| `spi` | `transfer` | One contiguous hexadecimal payload |
| `line-track` | `start`, `stop` | None |
| `computer-vision` | Interactive keys | No command-line options |
| `stare-at-you` | `start`, `stop` | Track faces with bounded camera servos |
| `bull-fight` | `start`, `stop` | Pursue a detected red target |
| `treasure-hunt` | Interactive keys | Spoken random-color driving game |
| `record-video` | Interactive keys | Timestamped AVI start, pause, and stop |
| `video-car` | Interactive keys | Drive with camera acquisition and photos |
| `video-stream` | No arguments | Serve loopback MJPEG until cancellation |
| `app-control` | `start`, `stop` | Explicitly configured mobile-app control |
| `sound-background-music` | Interactive keys | Foreground horn, background horn, and music toggle |
| `self-drive` | See the complete preset-action list below | None |
| `sound` | `play`, `volume`, `music`, `stop` | File or volume according to the selected action |
| `voice-chat` | `start`, `stop` | Start runs until SIGINT or SIGTERM |
| `voice-active-car` | `start`, `stop` | Base sensor-aware voice-car behavior |
| `voice-active-car-gpt` | `start`, `stop` | Gemini-backed Jarvis profile |
| `gpt-car` | `start`, `stop` | Optional `--keyboard` and `--no-img` source flags |
| `voice-controlled-car` | `start`, `stop` | “Hey robot” movement-command loop |
| `voice-prompt-car` | `start`, `stop` | Spoken four-movement demonstration |
| `storytelling-robot` | `start`, `stop` | Piper-narrated outward and home journey |
| `text-vision-talk` | `start`, `stop` | Image-grounded typed Ollama conversation |
| `online-llm-test` | `start`, `stop` | Typed OpenAI text conversation |
| `servo-zeroing` | None | Pulse and zero Robot HAT channels 0 through 11 |
| `calibrate` | No action name | Interactive calibration |

Complete command shapes:

```text
xwalk-picarx-control [--deployment-config PATH] [--resource-directory PATH] <command>
xwalk-picarx-control --deployment-config PATH --validate-config
xwalk-picarx-control --deployment-config PATH --print-effective-config
xwalk-picarx-control --deployment-config PATH --diagnose --no-hardware
xwalk-picarx-control doctor
xwalk-picarx-control move <forward|backward> [--speed N] [--duration S]
xwalk-picarx-control move demo
xwalk-picarx-control keyboard-control
xwalk-picarx-control avoid-obstacles start
xwalk-picarx-control avoid-obstacles stop
xwalk-picarx-control cliff-detection start
xwalk-picarx-control cliff-detection stop
xwalk-picarx-control computer-vision
xwalk-picarx-control stare-at-you start
xwalk-picarx-control bull-fight start
xwalk-picarx-control treasure-hunt
xwalk-picarx-control record-video
xwalk-picarx-control sound-background-music
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
xwalk-picarx-control gpt-car <start|stop> [--keyboard] [--no-img]
xwalk-picarx-control voice-controlled-car <start|stop>
xwalk-picarx-control voice-prompt-car <start|stop>
xwalk-picarx-control storytelling-robot <start|stop>
xwalk-picarx-control text-vision-talk <start|stop>
xwalk-picarx-control online-llm-test <start|stop>
xwalk-picarx-control servo-zeroing
xwalk-picarx-control calibrate
```

Relative deployment and resource paths are resolved from the process working
directory. From the workspace root, run the Raspberry Pi build's bounded Doctor
graph with its compiled generated-runtime configuration:

```bash
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control doctor
```

Use `xwalk-picarx-control --help` or `xwalk-picarx-control -h` for command
descriptions, option ranges, trace selectors, and examples.

Example commands:

```bash
xwalk-picarx-control --trace RPI.001.enable
xwalk-picarx-control --trace RPI.001.disable
xwalk-picarx-control --trace RPI.enable
xwalk-picarx-control --trace RPI.disable
xwalk-picarx-control --trace CTRL.001.enable
xwalk-picarx-control --trace CTRL.disable doctor
xwalk-picarx-control --trace RPIAGENT.enable
xwalk-picarx-control --trace LIB.disable
xwalk-picarx-control --trace all.enable
xwalk-picarx-control --trace all.disable
xwalk-picarx-control --trace xWalk-rpi5/xWalkController/xWalkConfig/xwalk-traces.json
xwalk-picarx-control doctor
xwalk-picarx-control move forward --speed 40 --duration 2.5
xwalk-picarx-control move demo
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
xwalk-picarx-control gpt-car start --keyboard --no-img
xwalk-picarx-control voice-controlled-car start
xwalk-picarx-control voice-prompt-car start
xwalk-picarx-control calibrate
xwalk-picarx-control calibrate grayscale
xwalk-picarx-control calibrate servo-motor
```

Every command that composes the PiCar-X actuator graph owns a scope-bound emergency-stop guard. The guard
attempts both motors independently on normal return and after an escaping backend failure. SIGINT and SIGTERM
only change the application-wide shutdown request; the controlling thread observes it and performs hardware
cleanup. Move, turn, self-drive, line-tracking, and voice-controlled movement share this policy, and bounded
movement waits poll cancellation in slices no longer than 20 milliseconds.

`move demo` delegates to the bounded `xWalkMoveExample` Agent corresponding to upstream `2.move.py`. It
drives forward at speed 30, sweeps steering through the source S-curve, stops the motors, waits one second,
and then sweeps camera pan and tilt through the same source angles. Every wait polls cancellation, and both
Agent and command-scope cleanup stop the motors after normal completion or failure.

`keyboard-control` delegates to `xWalkKeyboardControl`, corresponding to upstream
`3.keyboard_control.py`. Enter one of `w`, `s`, `a`, `d`, `i`, `k`, `j`, or `l`
at each prompt; enter `q` to center steering and camera servos, stop the motors,
and exit. SIGINT and SIGTERM request the same bounded cleanup path.

`avoid-obstacles start` delegates one ultrasonic sample at a time to
`xWalkObstacleAvoidance`, corresponding to upstream `4.avoiding_obstacles.py`.
The foreground loop preserves its 40-centimeter safe boundary, 20-centimeter
danger boundary, steering angles, 50-percent requested power, and bounded turn
delays. Invalid ultrasonic samples stop the loop and motors instead of causing
the source's unsafe reverse action. Use `avoid-obstacles stop` for an immediate
motor stop.

`cliff-detection start` delegates bounded samples to `xWalkCliffDetection`,
corresponding to upstream `5.cliff_detection.py`. Safe samples stop the motors;
danger samples reverse at 80-percent requested power and wait 100 milliseconds
only on a safe-to-danger transition. The Agent uses persisted calibrated cliff
references rather than overwriting them with the Python example's illustrative
value. Use `cliff-detection stop` for an immediate motor stop.

`computer-vision` delegates to the `xWalkComputerVision` port of upstream
`7.computer_vision.py`. Enter `1` through `6` for red through purple detection,
`0` to disable color detection, `f` to toggle faces, `r` to toggle QR decoding,
`s` to report object geometry, or `q` to save a timestamped JPEG. Enter `x` to
exit. The Raspberry Pi boot graph uses a camera-only OpenCV provider and does
not claim the Robot HAT or start Vilib's web server. Configure the camera,
photograph directory, face cascade, and frame size in `picar-x.d/vision.conf`.

`stare-at-you start` delegates face observations to `xWalkFaceTracking` and
retains the correction formula and 35-degree bounds from `8.stare_at_you.py`.

`bull-fight start` delegates red observations to `xWalkBullFight`, updates the
camera and steering, and requests 50-percent forward power while a target is
visible. The normal calibration and maximum-output safety caps still apply.

`treasure-hunt` delegates to `xWalkTreasureHunt`, preserving example 20's six
random targets, greater-than-100-pixel detection threshold, spoken success and
target prompts, 80-percent requested driving, minus/plus 30-degree turns,
half-second movement, space-key repeat, and goodbye cleanup. The Raspberry Pi
graph uses OpenCV and shell-free Pico2Wave `en-US` synthesis; it does not start
Vilib's web preview server.

`record-video` delegates to `xWalkVideoRecording`, preserving example 9's
800-millisecond camera warm-up, `q` transitions, `e` stop operation, and
100-millisecond post-key delay. Enter `x` to close the camera and exit.

`video-car` delegates to `xWalkVideoCar`, preserving example 11's `o`/`p`
speed controls, `w`/`s` direction changes, `a`/`d` steering, `f` stop, and `t`
photo capture. Enter `x` to stop the motors, close the camera, and exit. The
OpenCV provider does not start Vilib's web server.

`video-stream` delegates to `xWalkVideoStreaming`, opens only the configured
camera and loopback HTTP listener, and stops on SIGINT or SIGTERM.

`app-control start` delegates the SunFounder A-Q state to `xWalkAppControl`.
It publishes speed, grayscale, and ultrasonic telemetry; consumes drive and
camera joysticks, voice actions, line tracking, obstacle avoidance, horn, and
red/face detection; and safely stops on cancellation. Configure the listener
address, port, controller identity, and optional separately managed MJPEG URL
in `picar-x.d/connectivity.conf`. The default listener is `127.0.0.1:8765`; LAN exposure must
be explicitly selected. The port does not start Vilib's web server.

`sound-background-music` delegates to `xWalkSoundBackgroundMusic`, preserving
example 13's Space foreground horn, `c` background horn, `q` background-music
toggle, 20-percent music volume, and 50-millisecond post-horn delay. Enter `x`
to stop active music and exit. Sound and music files are resolved only below
the configured resource directories.

Steering is limited to 30 degrees. `picarx_max_motor_output_percent` limits the final calibrated PWM magnitude.
Until `calibrate` records successful raised-wheel motor-direction, motor-balance, and steering-center checks,
the effective limit cannot exceed 20 percent. Raise the configured limit only after those checks. Grayscale
acquisition performs a warm-up, five-sample filtering, poisoned-ADC signature rejection, automatic reference,
line status, and cliff output.

Fresh deployments keep `picarx_apply_persisted_servo_positions = false`. In
that state PiCar-X initialization configures the servo timers and arms the
motors at zero output, but it does not command the stored steering, pan, or tilt
angles. Complete the mechanically unloaded servo calibration first, review the
three stored offsets, and then explicitly change this key to `true`. Shutdown
disarms the motors and does not recenter servos; use an explicit reset only when
servo movement is known to be safe.

The `calibrate` workflow clears prior verification before actuator checks. After servo calibration, it requires
the operator to confirm that all wheels are raised, runs the left motor, right motor, and paired motors through
bounded low-output checks, and records verification only when motor direction, balance, and steering center are
all confirmed. It persists the signed `picarx_motor_speed_calibration` correction. It then runs the automatic
grayscale calibration described below. Cancellation, rejected persistence, or a negative actuator response
leaves the command incomplete and the 20-percent gate active.

`calibrate grayscale` composes the `xWalkGrayscaleCalibration` Agent port of upstream
`1.cali_grayscale.py`. After explicit readiness confirmation, it verifies steering movement, waits for the
source-compatible `q` confirmation, drives the bounded low-power left/right line-calibration pattern, and
derives each line reference from observed minima and maxima. After the operator positions the sensors and
enters `e`, it averages ten stationary cliff samples. It displays both pending arrays and persists them only
after a final `y` response. All waits poll cancellation in slices no longer than 20 milliseconds.

The line phase physically drives forward and backward. Use a clear reviewed calibration surface with room for
the complete pattern; do not run it with the vehicle near a table edge or obstruction.

`calibrate servo-motor` is the controller path corresponding to upstream
`1.cali_servo_motor.py` and composes the `xWalkServoMotorCalibration` Agent. It first resets all servo commands
and optionally runs the source nine-position servo test. Steering, camera-pan, and camera-tilt offsets are
previewed within the source ±20-degree range without persistence. Explicit `1` or `-1` motor directions are
also previewed while the CLI retains its motor-balance correction and bounded raised-wheel left, right,
paired-motor, and steering-center checks. All pending servo and direction values are persisted only after those
checks pass and the operator gives final confirmation. This mode does not modify grayscale references.

`spi transfer` creates only the configured Linux SPI backend and SPI Agent. It
does not detect or reset the Robot HAT and does not claim motors, GPIO, audio,
or camera resources. The payload accepts an optional `0x` prefix and 2 through
512 contiguous hexadecimal digits. Received bytes are printed as uppercase
space-separated hexadecimal text. Configure the device, speed, mode, and word
size in `../xWalkConfig/picar-x.d/hardware.conf` before connecting a peripheral.

`line-track start` runs in the foreground and repeatedly executes the Agent
module's bounded `step()` operation ported from `example/6.line_tracking.py`.
It reports each grayscale sample and classified state, including the final
bounded recovery sample. Press Ctrl+C, send SIGINT, or send SIGTERM to end the
loop; the command then stops both motors and applies the example's final
100-millisecond delay before returning.
`line-track stop` sends an immediate stop through a freshly composed
line-tracking coordinator, applies the final delay, and exits. It does not signal another foreground CLI
process, so use its signal-based cancellation for graceful shutdown.

`voice-chat start` dispatches the foreground `XWalkLocalVoiceChatbot` Agent and
uses SIGINT or SIGTERM for graceful cancellation. `voice-chat stop` requests
shutdown through a freshly composed service; like `line-track stop`, it does
not signal another foreground CLI process. The RPi composition supplies Vosk,
Piper, local Ollama, and speaker-power ownership. It defaults to Piper
`en_US-amy-low`, Ollama `llama3.2:3b`, and a 20-message history to preserve
`example/19.local_voice_chatbot.py`.

The two voice-active-car commands use the common sensor/action coordinator with
Rolly and Jarvis profiles. Each accepts only `start` or `stop`. The
`voice_active_car.py` Rolly profile requires `hey rolly`, answers `Hi there`,
uses image input and OpenAI `gpt-4o-mini`, and reads its credential exclusively
from `OPENAI_API_KEY`. The Jarvis profile requires `hey jarvis`, answers
`Systems online. Ready when you are, Joxy.`, uses Gemini `gemini-3.6-flash` with `GEMINI_API_KEY`, and
speaks through Piper `en_GB-alan-medium`. The RPi graph adds Music, SelfDrive,
the Robot HAT status LED, and still-image capture. Set
`camera_connection` to `csi` for the Raspberry Pi camera connector or `usb` for
a V4L2 webcam. The corresponding `rpicam-still` or `ffmpeg` provider must be installed.

`gpt-car start` delegates to `xWalkGptCar`, corresponding to the upstream
`gpt_examples/gpt_car.py` application. Voice input and image attachment are
enabled by default; `--keyboard` selects typed prompts and `--no-img` disables
camera context. The profile preserves the JSON `actions` and `answer` response,
the complete SelfDrive gesture and sound vocabulary, and `gpt-4o`. The C++
composition uses the OpenAI-compatible chat endpoint instead of persisting an
Assistants identifier, and reads the credential only from `OPENAI_API_KEY`.

The standalone `xWalkVoiceControlledCar` Agent backs `voice-controlled-car`
and preserves example 16's “hey robot” wake loop, repeated one-second movement
commands at 30-percent requested speed, minus/plus 25-degree turns, and “sleep”
session reset. The standalone `xWalkVoicePromptCar` Agent backs
`voice-prompt-car` and preserves example 14's
greeting, four spoken movements, 30-percent requested speed, two-second stages,
and minus/plus 20-degree turns. Both commands accept only `start` or `stop`.
The RPi boot graph connects
Vosk to ALSA microphone capture and connects Espeak PCM to shared ALSA playback.
Configure library, model, voice, PCM, camera, capture, and mixer values in
the `voice.conf` and `vision.conf` fragments below `../xWalkConfig/picar-x.d`.
Missing runtime packages or invalid device names produce
an explicit startup error before the voice Agent begins moving the vehicle.

`storytelling-robot start` delegates to `xWalkStorytellingRobot`. It preserves
example 15's Piper greeting, two three-second forward legs and jokes, farewell,
and six-second backward journey at 30-percent requested speed. Configure
`voice_piper_executable`, `voice_piper_playback_executable`, and
`voice_piper_model`; the default model remains `en_US-amy-low`. The provider
uses shell-free child processes and a private temporary WAV removed after each
synchronous request.

`text-vision-talk start` delegates to `xWalkTextVisionTalk`. It preserves
example 17's welcome, 20-message history, two-second camera warm-up,
1280-by-720 capture, per-prompt `/tmp/llm-img.jpg` replacement, and
case-insensitive `exit` or `quit` termination. The default model is `llava:7b`
on the local Ollama endpoint and is configurable in
`../xWalkConfig/picar-x.d/ai/features.conf`.

`online-llm-test start` delegates to `xWalkOnlineLlmTest`. It preserves example
18's instructions, welcome, 20-message history, and externally cancelled prompt
loop. The Raspberry Pi composition defaults to OpenAI `gpt-4o` and reads the
credential only from `OPENAI_API_KEY`; it never accepts or reports the key
through CLI arguments or committed configuration.

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
xwalk-picarx-control self-drive play-background-music
xwalk-picarx-control self-drive stop-background-music
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
| `play-background-music` | Plays the configured packaged background song |
| `stop-background-music` | Stops background music started by self-drive |

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
`--deployment-config` option for another deployment path.

Use the `hardware_board` value in `../xWalkConfig/picar-x.d/hardware.conf`. Run
`xWalkTool/shell-agent/deploy-tool/provision-hardware.sh --profile <profile> --config <file>` to discover and
record one GPIO device, kernel chip name, and label. Provisioning rejects a v4 selection when the v5 UUID is
present and rejects a v5 selection when that UUID is absent. It never interprets failure to detect v5 as proof
of v4.

The Robot HAT v5 PWM pairs follow the upstream SunFounder test mapping. Confirm the physical left/right ports
and direction polarity with raised wheels before allowing a movement command to run on a new hardware setup.

The command help is maintained as readable lines in `activate/resources/help.json`. CMake generates the private C++
help constant from that file, keeping the JSON configuration and executable output synchronized.

The RPi executable composes a real shared-ALSA Music backend for the standalone `sound` command. Sound-effect
and music operations play synchronously so the one-shot process retains its audio objects until playback
completes. `sound volume 80` deliberately consumes the documented positional volume value. `sound stop`
applies only to playback owned by the same process; it cannot control an earlier CLI process.
Speaker power is enabled through the board-selected GPIO only for `self-drive` and `sound`, and that GPIO
remains claimed until the selected command and its audio composition are destroyed.

The root-level [`xWalkAudioResources`](../../xWalkAudioResources) directory contains the packaged
sound-effect and background-music files used in CLI examples. Run the executable from `xWalkController`
when using the documented relative paths, or provide deployment-specific paths explicitly.
The RPi composition injects the optional libsndfile decoder, which accepts the packaged MP3 as well as the
PCM WAVE sound effects. Decoded audio is bounded to 64 MiB of signed 16-bit PCM per operation. Raspberry Pi
builds therefore require the `libsndfile1-dev` package in addition to the ALSA development package.

## Build and test

```bash
cmake -S xWalkController -B xWalk-rpi5/xWalkController/build-host -DXWALK_CLI_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkController/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkController/build-host --output-on-failure
```

The RPi build compiles the application and hardware-labelled tests. Do not execute them without a safe robot:

```bash
cmake -S xWalkController -B xWalk-rpi5/xWalkController/build-rpi -DXWALK_CLI_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkController/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkController/build-rpi -N -L hardware
```

## Layout

| Path | Responsibility |
| --- | --- |
| `../xWalkHandler/include/` | Controller contract, generated-help selection, callbacks, and types |
| `../xWalkHandler/src/` | Shared controller implementation and functionality-grouped handler sources |
| `../xWalkHandler/test/src/` | Deterministic in-memory command-handler test |
| `activate/include/` | Validated command activation and PiCar-X router declarations |
| `activate/src/` | Top-level dispatch, PiCar-X routing, and generated usage access |
| `activate/resources/help.json` | Build-time source for Linux-style command help |
| `boot/include/` | Application path, callback, boot-mode, and runner declarations |
| `boot/src/` | Process callbacks, boot selection, and service-owned command composition |
| `cli/hardware/src/xControllerMain.cpp` | Raspberry Pi entry point and hardware boot composition |
| `cli/host/src/xControllerHostApplication.cpp` | Host parsing, signals, stub boot, and execution |
| `cli/host/src/xControllerHostMain.cpp` | Thin device-free host entry point |
| `parse/include/xControllerParsing.h` | Typed command-parser and formatter declarations |
| `parse/src/` | Global-option, help, command, request, and output parsing |
| `test/src/xControllerAppTest.cpp` | Isolated GoogleTest coverage of the host application |
| `test/src/xControllerParsingTest.cpp` | Direct GoogleTest coverage of typed parsing and formatting |
| `test/README.md` | Application GoogleTest inventory and host-test commands |
| `CMakeLists.txt` | Executable targets, generated-help processing, and application-test registration |

The `xWalkController` target links the three Agent coordinators so a standalone CLI
consumer receives the same dependency set as the aggregate Agent build.
