# xWalk Agent

Agent production code qualifies the generic shared type vocabulary through the
`agent` namespace exported by `xHal_Rpi5CarTypes.h`. Hardware-specific classes,
callbacks, and sensor structures remain explicitly qualified through `hal`.

`xWalkAgent` contains application-level coordinators that compose caller-owned xWalk HAL objects. The
aggregate target exports `xWalk::Agent` and contains the `xWalkPicarx`, `xWalkLineTracking`,
`xWalkGrayscaleCalibration`, `xWalkServoMotorCalibration`, `xWalkServoZeroing`,
`xWalkMoveExample`, `xWalkKeyboardControl`,
`xWalkObstacleAvoidance`, `xWalkCliffDetection`, `xWalkComputerVision`,
`xWalkFaceTracking`, `xWalkBullFight`, `xWalkTreasureHunt`,
`xWalkVideoRecording`, `xWalkVideoCar`,
`xWalkAppControl`, `xWalkSoundBackgroundMusic`, `xWalkVoicePromptCar`,
`xWalkStorytellingRobot`, `xWalkVoiceControlledCar`, `xWalkTextVisionTalk`,
`xWalkOnlineLlmTest`,
`xWalkSelfDrive`, SPI transfer,
voice, camera capture, and `xWalkBoot` submodules. `xWalkLineTracking` ports the bounded behavior of
`example/6.line_tracking.py`, `xWalkGrayscaleCalibration` ports automatic line/cliff calibration, and
`xWalkServoMotorCalibration` ports pending
servo/motor calibration and previews, `xWalkMoveExample` ports `example/2.move.py`, `xWalkKeyboardControl`
ports `example/3.keyboard_control.py`, `xWalkObstacleAvoidance` ports
`example/4.avoiding_obstacles.py`, and `xWalkSelfDrive` ports named preset-action and background behavior.
`xWalkServoZeroing` ports `example/servo_zeroing.py` with all twelve servo
channels behind injected hardware and cancellation callbacks.
`xWalkCliffDetection` ports `example/5.cliff_detection.py` using persisted grayscale references.
`xWalkComputerVision` ports `example/7.computer_vision.py` through a portable
interactive state machine and an optional OpenCV Linux camera provider.
`xWalkFaceTracking` ports `example/8.stare_at_you.py` through the same
camera-provider boundary and bounded PiCar-X camera-servo commands.
`xWalkBullFight` ports `example/10.bull_fight.py` with red-target camera
tracking, steering, and forward motion.
`xWalkTreasureHunt` ports `example/20.treasure_hunt.py` with random color
targets, spoken prompts, bounded keyboard motion, and camera detection.
`xWalkVideoRecording` ports `example/9.record_video.py` with an optional
continuous OpenCV AVI provider.
`xWalkVideoCar` ports `example/11.video_car.py` with interactive motion and
photo capture through the existing OpenCV provider.
`xWalkAppControl` ports `example/12.app_control.py` through an injected A-Q
transport and an optional explicitly bound WebSocket provider.
`xWalkSoundBackgroundMusic` ports `example/13.sound_background_music.py`
through the shared music HAL, preserving synchronous and background horn
playback plus toggled background music.
`xWalkVoicePromptCar` ports `example/14.voice_promt_car.py` with synchronous
speech, four bounded movement stages, and fail-safe vehicle cleanup.
`xWalkStorytellingRobot` ports `example/15.storytelling_robot.py` with Piper
narration, two outward legs, and a longer backward trip home.
`xWalkVoiceControlledCar` ports `example/16.voice_controlled_car.py` with Vosk
wake-word recognition and repeated source-compatible movement commands.
`xWalkTextVisionTalk` ports `example/17.text_vision_talk.py` with typed prompts,
one still image per prompt, and a caller-owned Ollama-compatible model.
`xWalkOnlineLlmTest` ports `example/18.online_llm_test.py` with typed text-only
prompts and a caller-owned OpenAI-compatible model.
`xWalkLocalVoiceChatbot` ports `example/19.local_voice_chatbot.py` with Vosk
recognition, hidden-thinking filtering, and a caller-owned local voice pipeline.
`xWalkVoiceActiveCarGpt` ports `example/21.voice_active_car_gpt.py` with the
Jarvis wake profile, filtered action prompt, Gemini `gemini-3.6-flash`, Piper
`en_GB-alan-medium`, image input, and ten-centimetre proximity triggering.
`xWalkVoiceActiveCar` ports `example/voice_active_car.py` with the Rolly
profile, wake gating, ultrasonic trigger, image input, LED states, and actions.
`xWalkGptCar` ports `gpt_examples/gpt_car.py` with voice or keyboard input,
optional image context, JSON action responses, and the shared preset actions.
`xWalkCameraCapture` adapts the camera HAL to voice image input. `xWalkBoot` owns the process
hardware-composition lifetime, offline voice-provider graph, and device-free host stub.
`xWalkSpiTransfer` coordinates bounded full-duplex requests without owning the Linux device.

