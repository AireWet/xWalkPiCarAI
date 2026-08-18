# xWalk CLI sequence tests

The host suite includes a dedicated `ServoZeroing` scenario that verifies the
CLI output plus every channel, angle, delay, and cancellation transition from
`servo-zeroing`. No hardware counterpart is registered.

This module owns bounded multi-command CLI verification. It mirrors the HAL sequence-test separation while
keeping CLI commands and Agent types under the CLI aggregate. Its independent
`xCliSequenceTest` executable owns the sequence GoogleTest process entry point.

## Layout and responsibilities

| Path | Responsibility |
| --- | --- |
| `main.cpp` | Sequence process entry point, registration, and child isolation |
| `config/test_config.xml` | Complete enabled sequence inventory by functional group |
| `config/hardware_test_config.xml` | Complete disabled sequence inventory for hardware selection |
| `core/include/xControllerSequenceTypes.h` | Ordered CLI command-sequence type |
| `core/include/xControllerSequence.h` | Bounded sequence contract |
| `core/src/xControllerSequence.cpp` | Validation, ordered execution, and failure termination |
| `core/src/xControllerSequenceTest.cpp` | Device-free sequence host verification |
| `core/include/xControllerCommandTestSupport.h` | Shared controller-to-HAL host-test boundary |
| `core/src/xControllerCommandTestSupport.cpp` | In-memory Agent and HAL composition |
| `core/src/x*SequenceTest.cpp` | One sequence scenario per public controller command group |
| `config/` | Reserved for reviewed CLI sequence profiles |
| `hardware/` | Reserved for explicitly approved CLI hardware adapters |

The sequence accepts one through 32 non-empty commands, validates the complete list before execution, and
stops at the first non-zero controller status. The generic contract test uses a bounded Doctor report fixture.
The command-specific files use injected, in-memory I2C, GPIO, PWM, ADC, SPI, audio, speech, language-model,
and cancellation callbacks to verify the CLI-to-Controller-to-Agent-to-HAL sequence without opening physical
devices, audio endpoints, cameras, or network resources.
The keyboard-control scenario injects all eight upstream keys plus invalid and quit input, then verifies
centered steering and stopped motors.
The obstacle-avoidance scenario verifies explicit stop and failed-ultrasonic cleanup; the owning Agent host
test covers all three source distance bands without accessing physical sensors.
The cliff-detection scenario verifies explicit stop, bounded foreground
cancellation, and final motor cleanup; its Agent test covers safe/danger state
transitions with configured grayscale references.
The computer-vision scenario exercises every source key group through an
in-memory camera provider and verifies detector state, object geometry,
timestamped photograph reporting, QR change reporting, bounded delays, and
provider shutdown without opening a camera.
The face-tracking scenario verifies camera start, face-detector enablement,
observation, both camera-servo writes, bounded cancellation, provider shutdown,
and final motor safety for `stare-at-you start|stop`.
The bull-fight scenario verifies red-detector selection, observation, camera
and steering writes, forward movement, cancellation, provider shutdown, and
final motor safety.
The treasure-hunt scenario verifies camera warm-up, deterministic injected
target selection, wide-color success, spoken prompts, bounded movement, target
repeat, quit, camera shutdown, motor cleanup, and goodbye speech.
The voice-active-car-gpt scenario verifies case-insensitive Buddy wake
detection, the wake answer, one model prompt, parsed action dispatch, response
speech, recognition shutdown, and final motor cleanup without external service
or microphone access.
The video-recording scenario verifies camera warm-up, start, pause, continue,
AVI stop reporting, and camera shutdown without opening a physical device.
The video-car scenario verifies every interactive key, retained speed and
motion transitions, the 60-percent direction-change cap, timestamped capture,
camera teardown, and final motor stop through the simulated HAL.
The app-control scenario verifies transport and camera lifecycle, telemetry,
joystick motion, camera limits, voice stop, line and obstacle modes, horn audio,
vision toggles, unsupported-object reporting, cancellation, and motor cleanup.
The sound-background-music scenario verifies initial volume, background-music
start and stop, synchronous and background horn requests, exact cancellable
post-horn delay slices, ignored input, and final stopped state.
The voice-prompt-car scenario verifies the greeting, all four spoken movement
prompts, exact two-second stages, forward/backward direction, signed steering,
and final motor and steering cleanup through the simulated HAL.
The storytelling-robot scenario verifies all four source narrations, two
three-second forward legs, the six-second backward leg, exact cancellable delay
slices, and final motor and steering cleanup.
The voice-controlled-car scenario scripts wake-word rejection and acceptance,
empty and unknown transcripts, all four movement commands, sleep, recognition
shutdown, all 200 exact 20-millisecond cancellation slices, and final stopped
and centred vehicle state.
The voice-chat scenario verifies the example-19 welcome, silence retry,
recognized prompt, hidden-thinking removal, clean speech response, exact
100/50-millisecond delays, cancellation, farewell, recognition shutdown, and
explicit CLI stop through the simulated voice HAL.
The text-vision-talk scenario verifies model setup, the exact two-second camera
warm-up, typed input, one 1280-by-720 image at `/tmp/llm-img.jpg`, image-grounded
model prompting, final response output, whitespace-insensitive exit, and stop.
The online-LLM-test scenario verifies model setup, repeated text-only prompts,
empty image paths, final response output, bounded external cancellation, and
explicit stop without network or credentials.

The shared composition records an ordered event trace at each injected boundary. Every command-specific test
checks the relevant subsequence—for example cancellation before movement, HAL writes before bounded delays,
SPI transfer before formatted output, speech shutdown before vehicle cleanup, and motor stop before calibration
sampling—then verifies the final safe state. The calibration case covers both full calibration and the
stationary `calibrate grayscale` mode.

## Host verification

From the repository root:

```bash
cmake --fresh --preset host-debug
cmake --build --preset host-debug --parallel
./build-host/cmake/xCliSequenceTest
ctest --preset host-debug
```

Standard GoogleTest filters can select individual command scenarios, for
example `--gtest_filter=XWalkAgentVoiceGroup.GptCar`. Without an explicit
filter, the executable loads all suite and case enablement from its own strict
`config/test_config.xml` manifest copied into the build directory.

The sibling [`../xGoogleTest`](../xGoogleTest) executable owns only CLI unit
tests. Physical CLI sequences remain absent until a bounded, reviewed command
flow and safe composition are supplied.
