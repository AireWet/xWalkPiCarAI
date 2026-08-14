# xWalkHandler

`xWalkHandler` owns the hardware-independent `XWalkController` contract,
command-text conversion, callback boundary, and command-handler implementations
used by the standalone CLI. Application-level command dispatch is owned by the
sibling `xWalkApp` directory.

All Controller-owned classes, request DTOs, callbacks, and command functions
use `namespace xwalk::ctrl`. Existing Agent coordinators remain in the
parent `xwalk::agent` namespace. Inside the nested Controller namespace, shared
primitive aliases use the explicit global form `::ctrl::*` to avoid namespace
shadowing.

The handler stores non-owning pointers to caller-owned Agent coordinators and a
copied callback table. Its in-memory host test remains beside the implementation.
Application entry points, generated-help resources, and application GoogleTest
remain in the sibling `xWalkApp` directory. Its `CMakeLists.txt` file
compiles these sources without changing the public target name or runtime
behavior.

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xController.h` | Controller contract and command handlers |
| `include/xControllerHelp.h` | Generated-help selection and source fallback |
| `include/xControllerTypes.h` | Platform callback, option, and sound types |
| `src/xControllerLifecycle.cpp` | Dependency binding and callback forwarding |
| `src/common/` | Shared cancellation and command-safety support |
| `src/vehicle/` | Movement, sensing, line-tracking, and self-drive handlers |
| `src/vision/` | Camera, detection, tracking, and video handlers |
| `src/voice/` | Voice, language-model, and storytelling handlers |
| `src/media/` | Sound and background-music handlers |
| `src/connectivity/` | Mobile-app and SPI handlers |
| `src/calibration/` | Servo-zeroing, calibration, and verification handlers |
| `src/platform/` | Passive platform-diagnostic handlers |
| `test/src/xControllerTest.cpp` | Deterministic in-memory command-handler verification |

Command handlers use the requested
`XWALK_handler<CommandOrModuleName>` member-function convention. Public header
basenames remain unchanged, so consumers continue to include
`xController.h` through the CMake target's public include path.
Every `XWALK_handler...` member has one dedicated source file. Functionality
directories express the owning command domain; shared non-handler methods stay
in the root or the `common` and calibration-support sources. The application
free functions are declared and implemented under `../xWalkApp`.

CLI text is accepted only by the free parsing boundary in sibling `xWalkApp`.
Before dispatch, it is
converted into the command-specific enums and plain request structures declared
in `xWalk-rpi5/xWalkLibrary/common/include/xWalkControllerConfigTypes.h`. Handlers receive
typed actions, numeric values with explicit units, paths, byte payloads, and
flags; they do not index or interpret a `ctrl::stringvector`. Commands with no
payload receive `XWalkNoArgumentRequest`, and commands whose only value is
`start` or `stop` share `XWalkLifecycleRequest`.

## Handler source inventory

| Group | Source | Handler responsibility |
| --- | --- | --- |
| Vehicle | `vehicle/xControllerMoveHandler.cpp` | Timed forward and backward movement |
| Vehicle | `vehicle/xControllerMoveExampleHandler.cpp` | Bounded movement example |
| Vehicle | `vehicle/xControllerKeyboardControlHandler.cpp` | Keyboard driving |
| Vehicle | `vehicle/xControllerObstacleAvoidanceHandler.cpp` | Obstacle avoidance |
| Vehicle | `vehicle/xControllerCliffDetectionHandler.cpp` | Cliff detection |
| Vehicle | `vehicle/xControllerTurnHandler.cpp` | Steering-angle control |
| Vehicle | `vehicle/xControllerSensorHandler.cpp` | Sensor reporting |
| Vehicle | `vehicle/xControllerLineTrackingHandler.cpp` | Foreground line tracking |
| Vehicle | `vehicle/xControllerSelfDriveHandler.cpp` | Self-drive preset actions |
| Vision | `vision/xControllerComputerVisionHandler.cpp` | Interactive computer vision |
| Vision | `vision/xControllerFaceTrackingHandler.cpp` | Face tracking |
| Vision | `vision/xControllerBullFightHandler.cpp` | Red-target pursuit |
| Vision | `vision/xControllerTreasureHuntHandler.cpp` | Treasure-hunt behavior |
| Vision | `vision/xControllerVideoRecordingHandler.cpp` | Video recording |
| Vision | `vision/xControllerVideoCarHandler.cpp` | Camera-assisted driving |
| Vision | `vision/xControllerCameraHandler.cpp` | Camera-servo control |
| Voice | `voice/xControllerVoiceActiveCarHandler.cpp` | Voice-active car |
| Voice | `voice/xControllerGptCarHandler.cpp` | GPT car |
| Voice | `voice/xControllerVoiceControlledCarHandler.cpp` | Wake-word control |
| Voice | `voice/xControllerVoicePromptCarHandler.cpp` | Spoken movement demonstration |
| Voice | `voice/xControllerStorytellingRobotHandler.cpp` | Storytelling robot |
| Voice | `voice/xControllerTextVisionTalkHandler.cpp` | Image-grounded conversation |
| Voice | `voice/xControllerOnlineLlmTestHandler.cpp` | Online text conversation |
| Voice | `voice/xControllerVoiceChatHandler.cpp` | Local voice chatbot |
| Media | `media/xControllerSoundBackgroundMusicHandler.cpp` | Music and sound interaction |
| Media | `media/xControllerSoundHandler.cpp` | Sound playback and volume |
| Connectivity | `connectivity/xControllerAppControlHandler.cpp` | Mobile-app control |
| Connectivity | `connectivity/xControllerSpiHandler.cpp` | SPI transfer |
| Calibration | `calibration/xControllerServoZeroingHandler.cpp` | Servo zeroing |
| Calibration | `calibration/xControllerCalibrationHandler.cpp` | Calibration selection |
| Calibration | `calibration/xControllerFirstRunVerificationHandler.cpp` | First-run checks |
| Platform | `platform/xControllerDoctorHandler.cpp` | Passive diagnostic report |

`calibration/xControllerCalibrationSupport.cpp` contains only the
shared calibration steps used by the dedicated calibration handlers. It does
not define another command handler. PiCar-X command selection is an
application-owned free function declared and implemented in the sibling
`xWalkApp` directory.

## Trace reporting

Command handlers do not use the Controller output callback for status,
results, warnings, or failures. Normal handler records use a unique
`XWALK_CTRL_TRACE_UIDn` identifier, recoverable failures use
`XWALK_CTRL_ERROR` with `XWALK_EXCEPTION`, and warnings use
`XWALK_CTRL_WARNING`. Throwing failures use `XWALK_CTRL_ERROR` with the
appropriate short C++ selector. The output callback remains available at the
application boundary for
help text and to Agent workflows whose public interaction contract requires
conversation output.

## Build and host verification

Build and test through the owning controller aggregate:

```bash
cmake -S xWalkController -B xWalk-rpi5/xWalkController/build-host -DXWALK_CLI_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkController/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkController/build-host --output-on-failure
```

These commands are host-only. Physical Raspberry Pi behavior remains opt-in;
see the [application guide](../xWalkApp/README.md) for hardware
build discovery and command safety requirements.