## Functional groups

The modules remain independently buildable and keep their existing target names and public headers. Their
source directories are physically organized under seven functional parents. Each parent provides a smaller
public interface target for consumers that do not need the complete aggregate:

| Target | Primary responsibility |
| --- | --- |
| [`xWalk::AgentVehicle`](xWalkVehicle/README.md) | Movement, line following, reactions, and preset actions |
| [`xWalk::AgentCalibration`](xWalkCalibration/README.md) | Grayscale, motor, and servo calibration |
| [`xWalk::AgentVision`](xWalkVision/README.md) | Camera capture, detection, tracking, games, and video |
| [`xWalk::AgentMedia`](xWalkMedia/README.md) | Sound effects and background music |
| [`xWalk::AgentVoice`](xWalkVoice/README.md) | Speech, narration, voice control, and conversational AI |
| [`xWalk::AgentConnectivity`](xWalkConnectivity/README.md) | Mobile-app control and SPI transactions |
| [`xWalk::AgentPlatform`](xWalkPlatform/README.md) | Host and Raspberry Pi boot composition |

`xWalk::Agent` continues to provide the complete set by linking all seven
groups. Physical grouping does not combine the distinct lifecycles of similarly named modules.

The `xWalkAgentGoogleTest` executable is the root Agent verification layer. It contains one GoogleTest case
per functional group and runs each group suite in an isolated process. Those group suites contain one case per
child module, so the aggregate verifies all 29 Agent modules without duplicating their test implementation.
Run only the root suite with:

```bash
ctest --test-dir build-host/cmake --output-on-failure -L agent-aggregate
```

The host aggregate registers one GoogleTest executable for every group and one named case for every child
module. Cases run an existing deterministic child test in an isolated process where one exists; newer modules
are checked directly through their public behavior and configuration contracts. Run the focused inventory with:

```bash
ctest --test-dir xWalk-rpi5/xWalkAgent/build-host --output-on-failure -L agent-group
```

The Raspberry Pi aggregate registers matching GoogleTest hardware-profile cases for every child module. These
cases verify the RPi group build graph; owning child hardware tests retain responsibility for physical device
behavior. Discover the group hardware tests without executing them with:

```bash
ctest --test-dir xWalk-rpi5/xWalkAgent/build-rpi -N -L agent-group
```

The `xWalkAgentHardwareGoogleTest` executable composes the seven hardware-profile group suites. Discover it
with the `agent-aggregate` label and execute it only after Raspberry Pi and Robot HAT safety approval.

The command-line application is a separate sibling aggregate under `xWalkController`. The standalone Agent
tree does not contain or compose `xWalkController` or `xWalkApp`.

## Runtime tracing

Agent diagnostics use registered `RPIAGENT` trace identifiers. Enable the complete Agent module for one
Jarvis run with:

```bash
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --trace RPIAGENT.enable voice-active-car-gpt start
```

Use an individual selector such as `--trace RPIAGENT.012.enable` when only one event is needed. Selection is
persisted in `<build-directory>/generated/xwalk-traces.xml`, and records are appended to
`<build-directory>/log/xWalkTrace.log`.

