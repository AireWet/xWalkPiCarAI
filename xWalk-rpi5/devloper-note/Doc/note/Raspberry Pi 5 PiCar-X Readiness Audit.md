# Raspberry Pi 5 and PiCar-X Readiness Audit

Audit date: 2026-08-11

## Conclusion

**Partially ready — significant features are missing**

The repository has a substantial, safety-conscious C++ implementation of the Robot HAT transport, PiCar-X actuators
and sensors, controller, diagnostics, local speech recognition, speech synthesis, and several language-model
workflows. The complete host build, mocked group tests, staged installation, and generated-file checks pass.
Cppcheck and focused changed-file Clang-Tidy complete without a project diagnostic; the validation record retains
the boundary around prior whole-workspace advisory findings. One line-loss recovery defect that could retain a
reverse motor command is corrected in the current working tree.

This is not evidence that the robot is fully hardware-compatible. No physical PiCar-X has been tested, the ARM64
cross-build is blocked by the absence of a complete reviewed ARM64 dependency sysroot. The configurable GStreamer
path to the standard CSI camera has not been tested on Pi 5. Production object detection remains unsupported. A
bounded, pump-driven non-blocking HTTP/MJPEG listener is socket-tested on x86 loopback, but has not been validated
on Raspberry Pi networking or an external interface. OpenClaw is absent.
Ubuntu 24.04 is not the preferred camera platform. Unsupervised
movement must remain disabled until the board revision, wiring, calibration, actuator directions, stop behaviour,
and physical cut-off have been verified.

## X86 readiness remediation

The following software findings were addressed after the baseline audit. These
results are Linux x86 and mock/recorded-media evidence only.

<table>
<thead>
<tr><th>Area</th><th>Remediation</th><th>Verification boundary</th></tr>
</thead>
<tbody>
<tr>
<td>Servo startup</td>
<td>Construction validates and stores calibration without a PWM write. Explicit, idempotent
<code>initialize()</code> is required before commands and does not select a position.</td>
<td>Mock I2C tests prove only the software call sequence. Servo mechanics remain unverified.</td>
</tr>
<tr>
<td>Servo calibration</td>
<td>Minimum, centre, and maximum angle/pulse calibration is validated independently for steering, pan, and
tilt.</td>
<td>Boundary and invalid-configuration tests pass on x86. Physical limits remain unknown.</td>
</tr>
<tr>
<td>Motor safety</td>
<td>Paired motors start disarmed; movement and active braking require <code>arm()</code>. An injectable-clock
watchdog stops and disarms after command expiry.</td>
<td>Fake-clock tests are deterministic and use no sleep or GPIO hardware.</td>
</tr>
<tr>
<td>Motor failures</td>
<td>Invalid commands do not refresh the watchdog. A channel failure disarms and independently attempts to stop
both motors. Disarming and stopping are idempotent.</td>
<td>Injected callback failures pass on x86; electrical stop behaviour remains unverified.</td>
</tr>
<tr>
<td>Application lifecycle</td>
<td><code>XWalkPicarx::initialize()</code> is the explicit actuator boundary. Emergency stop disarms; clearing
the latch establishes stopped output before rearming.</td>
<td>Application and group tests use mocks.</td>
</tr>
<tr>
<td>Process shutdown</td>
<td><code>SIGINT</code> and <code>SIGTERM</code> handlers remain flag-only and async-signal-safe; ordinary flow
owns cleanup through scope guards and emergency stop.</td>
<td>Host tests cover callbacks. <code>SIGKILL</code>, kernel/power failure, and a complete stall remain outside
software control.</td>
</tr>
<tr>
<td>Camera sources</td>
<td>OpenCV selects V4L2, GStreamer, video file, image sequence, or automatic mode from configuration.
<code>/dev/video0</code> is no longer a class default.</td>
<td>Recorded-video tests cover open, frame, EOF, invalid input, and cleanup. CSI remains unverified.</td>
</tr>
<tr>
<td>Recorded safety scenario</td>
<td>A generated four-frame MJPEG scenario checks the safety pipeline. Twelve additional checksum-verified,
licensed five-frame fixtures cover road-user, visibility, false-positive, interruption, and end-of-video cases with
separate validated annotations.</td>
<td>The detector and classifier are deterministic test implementations. This is not production-model evidence.</td>
</tr>
<tr>
<td>HTTP/MJPEG streaming</td>
<td>Bounded per-client queues, multipart framing, drop-oldest backpressure, camera-loss flushing, non-blocking IPv4
sockets, health/status endpoints, timeouts, client limits, and idempotent lifecycle are implemented.</td>
<td>Real socket tests bind only to x86 loopback. External binding requires explicit authentication configuration;
Raspberry Pi and external-interface validation remain unsupported.</td>
</tr>
<tr>
<td>Language models</td>
<td><code>grok</code>/<code>xai</code> select the OpenAI-compatible dialect with the xAI endpoint and
environment-variable credentials.</td>
<td>Tests use fake credentials and no paid request. OpenClaw remains unsupported.</td>
</tr>
<tr>
<td>ARM64 configuration</td>
<td>A sysroot toolchain rejects missing target roots, restricts discovery to ARM64, confines
<code>pkg-config</code>,
and audits ten required target dependency families plus reported linker inputs.</td>
<td>The host's minimal root is missing all ten audited package families, so no ARM64 compile or link is
claimed.</td>
</tr>
</tbody>
</table>

The process-local motor watchdog is not an independent emergency stop. Before
movement, use raised wheels, a reachable physical power cut-off, conservative
calibration, and preferably an independent hardware watchdog.

### Remediation validation record

- The complete Debug and Release host production/test configurations and 603-step builds succeeded with strict
  warnings; Debug also exported compile commands.
- The sandbox-compatible suite passed 53 of 53. The complete 54-test invocation passed those same 53 and could not
  execute the Gerrit HTTP case because the sandbox rejected INET socket creation. That unchanged test had passed
  separately with loopback permission earlier in the audit; the blocked invocation is not reported as passed.
- All four HAL group tests passed in the separately requested `group-tests` label run.
- Device, sensor, PiCar-X, and line-tracking safety tests each passed ten consecutive runs.
- The recorded-media label passed three of three tests, including the generated end-to-end safety scenario; the
  Robot HAT/PiCar-X simulator tests also pass. None opens a physical camera or Robot HAT device.
- The five-case streaming suite passed 1,000 consecutive repetitions (5,000 case executions), including two
  concurrent publishers, bounded queue saturation, disconnect, and camera-loss paths. This is a bounded host stress
  run, not a multi-hour soak or Raspberry Pi network test.
- The expanded streaming executable passes 12 cases, including real loopback sockets for health, status and MJPEG,
  request/header bounds, header timeout, client limiting, camera loss, port collision, cleanup, and concurrent
  bounded observability production.
- Twelve committed recorded fixtures and twelve separate annotations pass SHA-256, schema, five-frame deterministic
  OpenCV decoding, monotonic-order, malformed-image, and clean-end-of-video checks. These remain detector-double
  safety tests, not production-model accuracy measurements.
- All nine Clang/libFuzzer executables built. Each completed a 200-input ASan/UBSan smoke run with leak detection
  disabled; LeakSanitizer cannot finalize under the sandbox wrapper. The image harness initially exposed an empty-
  buffer precondition error in the harness, which was corrected before its passing 200-input rerun.
- The deterministic logical-model tests and 5,000-iteration soak pass. The fixed-seed run injected 227 faults,
  observed 227 safe stops, retained one thread and six file descriptors, and grew RSS by approximately 70 KiB.
- ASan/UBSan passed all 53 sandbox-compatible tests with `ASAN_OPTIONS=detect_leaks=0`; LeakSanitizer cannot run
  beneath this environment's ptrace wrapper.
- ThreadSanitizer previously built the complete source set, but its focused simulator processes aborted before test
  execution with `unexpected memory mapping`. The new five-step streaming target also built successfully and then
  aborted at process startup with the same runtime mapping error. These environment results are recorded as
  unexecuted, not passed and not code failures.
- The configured Cppcheck warning/performance/portability target completed all 448 translation units with exit
  status zero. Focused Clang-Tidy completed after the empty-cleanup-catch and ignored-`nodiscard` findings were
  corrected; earlier whole-workspace style/analyzer findings remain recorded below.
- Release production build and staged installation succeeded. `clang-format` is installed, but the repository has
  neither a `.clang-format` policy nor a format-check target, so an authoritative formatter result is not claimed;
  `git diff --check` passed.
- A fresh strict native-host Raspberry Pi profile configured and built all 651 steps, including Linux backends, the
  deployable CLI, and hardware-test executables that were compiled but never run. The Release production source set
  also built all 515 steps. Every resulting executable is x86-64; this proves profile compile completeness only.
- The ARM64 cross compiler and ABI check succeeded against a minimal target root. The target-only dependency audit
  then reported Protobuf, gRPC, OpenCV, ALSA, CURL, OpenSSL, TinyXML2, JSON-C, libsndfile, and yaml-cpp missing.
  No ARM64 compile, link, executable, or runtime success is claimed.
- QEMU execution was not run because `qemu-aarch64` is not installed in the x86 environment.

## Evidence boundaries

- Host compilation means x86-64 compilation only.
- Mocked tests establish deterministic software behaviour, not electrical compatibility or timing.
- The AArch64 Vosk library is a genuine ARM64 ELF, but the whole repository has not been linked for ARM64.
- Official SunFounder Python mappings are behavioural references, not proof that this C++ implementation works.
- Every hardware row below remains conditional until the physical kit is inspected and tested.

## Evidence compatibility matrix

| Area | x86 verified | Simulation verified | ARM64 built | QEMU verified | Hardware verified |
| ---- | -----------: | ------------------: | ----------: | ------------: | ----------------: |
| Configuration and no-hardware diagnostics | Yes | Yes | No | No | No |
| Robot HAT transport and register model | Yes | Yes | No | No | No |
| Motor, servo, watchdog, and lifecycle safety logic | Yes | Yes | No | No | No |
| ADC, grayscale, ultrasonic, and battery logic | Yes | Yes | No | No | No |
| Recorded OpenCV camera sources | Yes | Yes | No | No | No |
| CSI/libcamera camera acquisition | No | No | No | No | No |
| Language-model transport protocols | Yes | Yes | No | No | No |
| Production object detector | No | No | No | No | No |
| Production Random Forest risk classifier | No | No | No | No | No |
| In-process MJPEG queue/framing core | Yes | Yes | No | No | No |
| Network video-stream listener | Yes, loopback | Yes | No | No | No |
| Complete Raspberry Pi 5 application | No | No | No | No | No |

`Yes` in the simulation column means deterministic software-double evidence,
not simulated proof of electrical, mechanical, timing, or physical safety.

## Repository architecture

<table>
<thead>
<tr class="header">
<th>Group</th>
<th>Modules and responsibility</th>
<th>Principal dependencies</th>
<th>Hardware or external service</th>
<th>Tests</th>
<th>Assessment</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>Foundation</td>
<td><code>xWalkLibrary</code>, <code>xWalkTrace</code>, <code>xWalkIW</code> provide platform types, tracing, and
generated Protobuf/gRPC contracts</td>
<td>C++17, Protobuf, gRPC, OpenSSL, zlib, c-ares, RE2</td>
<td>None at runtime except IPC/network users</td>
<td>Contract and trace validation, host tests</td>
<td>Host-ready</td>
</tr>
<tr class="even">
<td>HAL interface</td>
<td><code>xWalkI2c</code>, <code>xWalkSpi</code>, <code>xWalkGpio</code>, <code>xWalkAudio</code>,
<code>xWalkConfig</code>, <code>xWalkUtils</code>, <code>xWalkLanguageModel</code></td>
<td>Linux ioctl ABI, ALSA, libcurl, json-c</td>
<td><code>/dev/i2c-*</code>, <code>/dev/spidev*</code>, GPIO character device, ALSA, HTTPS or Ollama</td>
<td>Unit tests and <code>xWalkInterfaceGroupTest</code></td>
<td>Hardware verification required</td>
</tr>
<tr class="odd">
<td>HAL device</td>
<td><code>xWalkPwm</code>, <code>xWalkAdc</code>, <code>xWalkServo</code>, <code>xWalkAdxl345</code>,
<code>xWalkUltrasonic</code>, <code>xWalkCamera</code>, <code>xWalkUserButton</code></td>
<td>Interface group</td>
<td>Robot HAT MCU, GPIO, CSI/USB camera</td>
<td>Unit tests and <code>xWalkDeviceGroupTest</code></td>
<td>Mixed; camera integration is partial</td>
</tr>
<tr class="even">
<td>HAL sensor</td>
<td><code>xWalkLineTracker</code>, <code>xWalkMotor</code>, <code>xWalkLed</code>, <code>xWalkBuzzer</code></td>
<td>Device and interface groups</td>
<td>Grayscale board, motor driver, LEDs and optional buzzer</td>
<td>Unit tests and <code>xWalkSensorGroupTest</code></td>
<td>Core paths implemented; optional devices vary by kit</td>
</tr>
<tr class="odd">
<td>HAL layer 1</td>
<td><code>xWalkMusic</code>, <code>xWalkSpeaker</code>, <code>xWalkBoardControl</code>, <code>xWalkRobot</code>,
<code>xWalkGPT</code>, <code>xWalkVoiceAssistant</code></td>
<td>HAL groups, ALSA, Vosk and TTS executables</td>
<td>Robot HAT, audio, local recognizer</td>
<td>Unit tests and <code>xWalkLayer1GroupTest</code></td>
<td>Hardware verification required</td>
</tr>
<tr class="even">
<td>Vehicle agents</td>
<td><code>xWalkPicarx</code>, movement, keyboard control, avoidance, cliff detection, line tracking,
self-drive</td>
<td>Robot and sensor HAL</td>
<td>Motors, servos, ultrasonic and grayscale sensors</td>
<td>Component and vehicle group tests</td>
<td>Implemented with first-use speed cap</td>
</tr>
<tr class="odd">
<td>Calibration</td>
<td>Grayscale calibration, servo/motor calibration, servo zeroing</td>
<td>Vehicle configuration</td>
<td>All actuators and grayscale board</td>
<td>Component and calibration group tests</td>
<td>Must be run deliberately on hardware</td>
</tr>
<tr class="even">
<td>Vision agents</td>
<td>Capture, HSV colour, Haar face, QR, face tracking, games, AVI recording and video-car</td>
<td><code>rpicam-still</code>, OpenCV/V4L2, ffmpeg</td>
<td>CSI or USB camera</td>
<td>Mocked/component tests</td>
<td>Still capture probable; live CSI vision incomplete</td>
</tr>
<tr class="odd">
<td>Voice and media agents</td>
<td>Local chatbot, prompt/command cars, storytelling, voice-active and GPT cars, sounds/music</td>
<td>ALSA, Vosk, espeak/Piper/Pico, language-model interface</td>
<td>Microphone, speaker, local/cloud model</td>
<td>Mocked/component/group tests</td>
<td>Implemented but device/model setup is external</td>
</tr>
<tr class="even">
<td>Connectivity</td>
<td>App control/WebSocket and bounded SPI transfer</td>
<td>Network stack and SPI HAL</td>
<td>External app/service; optional SPI peripheral</td>
<td>Component/group tests</td>
<td>App protocol partial; no built-in video server</td>
</tr>
<tr class="odd">
<td>Platform/controller</td>
<td>Boot composition, doctor, CLI handlers, layered configuration and systemd deployment</td>
<td>All selected modules</td>
<td>Runtime platform</td>
<td>Controller, sequence and platform tests</td>
<td>Strong deployment foundation</td>
</tr>
</tbody>
</table>

The application call paths are:

``` text
xwalk-picarx-control
    -> Controller handler
    -> vehicle/vision/voice agent
    -> HAL layer 1
    -> device/sensor
    -> I2C/GPIO/ALSA/camera/Linux
    -> hardware or service
```

Examples are `move -> XWalkPicarx -> XWalkRobot -> XWalkMotor -> XWalkPwm -> XWalkI2c -> /dev/i2c-1`,
`line-track -> XWalkLineTracking -> XWalkPicarx + XWalkLineTracker -> XWalkAdc/XWalkMotor`, and
`voice-chat -> XWalkLocalVoiceChatbot -> XWalkVoiceAssistant -> ALSA/Vosk -> XWalkLanguageModel -> TTS/ALSA`.

## Hardware architecture and revision findings

The current code primarily targets Robot HAT V5, while retaining a V4 motor path. The deployed `hardware.conf`
selects the board revision and defaults to `/dev/i2c-1`, `/dev/gpiochip0`, MCU reset `MCURST`, I2C address `0x14`,
servos `P0/P1/P2`, grayscale `A0/A1/A2`, ultrasonic `D2/D3`, battery `A4`, V5 motors `P12/P13` and `P14/P15`, and
V4 motors `P13` plus `D4` and `P12` plus `D5`. The physical Robot HAT marking must decide the profile;
auto-detection is not a substitute for inspecting the board.