| Functional group | Child module | Trace identifiers |
| --- | --- | --- |
| Calibration | GrayscaleCalibration | `RPIAGENT.022` |
| Calibration | ServoMotorCalibration | `RPIAGENT.023` |
| Calibration | ServoZeroing | `RPIAGENT.016` |
| Connectivity | AppControl | `RPIAGENT.024` |
| Connectivity | SpiTransfer | `RPIAGENT.017` |
| Media | SoundBackgroundMusic | `RPIAGENT.018` |
| Platform | Boot | `RPIAGENT.019` |
| Vehicle | CliffDetection | `RPIAGENT.025` |
| Vehicle | KeyboardControl | `RPIAGENT.026` |
| Vehicle | LineTracking | `RPIAGENT.027` |
| Vehicle | MoveExample | `RPIAGENT.028` |
| Vehicle | ObstacleAvoidance | `RPIAGENT.029` |
| Vehicle | Picarx | `RPIAGENT.001` |
| Vehicle | SelfDrive | `RPIAGENT.002`–`RPIAGENT.009` |
| Vision | BullFight | `RPIAGENT.030` |
| Vision | CameraCapture | `RPIAGENT.020` |
| Vision | ComputerVision | `RPIAGENT.021` |
| Vision | FaceTracking | `RPIAGENT.031` |
| Vision | RoadUserSafety | `RPIAGENT.032` |
| Vision | TreasureHunt | `RPIAGENT.033` |
| Vision | VideoCar | `RPIAGENT.034` |
| Vision | VideoRecording | `RPIAGENT.035` |
| Vision | VideoStreaming | `RPIAGENT.036` |
| Voice | GptCar | `RPIAGENT.037` |
| Voice | LocalVoiceChatbot | `RPIAGENT.038` |
| Voice | OnlineLlmTest | `RPIAGENT.039` |
| Voice | StorytellingRobot | `RPIAGENT.040` |
| Voice | TextVisionTalk | `RPIAGENT.041` |
| Voice | VoiceActiveCar | `RPIAGENT.010`–`RPIAGENT.015` |
| Voice | VoiceActiveCarGpt | `RPIAGENT.042` |
| Voice | VoiceControlledCar | `RPIAGENT.043` |
| Voice | VoicePromptCar | `RPIAGENT.044` |

The production-source coverage continues with file-level operational events:

| Trace identifiers | Production source coverage |
| --- | --- |
| `RPIAGENT.045` | Grayscale calibration execution |
| `RPIAGENT.046`–`RPIAGENT.047` | App-control WebSocket facade and worker state |
| `RPIAGENT.048`–`RPIAGENT.071` | Individual Raspberry Pi Boot composition sources |
| `RPIAGENT.072`–`RPIAGENT.075` | Doctor assessment, Linux prerequisites, and host Boot stub |
| `RPIAGENT.076`–`RPIAGENT.083` | Vehicle danger, recovery, input, calibration, guard, sensing, and gesture events |
| `RPIAGENT.084` | MJPEG HTTP transport startup |
| `RPIAGENT.085`–`RPIAGENT.086` | Voice response filtering and default-profile selection |

Every project-owned production `.cpp` below the seven Agent functional groups contains at least one registered
diagnostic. Test sources remain governed by their owning test assertions and do not emit production trace IDs.

Warnings remain visible independently of normal trace selection. Agent traces never contain recognized speech,
prompts, model-response text, spoken text, credentials, action text, audio, images, or cleanup callback data.

Host verification is deterministic and uses in-memory callbacks:

```bash
cmake -S xWalkAgent -B xWalk-rpi5/xWalkAgent/build-host -DXWALK_AGENT_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkAgent/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkAgent/build-host --output-on-failure
```

The RPi build compiles Linux backends and hardware-labelled tests without running them:

```bash
cmake -S xWalkAgent -B xWalk-rpi5/xWalkAgent/build-rpi -DXWALK_AGENT_BUILD_RPI=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkAgent/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkAgent/build-rpi -N -L hardware
```

Never execute the RPi test until the correct Raspberry Pi and Robot HAT are connected, wheels are lifted, the
camera and steering mechanisms have clear travel, and powered motion has been approved.