The Robot HAT MCU exposes ADC, PWM and named pins at address `0x14`; this repository also probes `0x15` and `0x16`.
PWM uses output base `0x20`, timer prescaler registers `0x40/0x50`, period registers `0x44/0x54`, a 72 MHz clock,
20 channels, 50 Hz servo output, and 100 Hz motor output. The ADC command is `0x10 | (7 - channel)` and returns a
big-endian 12-bit result. These values agree substantially with SunFounder's current protocol, except that the
official current PWM helper stores `period - 1` while this repository stores the requested period directly. That
one-count difference and the 100 Hz motor choice require measurement on the actual board.

Generic servo conversion permits -90 through +90 degrees and 500 through 2500 microseconds. The vehicle layer
narrows steering to +/-30 degrees, pan to +/-90, and tilt to -35 through +65 before calibration offsets. Servo
construction does not write PWM. `XWalkPicarx::initialize()` initializes all three PWM timers, but it commands
persisted calibration angles only when `picarx_apply_persisted_servo_positions` is explicitly enabled; the shipped
safe default is `false`. Assembly and any deliberate servo-position application must still follow the unloaded
servo/zeroing procedure. Do not assume a safe horn position from a zero offset in a file.

Robot HAT V5 documents 6.0--8.4 V battery input and includes onboard microphone/speaker functions. Older V4
hardware does not have the same audio arrangement or dual-PWM motor control. Battery documentation contains
revision-dependent thresholds; use the received board label and manual, confirm the keyed three-pin polarity, and
never improvise the power wiring. The supplied camera is normally an OV5647-class 5 MP CSI module. Raspberry Pi 5
uses 22-pin camera connectors, so a suitable 15-to-22-pin cable/adapter is required unless the received camera
cable already has the correct ends.

Primary official references are the [PiCar-X manual](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/),
[Robot HAT component page](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/hardware/cpn_robot_hat.html),
[Robot HAT V5 hardware
guide](https://docs.sunfounder.com/projects/robot-hat-v4/en/latest/robot_hat_v5/hardware_introduction.html), [Robot
HAT MCU protocol](https://docs.sunfounder.com/projects/robot-hat-v4/en/latest/robot_hat_v5/onboard_mcu.html),
[official PiCar-X V2 Python mapping](https://github.com/sunfounder/picar-x/blob/v2.0/picarx/picarx.py), [Raspberry
Pi camera software](https://www.raspberrypi.com/documentation/computers/camera_software.html), and [Raspberry Pi
camera hardware](https://www.raspberrypi.com/documentation/accessories/camera.html).

## Compatibility matrix

<table>
<thead>
<tr class="header">
<th>Component/feature</th>
<th>Required hardware/interface</th>
<th>Repository implementation</th>
<th>Evidence</th>
<th>Host test</th>
<th>ARM64 build</th>
<th>Hardware test</th>
<th>Status</th>
<th>Risk</th>
<th>Required action</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>Raspberry Pi 5</td>
<td>Pi 5, Pi-compatible Robot HAT</td>
<td>Pi model/board guards and RPI composition</td>
<td><code>xWalkTool/shell/setup-rpi.sh</code>, <code>xWalkBoot</code></td>
<td>Yes, composition mocked</td>
<td>Blocked at Protobuf sysroot</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>High</td>
<td>Build natively and run doctor first</td>
</tr>
<tr class="even">
<td>Raspberry Pi OS 64-bit</td>
<td>Bookworm/Trixie ARM64</td>
<td>Primary deployment path</td>
<td>setup/deployment scripts use <code>rpicam-*</code></td>
<td>Script/static only</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Prefer this OS and verify packages</td>
</tr>
<tr class="odd">
<td>Ubuntu 24.04 ARM64</td>
<td>Pi 5 image</td>
<td>Generic Debian/Ubuntu setup</td>
<td>OS script branches</td>
<td>Script/static only</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>High</td>
<td>CSI support is not assured; use USB camera or newer supported stack</td>
</tr>
<tr class="even">
<td>ARM64 compilation</td>
<td>ARM64 compiler and development sysroot</td>
<td>AArch64 library selection exists</td>
<td>Configure detected GCC 13.3 AArch64 and ARM Vosk</td>
<td>No</td>
<td>Configure blocked</td>
<td>Pending</td>
<td>Cannot determine</td>
<td>High</td>
<td>Install ARM64 Protobuf/gRPC/dependency sysroot or build natively</td>
</tr>
<tr class="odd">
<td>I2C</td>
<td><code>/dev/i2c-1</code>, MCU <code>0x14</code></td>
<td>Linux i2c-dev/SMBus backend, retries and cleanup</td>
<td><code>xWalkI2c</code>, <code>hardware.conf</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Verify bus/address/permissions and combined conversion behaviour on the MCU</td>
</tr>
<tr class="even">
<td>SPI</td>
<td><code>/dev/spidev0.0</code></td>
<td>Linux spidev backend and transfer command</td>
<td><code>xWalkSpi</code>, <code>xWalkSpiTransfer</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Low</td>
<td>No standard PiCar-X SPI peripheral; loopback only if wired</td>
</tr>
<tr class="odd">
<td>GPIO</td>
<td><code>/dev/gpiochip0</code>, named pin mapping</td>
<td>Linux GPIO character-device ioctl backend</td>
<td><code>xWalkGpio</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Confirm RP1 chip identity and line mappings</td>
</tr>
<tr class="even">
<td>PWM controller</td>
<td>Robot HAT MCU, 20 PWM channels</td>
<td>Register-level I2C implementation</td>
<td><code>xWalkPwm</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Scope frequency/period and verify firmware revision</td>
</tr>
<tr class="odd">
<td>ADC controller</td>
<td>Robot HAT MCU ADC</td>
<td>Command/channel and raw conversion implementation</td>
<td><code>xWalkAdc</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Confirm A0--A4 and the atomic conversion protocol on the received MCU firmware</td>
</tr>
<tr class="even">
<td>DC motors</td>
<td>H-bridges and two TT motors</td>
<td>V5 dual PWM and V4 PWM+direction paths</td>
<td><code>xWalkMotor</code>, <code>xWalkPicarx</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Critical</td>
<td>Wheels up; verify ports, inversion, stop and power-loss behaviour</td>
</tr>
<tr class="odd">
<td>Steering servo</td>
<td>P2</td>
<td>50 Hz angle/pulse conversion and calibration</td>
<td><code>xWalkServo</code>, <code>xWalkPicarx</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Critical</td>
<td>Unload horn; restrict angle; centre before assembly</td>
</tr>
<tr class="even">
<td>Camera pan servo</td>
<td>P0</td>
<td>Vehicle-bounded +/-90 degree control</td>
<td><code>xWalkPicarx</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>High</td>
<td>Begin near centre with narrow temporary range</td>
</tr>
<tr class="odd">
<td>Camera tilt servo</td>
<td>P1</td>
<td>Vehicle-bounded -35..65 degree control</td>
<td><code>xWalkPicarx</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>High</td>
<td>Begin near centre with narrow temporary range</td>
</tr>
<tr class="even">
<td>Ultrasonic sensor</td>
<td>Trigger D2/GPIO27, echo D3/GPIO22</td>
<td>10 us pulse, 20 ms wait, retries and invalid result</td>
<td><code>xWalkUltrasonic</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>High</td>
<td>Verify wiring/logic level and stop on invalid readings</td>
</tr>
<tr class="odd">
<td>Grayscale sensors</td>
<td>A0/A1/A2</td>
<td>Raw reads, calibration and interpretation</td>
<td><code>xWalkAdc</code>, <code>xWalkLineTracker</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>High</td>
<td>Calibrate black/white surfaces and persist thresholds</td>
</tr>
<tr class="even">
<td>Line tracking</td>
<td>Grayscale plus drive</td>
<td>Bounded recovery and motor stop; regression added</td>
<td><code>xWalkLineTracking</code></td>
<td>Yes, repeated</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>High</td>
<td>Test wheels-up, line loss, Ctrl+C and sensor failure</td>
</tr>
<tr class="odd">
<td>User button</td>
<td>Robot HAT onboard button</td>
<td>Named-pin/button abstraction</td>
<td><code>xWalkUserButton</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Low</td>
<td>Confirm revision alias and pull behaviour</td>
</tr>
<tr class="even">
<td>LED</td>
<td>Robot HAT status LED; optional external RGB</td>
<td>Onboard and RGB abstractions</td>
<td><code>xWalkLed</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Low</td>
<td>Confirm which LED hardware is actually included</td>
</tr>
<tr class="odd">
<td>Buzzer</td>
<td>Optional external buzzer</td>
<td>Generic buzzer output</td>
<td><code>xWalkBuzzer</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Low</td>
<td>Standard kit may not contain this peripheral</td>
</tr>
<tr class="even">
<td>ADXL345</td>
<td>Optional I2C accelerometer</td>
<td>Complete register abstraction</td>
<td><code>xWalkAdxl345</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Low</td>
<td>Not standard PiCar-X contents; test only if supplied</td>
</tr>
<tr class="odd">
<td>Image capture</td>
<td>CSI camera and <code>rpicam-still</code></td>
<td>Bounded fork/exec still capture</td>
<td><code>xWalkCamera</code>, <code>camera_csi_command</code></td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Verify cable, enumeration and one still first</td>
</tr>
<tr class="even">
<td>Video capture</td>
<td>OpenCV V4L2, GStreamer, file, image sequence, or automatic source</td>
<td>Configurable source and MJPEG AVI recorder</td>
<td><code>xWalkVideoRecordingOpenCv</code>, <code>vision.conf</code></td>
<td>Yes, recorded media</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>High</td>
<td>Use a reviewed rpicam/libcamera GStreamer pipeline and verify CSI shutdown on Pi 5</td>
</tr>
<tr class="odd">
<td>Video streaming</td>
<td>Bounded in-process MJPEG core plus an external listener</td>
<td>Multipart framing and per-client backpressure; URL publication remains separate</td>
<td><code>xWalkVideoStreaming</code>, <code>xWalkAppControl</code>, and <code>app_control_video_url</code></td>
<td>Yes, stream-core simulation</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Medium</td>
<td>Integrate a reviewed non-blocking listener and authentication, or configure a separately managed service</td>
</tr>
<tr class="even">
<td>Camera detection</td>
<td>libcamera/rpicam and V4L2 discovery</td>
<td>Doctor checks both configured paths</td>
<td><code>xWalkDoctorLinux</code></td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Medium</td>
<td>Treat media nodes and V4L2 nodes separately</td>
</tr>
<tr class="odd">
<td>Colour/face/QR vision</td>
<td>OpenCV frame source</td>
<td>HSV, Haar cascade and QR pipelines</td>
<td><code>xWalkComputerVision</code></td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>High</td>
<td>Configure and verify the existing GStreamer source against rpicam/libcamera on Pi 5</td>
</tr>
<tr class="even">
<td>Object detection</td>
<td>Model/runtime/inference</td>
<td>Explicit unsupported event; no model</td>
<td><code>XWalkAppControlEvent::ObjectDetectionUnsupported</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Not supported</td>
<td>High</td>
<td>Select model/runtime and design integration before implementation</td>
</tr>
<tr class="odd">
<td>Video recognition</td>
<td>Frame inference pipeline</td>
<td>No general recognition model</td>
<td>Repository search and app behaviour</td>
<td>No</td>
<td>Pending</td>
<td>Pending</td>
<td>Not supported</td>
<td>High</td>
<td>Same as object detection</td>
</tr>
<tr class="even">
<td>Microphone</td>
<td>V5 onboard or external ALSA capture</td>
<td>ALSA PCM capture and Vosk input</td>
<td><code>xWalkAudio</code>, <code>xWalkSpeechToTextAlsa</code></td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Confirm V5/revision, overlay and <code>arecord -l</code> device</td>
</tr>
<tr class="odd">
<td>Speaker</td>
<td>V5 onboard or external ALSA playback</td>
<td>ALSA playback and mixer control</td>
<td><code>xWalkSpeaker</code>, <code>xWalkMusic</code></td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Confirm codec/overlay and conservative volume</td>
</tr>
<tr class="even">
<td>Music</td>
<td>ALSA plus libsndfile/audio files</td>
<td>Playback, stop and bundled resources</td>
<td><code>xWalkMusic</code>, resources</td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Low</td>
<td>Verify formats and device selection</td>
</tr>
<tr class="odd">
<td>Speech-to-text</td>
<td>Mic, Vosk runtime/model</td>
<td>Local English Vosk pipeline</td>
<td><code>xWalkGPT</code>; ARM64 <code>.so</code> and model bundled</td>
<td>Yes, mocked</td>
<td>Partial artifact only</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Native link/run and test actual noise conditions</td>
</tr>
<tr class="even">
<td>Text-to-speech</td>
<td>espeak-ng/Piper/Pico and ALSA</td>
<td>Process-based local synthesis/playback</td>
<td><code>xWalkTextToSpeechAlsa</code></td>
<td>Yes, mocked</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Install selected engine/model; verify stop/timeout</td>
</tr>
<tr class="odd">
<td>Voice assistant</td>
<td>STT, LLM and TTS</td>
<td>Connected synchronous workflow</td>
<td><code>xWalkVoiceAssistant</code>, voice agents</td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>High</td>
<td>Test each stage and failure mode before enabling motion actions</td>
</tr>
<tr class="even">
<td>ChatGPT/OpenAI</td>
<td>Network, endpoint, API key env</td>
<td>OpenAI-compatible <code>/v1/chat/completions</code></td>
<td>provider config and <code>xWalkLanguageModel</code></td>
<td>Mock HTTP tests</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Set key only in environment; verify rate/error handling</td>
</tr>
<tr class="odd">
<td>Gemini</td>
<td>Network and key</td>
<td>OpenAI-compatible Gemini endpoint configuration</td>
<td>provider config</td>
<td>Mock HTTP tests</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Medium</td>
<td>Verify current endpoint/model against account</td>
</tr>
<tr class="even">
<td>Grok</td>
<td>Network and xAI-compatible endpoint</td>
<td>OpenAI-compatible xAI provider profile and example</td>
<td><code>ai/providers/grok.conf</code>, <code>XWalkGrokExampleLinux</code></td>
<td>Serialization/configuration mock only</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Medium</td>
<td>Set key/model through environment; verify the current xAI endpoint against the account</td>
</tr>
<tr class="odd">
<td>OpenClaw</td>
<td>Official Python stack and skill/service</td>
<td>No repository implementation</td>
<td>No source/config references</td>
<td>No</td>
<td>No</td>
<td>No</td>
<td>Not supported</td>
<td>Low</td>
<td>Use official separate software or propose an adapter</td>
</tr>
<tr class="even">
<td>Ollama/local LLM</td>
<td>Ollama HTTP service and local model</td>
<td><code>/api/chat</code> backend</td>
<td>provider config and language-model transport</td>
<td>Mock HTTP tests</td>
<td>Pending</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Install Ollama/model separately; benchmark Pi 5 memory/latency</td>
</tr>
<tr class="odd">
<td>Configuration</td>
<td>Installed layered <code>.conf</code> files</td>
<td>Validated typed access and conservative defaults</td>
<td><code>xWalkConfig</code>, deployment config</td>
<td>Yes</td>
<td>Independent</td>
<td>Pending</td>
<td>Probably ready — hardware verification required</td>
<td>Medium</td>
<td>Copy examples and record received-board values</td>
</tr>
<tr class="even">
<td>Logging/tracing</td>
<td>Writable state/log path</td>
<td>393 validated trace IDs and XML/JSON control</td>
<td><code>xWalkTrace</code></td>
<td>Yes</td>
<td>Independent</td>
<td>Pending</td>
<td>Ready</td>
<td>Low</td>
<td>Choose log level; protect credential-bearing prompts</td>
</tr>
<tr class="odd">
<td>Safe initialization</td>
<td>Correct calibration and disconnected load</td>
<td>Motor arming first establishes zero; persisted servo positions are disabled by default; 20% unverified cap</td>
<td>lifecycle code and <code>vehicle.conf</code></td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Critical</td>
<td>Do not run a movement-capable command until unloaded servo calibration is safe</td>
</tr>
<tr class="even">
<td>Emergency stop</td>
<td>Cooperative process and physical power cut</td>
<td>Atomic software latch, stop callbacks and signals</td>
<td><code>xWalkPicarx</code>, controller runner</td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Critical</td>
<td>Process-local 500 ms watchdog is proven in simulation; no crash/SIGKILL-independent watchdog exists</td>
</tr>
<tr class="odd">
<td>Safe shutdown</td>
<td>Timely destructor/signal path</td>
<td>Normal and SIGINT/SIGTERM stop paths</td>
<td>motor/robot/controller lifecycles</td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>Critical</td>
<td>Verify Ctrl+C; keep physical cut-off reachable</td>
</tr>
<tr class="even">
<td>Hardware-failure handling</td>
<td>Timeouts and fail-safe consumers</td>
<td>Bounded ultrasonic/camera/HTTP; I2C retries; mixed propagation</td>
<td>HAL and agent tests</td>
<td>Yes</td>
<td>Pending</td>
<td>Pending</td>
<td>Partially supported</td>
<td>High</td>
<td>Validate bus loss and actuator state; consider an independent hardware watchdog</td>
</tr>
<tr class="odd">
<td>Scratch</td>
<td>Scratch runtime/adapter</td>
<td>None</td>
<td>Repository-wide search</td>
<td>No</td>
<td>No</td>
<td>No</td>
<td>Not supported</td>
<td>Low</td>
<td>Use official SunFounder software separately</td>
</tr>
</tbody>
</table>

## Principal findings and required changes

### Critical

1.  `xWalkPicarx` emergency stop is process-local and cooperative. There is no demonstrated Robot HAT motor
    watchdog that turns outputs off after a crash, `SIGKILL`, stalled syscall, or host power failure. Destructors
    and the signal path are good but cannot cover those cases. Keep the wheels raised and a physical battery
    cut-off within reach. A hardware or supervised heartbeat/watchdog design is required before unattended driving.
    This is a design proposal, not an authorized change.
2.  `XWalkPicarx::initialize()` can move P0/P1/P2 to stored zero/offset values when
    `picarx_apply_persisted_servo_positions` is explicitly enabled. Construction is non-actuating and the shipped
    default keeps persisted-position application disabled, but incorrect stored calibration or an assembled horn
    can strike mechanical limits if that gate is later enabled. Perform initial zeroing unloaded, with motor power
    disconnected where the board permits, and inspect every value before enabling it.
3.  Persistent line loss after a left/right state could previously retain reverse output because recovery returned
    for any state differing from the prior state, including `Stop`. `XWalkLineTracking::recoverLine()` now returns
    only for a non-Stop reading, and its test proves the bounded timeout stops both motors.

### High

1.  Computer vision and recording support explicit V4L2, GStreamer, video-file, image-sequence, and automatic
    sources. The shipped live-source default remains V4L2 `/dev/video0`, while the standard CSI camera uses
    libcamera/rpicam and is not guaranteed to expose that node. The existing GStreamer route must be configured and
    tested with a reviewed rpicam/libcamera pipeline before CSI live vision can be claimed.
2.  The AArch64 toolchain rejected the installed incomplete target root before compiler use because it lacks the
    reviewed `usr/include`/`usr/lib` sysroot layout and target dependencies such as Protobuf and gRPC. Native Pi
    build or a complete ARM64 sysroot is mandatory.
3.  `XWalkVideoRecording` joins its capture worker without a repository-controlled timeout around a blocked OpenCV
    `read()`. Verify camera disconnect behaviour before relying on prompt shutdown.
4.  Language-model transport is bounded and validates HTTPS for cloud endpoints, but no retry/backoff or rate-limit
    policy was found. A cloud outage can delay voice-driven stop logic; never make network completion part of the
    only stop mechanism.
5.  The CLI has no actuator-specific steering diagnostic and no operator command that directly latches emergency
    stop. Servo offset persistence is coupled to raised-wheel motor verification, and automatic grayscale
    calibration includes motor and steering motion. Use the non-actuating/manual procedure below for initial
    values; add narrowly scoped diagnostics in a future authorized change.

### Medium and low

- V5 motor PWM is 100 Hz in this repository; SunFounder examples use a different timer configuration. Measure
  torque, noise and low-speed control before changing it.
- PWM period storage differs by one count from the current official helper. Measure before changing a protocol that
  otherwise has strong mock coverage.
- `XWalkI2c::writeRegisterThenRead()` holds one mutex across each ADC request and response; concurrent A0/A1 tests
  prove software ordering, while physical MCU timing and bus-failure behaviour remain unverified.
- `/dev/i2c-1`, `/dev/gpiochip0`, the V4L2 `/dev/video0` source, ALSA `default`, channel inversion and centres are
  configurable defaults, not discovered facts.
- External RGB LED, buzzer and ADXL345 modules are implemented but are not guaranteed standard kit contents.
- The configuration doctor is useful but cannot prove actuator wiring or a safe mechanical range.

Before first power-on, confirm board revision, battery chemistry/voltage/polarity, all cable orientations,
motor-power isolation, unloaded servo state, camera cable, and a reachable physical power cut-off. No additional
software change is known to be mandatory before a non-actuating boot and `doctor` run.

Before first movement, verify actuator mappings and inversion, calibrate all three servos and grayscale thresholds,
prove normal stop and Ctrl+C with wheels raised, deliberately simulate sensor/I2C failure, and accept that the
software emergency stop is not an independent hardware watchdog.

## AI, voice and video scope

“AI Robot” is not a hardware guarantee. This repository provides local Vosk speech recognition, local TTS adapters,
OpenAI-compatible and Ollama chat transports, and rule/action coordinators. It does not contain YOLO, ONNX,
TensorFlow Lite or a general object-detection model. Its visual processing is classical OpenCV colour segmentation,
Haar face detection and QR decoding. No accelerator integration exists. Streaming is an external URL contract, not
a server.

OpenAI/ChatGPT has the most direct provider path. Gemini is configured through its OpenAI-compatible endpoint. Grok
has a first-class xAI OpenAI-compatible provider profile and example, but only mocked serialization/configuration
has been exercised; no paid cloud request was made. Ollama is implemented through local HTTP and requires a
separately installed server/model.
OpenClaw and Scratch have no integration here; SunFounder's advertised OpenClaw feature belongs to its Python
`picarx`, `robot_hat`, `vilib` and skill ecosystem. All cloud keys must remain environment variables or encrypted
private configuration; none belongs in the repository.

## Feature support summary

### Implemented and host-validated

Configuration, tracing, I2C/SPI/GPIO abstractions, PWM/ADC/servo/motor logic, ultrasonic and grayscale reads,
calibrated vehicle control, line/cliff/obstacle agents, still-camera process control, ALSA ownership,
music/speaker, Vosk STT, local TTS adapters, language-model HTTP transport, voice coordination, controller parsing,
doctor,
deployment scripts, staged installation, and the four HAL group tests are implemented and mocked on the host.
Hardware-dependent entries remain “probably ready”, not “ready”.

### Partial or disconnected

Live vision and AVI recording support V4L2, GStreamer, finite media and automatic OpenCV sources, but the CSI
GStreamer pipeline remains unverified on Pi 5. The bounded MJPEG core frames and queues data but does not listen on
a socket; app-control video still publishes an external URL. Voice workflows are connected but depend on unverified ALSA,
TTS and model setup. Gemini and Grok depend on compatibility endpoints rather than dedicated SDKs. ADXL345, RGB
LED, buzzer and SPI support may have no matching standard kit device. Several Agent implementations are callable
from the CLI, but their safe physical behaviour is not demonstrated.

### Unsupported

General object detection, video recognition, a network video-stream listener, OpenClaw, Scratch, hardware
acceleration, and an independent crash-safe motor watchdog are absent. There is no included
YOLO/ONNX/TFLite model or inference runtime.

## Recommended diagnostic order

Use `doctor`, Linux node discovery, `i2cdetect`, `gpioinfo`, repeated grayscale reads, manual threshold
calibration, restricted servo previews, isolated low-output motor pulses, normal/TERM/Ctrl+C stops, ultrasonic
reads, optional button/LED/buzzer checks, `rpicam-*` still/video capture, ALSA record/play, local STT, local TTS,
one non-actuating LLM round, the complete voice pipeline, wheels-up line tracking, and only then controlled floor
movement. The detailed commands and recovery actions are in the 36-step table below.

## Raspberry Pi deployment

Raspberry Pi OS 64-bit is recommended for the first hardware session because current `rpicam-*` support and OV5647
documentation are clear. Ubuntu 24.04 supports Raspberry Pi 5 generally, but Canonical documents broad CSI camera
support only in newer releases; treat Ubuntu 24.04 camera use as unsupported until proven or use a compatible USB
camera. Canonical's Ubuntu 24.04 release notes record broken libcamera support, while its current Raspberry Pi
camera guide states broad CSI-stack support from Ubuntu 25.04:

<https://documentation.ubuntu.com/release-notes/24.04/>

<https://documentation.ubuntu.com/hardware-support/boards/how-to/special_hardware/rpi-camera/>

The setup script supports both OS families for non-camera dependencies.

Inspect the script before use, then run it with the observed board revision and user:

``` bash
./xWalkTool/shell/setup-rpi.sh --help
```

``` bash
sudo ./xWalkTool/shell/setup-rpi.sh --board v5 --runtime-user "$USER" --gpio-device /dev/gpiochip0
```

The script installs the compiler, CMake, Ninja, Git-related build tooling, Protobuf/gRPC dependencies, I2C tools
and headers, `gpiod`, SPI/GPIO Linux headers, ALSA, libsndfile, OpenCV, curl/OpenSSL, json-c, YAML, Boost,
espeak-ng, Pico TTS, ffmpeg and `rpicam-apps` where the OS provides them. It enables I2C and SPI, creates narrowly
scoped device groups/udev rules, and adds the runtime user to available `audio`, `video`, and `render` groups. Log
out and back in after group changes. Do not install a Device Tree overlay unless the dependency tool verifies the
exact V5 identity; the shell setup intentionally does not alter overlays.

The current Debian-family package set in the script is `build-essential`, `cmake`, `ninja-build`, `pkg-config`,
`python3`, `python3-nacl`, `linux-libc-dev`, `libasound2-dev`, `alsa-utils`, `libatomic1`,
`libcurl4-openssl-dev`, `libjson-c-dev`, `libsndfile1-dev`, `libprotobuf-dev`, `libgrpc++-dev`, `libgtest-dev`,
`libtinyxml2-dev`, `libyaml-cpp-dev`, `libopencv-dev`, `libboost-dev`, `i2c-tools`, `libi2c-dev`, `gpiod`,
`espeak-ng`, `libttspico-utils`, `curl`, `ca-certificates`, plus `rpicam-apps` or `ffmpeg` according to the OS.
Transitive packages supply OpenSSL, zlib, c-ares and RE2 on the supported Debian-family images. Piper, its voice
model, Ollama and local LLM weights are external and must be installed deliberately, not through an unverified
pipe.

Verify the platform without actuating anything:

``` bash
uname -a && dpkg --print-architecture && cat /proc/device-tree/model
```

``` bash
ls -l /dev/i2c-* /dev/gpiochip* /dev/spidev* 2>/dev/null
```

``` bash
i2cdetect -y 1
```

``` bash
gpioinfo /dev/gpiochip0
```

``` bash
arecord -l && aplay -l
```

``` bash
rpicam-hello --list-cameras
```

Configure, build and run host-compatible tests natively on the Pi:

``` bash
cmake --fresh -S . -B build-rpi/release -G Ninja -DBUILD_TESTING=ON -DXWALK_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Release
```

``` bash
cmake --build build-rpi/release --parallel
```

``` bash
ctest --test-dir build-rpi/release -N -L hardware
```

``` bash
ctest --test-dir build-rpi/release -LE hardware --output-on-failure
```

The `-N` command lists hardware tests only; do not execute them as a batch. Install to a staging area first,
inspect it, then perform the actual install if desired:

``` bash
DESTDIR="$PWD/build-rpi/stage" cmake --install build-rpi/release --prefix /usr
```

``` bash
sudo cmake --install build-rpi/release --prefix /usr
```

Run only diagnostics at first:

``` bash
/usr/bin/xwalk-picarx-control doctor
```

The packaged systemd unit defaults to `XWALK_COMMAND=doctor`; do not change it to a movement command before the
individual checks below pass.

## Configuration template

Copy the installed examples and retain conservative values. The exact key spelling is defined by the shipped
`hardware.conf`, `vehicle.conf`, `vision.conf`, `voice.conf`, and provider files.

``` ini
# hardware.conf
hardware_board = auto
hardware_i2c_device = /dev/i2c-1
hardware_gpio_device = /dev/gpiochip0
hardware_direction_pwm_channel = P2
hardware_pan_pwm_channel = P0
hardware_tilt_pwm_channel = P1
hardware_grayscale_left_channel = A0
hardware_grayscale_middle_channel = A1
hardware_grayscale_right_channel = A2
hardware_ultrasonic_trigger_pin = D2
hardware_ultrasonic_echo_pin = D3
```

``` ini
# vehicle.conf: first-use values
picarx_dir_servo = 0
picarx_cam_pan_servo = 0
picarx_cam_tilt_servo = 0
picarx_dir_motor = [1, 1]
picarx_motor_speed_calibration = 0
picarx_max_motor_output_percent = 20
picarx_calibration_verified = false
line_reference = [1000, 1000, 1000]
cliff_reference = [500, 500, 500]
```

``` ini
# vision/voice/provider examples
camera_connection = csi
camera_csi_executable = rpicam-still
camera_csi_device = /dev/media0
computer_vision_camera_backend = video_file
computer_vision_camera_device = /absolute/path/to/reviewed-test-video.avi
computer_vision_read_timeout_ms = 1000
video_recording_camera_backend = image_sequence
video_recording_camera_device = /absolute/path/to/frames/frame-%03d.png
video_recording_read_timeout_ms = 1000
voice_capture_device = default
voice_playback_device = default
voice_language_model_provider = ollama
voice_language_model_endpoint = http://127.0.0.1:11434/api/chat
voice_language_model_model_environment = OLLAMA_MODEL
voice_language_model_api_key_environment =
voice_language_model_timeout_ms = 30000
```

Do not mark calibration verified until the wheels-up procedure succeeds. Provider credentials use the named
environment variable in the provider configuration; never place a key in these files. MCU addresses (`0x14`,
`0x15`, `0x16` probes), servo mechanical limits, motor channel pairs, ADC conversion rules, ultrasonic timeout, and
trace level are not all exposed as general deployment keys. Motor/PWM pins are present in `hardware.conf`;
servo/ultrasonic safety limits remain typed code defaults, and tracing is selected with `--trace`. Treat the absent
knobs as configuration gaps, not as permission to add undocumented keys.

## Safe first-hardware test plan

Before step 1, photograph and record the exact kit/Robot HAT revisions; read the matching manual; verify battery
type, 6.0--8.4 V requirement where applicable, keyed polarity, switch/jumpers, motor and sensor ports, servo
labels, and all cable orientations. Connect the Pi 5 camera only while powered off and confirm the 22-pin adapter.
Raise the chassis so all wheels are clear. Disconnect motor leads for the first boot. Leave servo horns unloaded
where practical. Keep a physical battery disconnect within reach. Never connect external GPIO signals above the
documented logic voltage.

<table>
<thead>
<tr class="header">
<th>Step</th>
<th>Command/executable</th>
<th>Expected result</th>
<th>Pass condition</th>
<th>Failure condition</th>
<th>Safe recovery</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>1</td>
<td>Boot with robot application disabled</td>
<td>OS login, no actuator motion</td>
<td>Stable boot; motors disconnected</td>
<td>Servo motion, heat, smell, undervoltage</td>
<td>Cut battery immediately; inspect wiring</td>
</tr>
<tr class="even">
<td>2</td>
<td><code>uname -a; dpkg --print-architecture; cat /proc/device-tree/model</code></td>
<td>Pi 5 and <code>arm64</code></td>
<td>Exact target identified</td>
<td>Wrong model/architecture</td>
<td>Stop and install correct 64-bit image</td>
</tr>
<tr class="odd">
<td>3</td>
<td>Configure/build commands above</td>
<td>Native build completes</td>
<td>All production/test targets link</td>
<td>Compile/link failure</td>
<td>Keep actuators disconnected; fix dependencies</td>
</tr>
<tr class="even">
<td>4</td>
<td><code>ls -l /dev/i2c-*</code></td>
<td><code>/dev/i2c-1</code> visible</td>
<td>User has intended access</td>
<td>Missing/wrong bus</td>
<td>Recheck I2C enablement and reboot</td>
</tr>
<tr class="odd">
<td>5</td>
<td><code>i2cdetect -y 1</code></td>
<td>Normally <code>14</code> appears</td>
<td>Expected address only and stable</td>
<td>No device, conflict, unstable bus</td>
<td>Power off; inspect board/cable/revision</td>
</tr>
<tr class="even">
<td>6</td>
<td><code>xwalk-picarx-control doctor</code></td>
<td>Read-only report</td>
<td>Board, nodes and config agree</td>
<td>Any red/mismatched diagnostic</td>
<td>Do not start actuators; correct config</td>
</tr>
<tr class="odd">
<td>7</td>
<td><code>gpioinfo /dev/gpiochip0</code></td>
<td>RP1 lines enumerate</td>
<td>Chip identity and access match config</td>
<td>Wrong chip or permission</td>
<td>Correct device/udev; do not toggle outputs</td>
</tr>
<tr class="even">
<td>8</td>
<td><code>xwalk-picarx-control sensor grayscale</code> with inputs disconnected only if manual permits</td>
<td>Bounded ADC readings</td>
<td>A0/A1/A2 values are plausible</td>
<td>Timeout, stuck or cross-channel values</td>
<td>Stop; inspect ADC mapping and atomic access</td>
</tr>
<tr class="odd">
<td>9</td>
<td>Same command with grayscale board connected</td>
<td>Three changing values</td>
<td>Each sensor responds to shading</td>
<td>Fixed/swapped/noisy values</td>
<td>Power off and inspect cable/ports</td>
</tr>
<tr class="even">
<td>10</td>
<td>Repeat <code>sensor grayscale</code> over manually presented white/black surfaces; set
<code>line_reference</code> and <code>cliff_reference</code></td>
<td>Non-actuating white/black references stored</td>
<td>Clear separation and repeatability</td>
<td>Overlap or unstable thresholds</td>
<td>Do not line-track; improve surface/lighting</td>
</tr>
<tr class="odd">
<td>11</td>
<td><code>xwalk-picarx-control calibrate servo-motor</code>; skip the sweep, preview steering within +/-5,
then abort before raised-wheel motor verification</td>
<td>Small restricted servo motion</td>
<td>Correct servo moves without binding</td>
<td>Wrong servo, chatter, hard stop</td>
<td>Cut power; correct channel/horn/offset</td>
</tr>
<tr class="even">
<td>12</td>
<td>Record the previewed <code>picarx_dir_servo</code> offset, update <code>vehicle.conf</code>, and repeat the
skip-only calibration preview</td>
<td>Steering centre configured</td>
<td>Wheels mechanically centred</td>
<td>Binding or excessive travel</td>
<td>Cut power and remove/reposition horn</td>
</tr>
<tr class="odd">
<td>13</td>
<td><code>xwalk-picarx-control cam pan --angle 5</code> then <code>--angle -5</code></td>
<td>Small pan motion</td>
<td>Correct direction and free travel</td>
<td>Wrong channel/direction/binding</td>
<td>Stop and correct P0/offset</td>
</tr>
<tr class="even">
<td>14</td>
<td><code>xwalk-picarx-control cam tilt --angle 5</code> then <code>--angle -5</code></td>
<td>Small tilt motion</td>
<td>Correct direction and free travel</td>
<td>Wrong channel/direction/binding</td>
<td>Stop and correct P1/offset</td>
</tr>
<tr class="odd">
<td>15</td>
<td>Reconnect one motor; <code>xwalk-picarx-control move forward --speed 5 --duration 0.2</code></td>
<td>One low-speed wheel pulse</td>
<td>Intended wheel/direction then zero</td>
<td>Wrong port/direction or continued spin</td>
<td>Physical cut-off; correct mapping/inversion</td>
</tr>
<tr class="even">
<td>16</td>
<td>Observe and mark wheel direction</td>
<td>Forward agrees with chassis</td>
<td>Direction documented</td>
<td>Reverse/ambiguous</td>
<td>Change inversion only after power off</td>
</tr>
<tr class="odd">
<td>17</td>
<td>Repeat step 15 for the second motor alone</td>
<td>Second wheel pulse</td>
<td>Intended wheel/direction then zero</td>
<td>Wrong channel or continued spin</td>
<td>Physical cut-off; correct mapping</td>
</tr>
<tr class="even">
<td>18</td>
<td>Repeat with both motors, then issue stop/allow bounded command to end</td>
<td>Both PWM outputs reach zero</td>
<td>Audible/visible complete stop</td>
<td>Coasting output remains commanded</td>
<td>Cut power; inspect stop semantics</td>
</tr>
<tr class="odd">
<td>19</td>
<td>Start wheels-up <code>line-track start</code>; from another terminal run <code>kill -TERM PID</code></td>
<td>Immediate stop and command rejection</td>
<td>Both wheels stop and latch holds</td>
<td>Movement continues/restarts</td>
<td>Cut battery; do not drive on floor</td>
</tr>
<tr class="even">
<td>20</td>
<td>Start a low bounded command and press <code>Ctrl+C</code></td>
<td>Signal handler stops outputs</td>
<td>Both stop before process exits</td>
<td>Hang or retained output</td>
<td>Cut battery; diagnose shutdown path</td>
</tr>
<tr class="odd">
<td>21</td>
<td><code>xwalk-picarx-control sensor distance</code> at known targets</td>
<td>Plausible centimetres, bounded timeout</td>
<td>Multiple readings track distance</td>
<td>Timeout, impossible or stuck values</td>
<td>Do not run avoidance; inspect D2/D3</td>
</tr>
<tr class="even">
<td>22</td>
<td>Test button/LED/buzzer individually with applicable hardware diagnostic</td>
<td>Only selected device changes</td>
<td>Correct onboard/optional device</td>
<td>Unexpected actuator or absent option</td>
<td>Stop; confirm kit contents and revision</td>
</tr>
<tr class="odd">
<td>23</td>
<td><code>rpicam-hello --list-cameras</code></td>
<td>OV5647 camera enumerates</td>
<td>Correct sensor and modes listed</td>
<td>No camera</td>
<td>Power off; reseat correct 22-pin cable</td>
</tr>
<tr class="even">
<td>24</td>
<td><code>rpicam-still -o /tmp/picarx-first.jpg --width 640 --height 480</code></td>
<td>One image</td>
<td>Valid, correctly oriented image</td>
<td>Command error/corrupt image</td>
<td>Fix camera stack before repository vision</td>
</tr>
<tr class="odd">
<td>25</td>
<td><code>rpicam-vid -t 3000 -o /tmp/picarx-first.h264</code></td>
<td>Three-second video</td>
<td>Decodable stable video</td>
<td>Dropout/timeout</td>
<td>Check supply/cable/camera software</td>
</tr>
<tr class="even">
<td>26</td>
<td>Start the separately configured MJPEG service, then <code>app-control start</code></td>
<td>URL is reachable</td>
<td>Client displays current frames</td>
<td>Empty/dead URL</td>
<td>Stop app control; configure external stream</td>
</tr>
<tr class="odd">
<td>27</td>
<td><code>xwalk-picarx-control computer-vision</code></td>
<td>HSV/face/QR menu receives frames</td>
<td>Configured source works</td>
<td>Standard CSI absent at <code>/dev/video0</code></td>
<td>Use compatible bridge/backend; do not claim support</td>
</tr>
<tr class="even">
<td>28</td>
<td><code>arecord -l; aplay -l</code></td>
<td>Capture/playback devices listed</td>
<td>Intended codec/device identified</td>
<td>No matching device</td>
<td>Verify V5 board and approved overlay</td>
</tr>
<tr class="odd">
<td>29</td>
<td><code>arecord -D default -f S16_LE -r 16000 -c 1 -d 3 /tmp/picarx.wav &amp;&amp;
aplay -D default /tmp/picarx.wav</code></td>
<td>Record/play short sample</td>
<td>Intelligible, no severe clipping</td>
<td>Silence/noise/device error</td>
<td>Lower volume; select explicit ALSA device</td>
</tr>
<tr class="even">
<td>30</td>
<td><code>xwalk-picarx-control voice-chat start</code> with local model and no motion actions</td>
<td>Speech becomes text</td>
<td>Stable recognition in quiet room</td>
<td>Timeout/wrong text/device failure</td>
<td>Stop; test ALSA and Vosk separately</td>
</tr>
<tr class="odd">
<td>31</td>
<td><code>xwalk-picarx-control sound play sounds/car-double-horn.wav --volume 20</code> and selected TTS
workflow</td>
<td>Quiet output</td>
<td>Clear output and bounded stop</td>
<td>Distortion or no output</td>
<td>Stop; inspect mixer/device selection</td>
</tr>
<tr class="even">
<td>32</td>
<td><code>curl http://127.0.0.1:11434/api/tags</code> then a non-actuating <code>online-llm-test</code>
profile</td>
<td>One configured backend replies</td>
<td>Bounded valid response, no secret logged</td>
<td>Timeout/auth/rate error</td>
<td>Stop workflow; fix endpoint off-robot</td>
</tr>
<tr class="odd">
<td>33</td>
<td><code>xwalk-picarx-control voice-chat start</code></td>
<td>STT -&gt; LLM -&gt; TTS completes</td>
<td>One safe conversational round</td>
<td>Any stage hangs/fails</td>
<td>Cancel; isolate stage; no movement agent</td>
</tr>
<tr class="even">
<td>34</td>
<td><code>xwalk-picarx-control line-track start</code> with wheels raised</td>
<td>Corrections and stop on line loss</td>
<td>Correct left/centre/right response and bounded loss</td>
<td>Wrong direction or continued spin</td>
<td>Ctrl+C then physical cut-off; recalibrate</td>
</tr>
<tr class="odd">
<td>35</td>
<td>Low bounded <code>move</code> in a clear controlled area</td>
<td>Slow straight motion</td>
<td>Direction, steering and stop all correct</td>
<td>Pull, collision tendency, delayed stop</td>
<td>Physical cut-off; return wheels-up</td>
</tr>
<tr class="even">
<td>36</td>
<td>Selected full application at speed cap 20 or less</td>
<td>Integrated behaviour</td>
<td>All prior safeguards remain effective</td>
<td>Unexpected action or blocked stop</td>
<td>Physical cut-off; disable service and diagnose</td>
</tr>
</tbody>
</table>

Object detection/video recognition cannot pass step 27 without a new implementation. Do not treat a colour or Haar
face result as general object recognition. Do not run a movement-capable voice assistant until step 35 passes.

## Validation record

The audit host was Ubuntu 24.04.4 x86-64 with GCC 13.3, Clang 18.1.3, CMake 3.28.3 and Ninja 1.11.1.

The 2026-08-11 host-quality extension adds separate ASan/UBSan, leak-enabled
LSan, TSan, GCC and Clang coverage, Valgrind, Clang Static Analyzer, and
ShellCheck workflows. The measured GCC production-code baseline is 80.7
percent lines, 85.2 percent functions, and 66.6 percent branches. LSan reports
`BLOCKED_BY_ENVIRONMENT` because the audit process runs under a ptrace wrapper.
The intentional Clang TSan race probe and four focused project tests pass when
loopback access is granted. The LSan result is not recorded as passed. The exact
native-host reproduction commands and result terminology are in
`xWalkTool/quality/README.md`.

- Clean Debug configuration with tests, strict warnings and compile commands: successful.
- Complete clean strict Debug and Release host builds: 597 build steps successful in each profile.
- Staged install/export/resource layout: successful; installed binary dependency resolution and `--help`
  successful.
- Generated Protobuf/XML and 393 trace IDs: validation successful.
- Configured `cppcheck`: 444 translation units completed with exit status zero after documented generated-code,
  GoogleTest-parser, and callback-address false positives were narrowly suppressed. The optional
  `cppcheck-full` style/inconclusive diagnostics remain advisory rather than a zero-finding claim.
- The previously reported Clang-Tidy SPI pointer-conversion and GoogleTest ownership findings were reviewed. The
  SPI host stub now carries a narrow Linux-ABI conversion justification, and the central runner documents the
  GoogleTest factory ownership transfer. Focused Clang-Tidy completed with no project diagnostics.
- No repository format-check target or `.clang-format` policy is present. The host does have `clang-format`, so its
  presence is not presented as a project formatting result. `cmake-format` and `markdownlint` were unavailable;
  ShellCheck and `git diff --check` passed.
- Final sandbox-compatible CTest run: 52 passed, 0 failed, 0 skipped. The loopback Gerrit test was excluded because
  the sandbox denies its bind; it passed separately in the preceding audit. Address/undefined sanitizer CTest run:
  52 passed, 0 failed, 0 skipped with leak detection disabled for the ptrace limitation. The four `group-tests`
  entries passed separately.
- The clean Release CTest run also passed 52 of 52 sandbox-compatible registrations. A new no-hardware CLI path
  validated the repository and staged-install manifests and confirmed that it did not construct a device backend.
  ThreadSanitizer built all 597 steps, but all four focused executions were blocked before test code by the known
  sandbox runtime error `unexpected memory mapping`; this is not counted as a pass or a code failure.
- `xWalkInterfaceGroupTest`, `xWalkDeviceGroupTest`, `xWalkSensorGroupTest`, and `xWalkLayer1GroupTest`: all pass
  and use deterministic hardware/service doubles with behaviour and failure assertions.
- Hardware-labelled tests were enumerated only and were not executed: the Raspberry Pi profile registers 44
  hardware-labelled entries and marks `xWalkToneSequenceHardwareTest` disabled.
- ARM64 configure: the cross compiler completed its ABI check after the toolchain began propagating the reviewed
  sysroot into CMake `try_compile` projects. The target-only audit then failed with an exact list of ten absent
  target package families: Protobuf, gRPC, OpenCV, ALSA, CURL, OpenSSL, TinyXML2, JSON-C, libsndfile, and yaml-cpp.
  Its synthetic host tests also reject target metadata outside the sysroot. No ARM64 compilation, QEMU execution,
  or hardware result is claimed.
- Native-host Raspberry Pi strict profile: configure and all 651 build steps succeeded, including Linux hardware
  backends, the CLI, and compiled-only hardware-test executables. The Release production source set built all 515
  steps. The outputs are x86-64 and no hardware test was run.
- Packaging validation first demonstrated that CPack could emit an `amd64` package from the x86 RPI compile-check
  profile. CMake now rejects that unsafe combination and fixes target packages to `arm64`; an actual ARM64 package
  remains blocked until the reviewed sysroot is available.

An initial parallel CTest run exposed the sandbox's prohibition on a loopback bind in `xWalkGerritCiHostTest`; that
test passed when granted loopback permission. Parallel aggregate wrappers also collided with their independently
registered child tests through shared writable fixture directories, producing a bad persisted value and one
concurrent directory removal. Serial aggregate registration corrected the test isolation; the affected component
tests passed 20 repetitions and the final four-way parallel suite passed. These observations are retained rather
than hidden by the final count.

## Working-tree changes relevant to this audit

- `xWalk-rpi5/xWalkAgent/xWalkVehicle/xWalkLineTracking/src/xAgent_Rpi5CarLineTracking.cpp`: stop recovery only after a valid
  non-Stop line state.
- `xWalk-rpi5/xWalkAgent/xWalkVehicle/xWalkLineTracking/test/src/xAgent_Rpi5CarLineTrackingTest.cpp`: persistent-loss
  regression with a bounded sample count and zero-output assertions.
- `xWalk-rpi5/xWalkAgent/CMakeLists.txt`: serialize the aggregate Agent test because it invokes the same group executables and
  writable group fixtures that CTest otherwise schedules independently.
- `xWalk-rpi5/xWalkController/xWalkTest/xGoogleTest/CMakeLists.txt` and
  `xWalk-rpi5/xWalkController/xWalkTest/xSequenceTest/CMakeLists.txt`: serialize controller aggregate wrappers for the same
  shared-child-fixture reason.
- `xWalkTool/shell/setup-rpi.sh`: install the Protobuf, gRPC, GoogleTest and TinyXML2 development packages required
  by a clean Raspberry Pi build.
- `xWalk-rpi5/xWalkHal/simulation/xWalkRobotHat`, the atomic I2C/ADC path, motor/servo lifecycle code, and their tests:
  deterministic device-free Robot HAT behaviour, fault injection, paired-stop, watchdog, and actuator lifecycle
  validation.
- `xWalk-rpi5/xWalkAgent/xWalkVision/xWalkRoadUserSafety` and OpenCV source tests: stable detector/classifier safety
  boundaries,
  synthetic risk scenarios, a generated end-to-end recorded scenario, finite-media EOF handling, and
  recorded-video input without a physical camera.
- `xWalk-rpi5/xWalkAgent/xWalkVision/xWalkVideoStreaming`: bounded multi-client MJPEG queueing, framing, drop-oldest
  backpressure, and camera-loss shutdown. It is not a network listener.
- `xWalk-rpi5/xWalkAgent/xWalkVision/xWalkVideoRecording/hardware/src/xAgent_Rpi5CarVideoRecordingOpenCv.cpp`: retain the
  released rollback context explicitly so focused Clang-Tidy reports no ignored `nodiscard` result.
- `xWalk-rpi5/cmake/toolchains/aarch64-linux-gnu.cmake` and `xWalkTool/deployment`: reviewed-sysroot cross-build
  guardrails
  and
  hardware-independent/wheels-up guidance. The dependency audit confines `pkg-config`, reports all missing target
  package families in one run, inspects linker inputs, and rejects host path contamination.
- `xWalk-rpi5/xWalkController/xWalkApp`: device-free configuration validation, sanitized effective values, and
  `--diagnose --no-hardware` execution before either host or Raspberry Pi boot construction. Schema version 1 is
  explicit, deterministically printed, and an explicitly unsupported version is rejected.
- `xWalk-rpi5/xWalkAgent/xWalkVehicle/xWalkPicarx/test/src/xAgent_Rpi5CarPicarxSimulationTest.cpp`: simulator-backed
  rejection,
  per-stage initialization faults, emergency recovery without command replay, and active-movement shutdown checks.
- `CMakePresets.json`, the AArch64 toolchain, and host CI: a reviewed-sysroot cross profile, reliable sysroot
  propagation through compiler probes, and deployment-manifest validation in host and staged-install jobs.
- `xWalk-rpi5/devloper-note/Doc/note/Raspberry Pi Setup Script Guide.md`: synchronize the documented
  package list.
- This audit report was updated with the current validation evidence and corrections above.
- `xWalk-rpi5/devloper-note/index.md`: link this report from the workspace documentation index.

The repository's unrelated pre-existing uncommitted work remains user-owned and was preserved. No commit, push,
pull request, hardware access, credential access, or real cloud request was performed.
