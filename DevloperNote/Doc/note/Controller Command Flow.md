# xWalk Controller Command Flow

This document traces every `xwalk-picarx-control` command from terminal input to its final Raspberry Pi
hardware, local service, file, or network boundary. It reflects the current Controller, Agent, boot, and HAL
composition.

The host executable follows the same parsing and handler path but uses host-owned test or stub services. Only
the Raspberry Pi executable creates Linux hardware backends.

## Common Raspberry Pi path

Help is handled before boot and therefore claims no device. Every other command first selects its minimum boot
graph from the command name. The complete command is converted into a typed request only after the selected
services are alive inside the synchronous boot callback.

```mermaid
flowchart TD
    terminal["Terminal: xwalk-picarx-control COMMAND"]
    global["Parse global deployment and resource options"]
    helpCheck{"Help request?"}
    help["Generate usage text and exit without boot"]
    bootMode["XWALK_selectBootMode: command name to boot macro"]
    config["Load picar-x.conf and included fragments"]
    boot["XWalkBootRpi: create only the selected service graph"]
    callback["XWALK_runController: bind services to XWalkController"]
    parser["XWALK_parseControllerCommand: command request macro"]
    request["Command-specific parser: validated typed request DTO"]
    handler["XWALK_handler command function"]
    agent["Command-specific Agent or XWalkPicarx operation"]
    hal["HAL coordinator and Linux backend"]
    endpoint["Hardware, local file/process, or network endpoint"]
    cleanup["Safety guard, service shutdown, and reverse-order teardown"]

    terminal --> global
    global --> helpCheck
    helpCheck -- Yes --> help
    helpCheck -- No --> bootMode
    bootMode --> config
    config --> boot
    boot --> callback
    callback --> parser
    parser --> request
    request --> handler
    handler --> agent
    agent --> hal
    hal --> endpoint
    endpoint --> cleanup
```

Invalid global options stop before boot. An unsupported command, malformed payload, out-of-range value, or
missing command-specific service stops before the requested operation reaches its endpoint. Commands using the
PiCar-X graph also receive a command-scope emergency-stop guard.

The current boot selector returns `XWALK_BOOT_BASE_REQ` for an empty or unknown command name. Consequently, the
RPi executable can construct the base hardware graph before the typed parser rejects that command. This is a
known command-boundary limitation; only supported command names should be passed to a deployed controller.

## Platform and isolated-service commands

These commands deliberately bypass the complete PiCar-X motor graph unless their documented service needs it.

```mermaid
flowchart LR
    help["-h, --help, help"]
    helpParser["XWALK_CNTRL_HELP_REQ"]
    helpHandler["XWALK_controllerUsage"]
    console["Standard output; no boot or hardware"]
    help --> helpParser --> helpHandler --> console

    doctor["doctor"]
    doctorBoot["XWALK_BOOT_DOCTOR_REQ"]
    doctorRequest["XWalkNoArgumentRequest"]
    doctorHandler["XWALK_handlerDoctor"]
    doctorAgent["XWalkDoctorLinux inspection"]
    doctorEnd["Read-only system, configuration, and device metadata"]
    doctor --> doctorBoot --> doctorRequest --> doctorHandler
    doctorHandler --> doctorAgent --> doctorEnd

    spi["spi transfer HEX"]
    spiBoot["XWALK_BOOT_SPI_TRANSFER_REQ"]
    spiRequest["XWalkSpiRequest: transmitData"]
    spiHandler["XWALK_handlerSpi"]
    spiAgent["XWalkSpiTransfer"]
    spiHal["XWalkSpi and XWalkSpiLinux"]
    spiEnd["Configured /dev/spidev endpoint"]
    spi --> spiBoot --> spiRequest --> spiHandler
    spiHandler --> spiAgent --> spiHal --> spiEnd

    zero["servo-zeroing"]
    zeroBoot["XWALK_BOOT_SERVO_ZEROING_REQ"]
    zeroRequest["XWalkNoArgumentRequest"]
    zeroHandler["XWALK_handlerServoZeroing"]
    zeroAgent["XWalkServoZeroing"]
    zeroHal["XWalkServo, XWalkPwm, and XWalkI2cLinux"]
    zeroEnd["Robot HAT PWM channels 0 through 11"]
    zero --> zeroBoot --> zeroRequest --> zeroHandler
    zeroHandler --> zeroAgent --> zeroHal --> zeroEnd
```

`doctor` may open configured descriptors and read metadata, firmware, or battery information. It does not reset
the MCU, request output GPIO lines, transfer SPI payloads, enable audio, capture media, or contact a model.

## Direct vehicle and sensor commands

These commands use `XWALK_BOOT_BASE_REQ` unless a specialized graph is shown. Their typed requests prevent raw
command strings from reaching PiCar-X or HAL objects.

```mermaid
flowchart LR
    move["move forward, backward, or demo"]
    moveReq["XWalkMoveRequest"]
    moveHandler["XWALK_handlerMove"]
    moveAgent["XWalkPicarx or XWalkMoveExample"]
    moveOut["Motor PWM, steering servo, and optional camera servos"]
    move --> moveReq --> moveHandler --> moveAgent --> moveOut

    keyboard["keyboard-control"]
    keyboardReq["XWalkNoArgumentRequest"]
    keyboardHandler["XWALK_handlerKeyboardControl"]
    keyboardAgent["XWalkKeyboardControl"]
    keyboardOut["Motor PWM and steering or camera servos"]
    keyboard --> keyboardReq --> keyboardHandler --> keyboardAgent --> keyboardOut

    avoid["avoid-obstacles start or stop"]
    avoidReq["XWalkLifecycleRequest"]
    avoidHandler["XWALK_handlerObstacleAvoidance"]
    avoidAgent["XWalkObstacleAvoidance and XWalkPicarx"]
    avoidOut["Ultrasonic GPIO input plus motors and steering"]
    avoid --> avoidReq --> avoidHandler --> avoidAgent --> avoidOut

    cliff["cliff-detection start or stop"]
    cliffReq["XWalkLifecycleRequest"]
    cliffHandler["XWALK_handlerCliffDetection"]
    cliffAgent["XWalkCliffDetection and XWalkPicarx"]
    cliffOut["Three ADC grayscale channels plus motors"]
    cliff --> cliffReq --> cliffHandler --> cliffAgent --> cliffOut

    turn["turn left or right"]
    turnReq["XWalkTurnRequest"]
    turnHandler["XWALK_handlerTurn"]
    turnAgent["XWalkPicarx steering and bounded movement"]
    turnOut["Direction servo and motor PWM"]
    turn --> turnReq --> turnHandler --> turnAgent --> turnOut

    camera["cam pan or tilt"]
    cameraReq["XWalkCameraRequest"]
    cameraHandler["XWALK_handlerCamera"]
    cameraAgent["XWalkPicarx camera-servo operation"]
    cameraOut["Pan or tilt PWM servo"]
    camera --> cameraReq --> cameraHandler --> cameraAgent --> cameraOut

    sensor["sensor distance or grayscale"]
    sensorReq["XWalkSensorRequest"]
    sensorHandler["XWALK_handlerSensor"]
    sensorAgent["XWalkPicarx sensing"]
    sensorOut["Ultrasonic GPIO or three ADC channels"]
    sensor --> sensorReq --> sensorHandler --> sensorAgent --> sensorOut
```

## Autonomous vehicle and media commands

```mermaid
flowchart LR
    line["line-track start or stop"]
    lineBoot["XWALK_BOOT_LINE_TRACKING_REQ"]
    lineReq["XWalkLifecycleRequest"]
    lineHandler["XWALK_handlerLineTracking"]
    lineAgent["XWalkLineTracking and XWalkPicarx"]
    lineOut["Grayscale ADC, steering servo, and motor PWM"]
    line --> lineBoot --> lineReq --> lineHandler --> lineAgent --> lineOut

    self["self-drive ACTION"]
    selfBoot["XWALK_BOOT_SELF_DRIVE_REQ"]
    selfReq["XWalkSelfDriveRequest"]
    selfHandler["XWALK_handlerSelfDrive"]
    selfAgent["XWalkSelfDrive and XWalkPicarx"]
    selfOut["Motors, servos, ALSA output, and packaged sounds"]
    self --> selfBoot --> selfReq --> selfHandler --> selfAgent --> selfOut

    sound["sound play, volume, music, or stop"]
    soundBoot["XWALK_BOOT_SOUND_REQ"]
    soundReq["XWalkSoundRequest"]
    soundHandler["XWALK_handlerSound"]
    soundAgent["Controller sound callback and XWalkMusic"]
    soundOut["ALSA PCM or mixer and packaged audio file"]
    sound --> soundBoot --> soundReq --> soundHandler --> soundAgent --> soundOut

    background["sound-background-music"]
    backgroundBoot["XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ"]
    backgroundReq["XWalkNoArgumentRequest"]
    backgroundHandler["XWALK_handlerSoundBackgroundMusic"]
    backgroundAgent["XWalkSoundBackgroundMusic and XWalkMusic"]
    backgroundOut["ALSA output and packaged sound or music files"]
    background --> backgroundBoot --> backgroundReq --> backgroundHandler
    backgroundHandler --> backgroundAgent --> backgroundOut
```

## Vision and connectivity commands

```mermaid
flowchart LR
    vision["computer-vision"]
    visionBoot["XWALK_BOOT_COMPUTER_VISION_REQ"]
    visionReq["XWalkNoArgumentRequest"]
    visionHandler["XWALK_handlerComputerVision"]
    visionAgent["XWalkComputerVision and OpenCV provider"]
    visionOut["Configured camera and photograph directory"]
    vision --> visionBoot --> visionReq --> visionHandler --> visionAgent --> visionOut

    face["stare-at-you start or stop"]
    faceBoot["XWALK_BOOT_FACE_TRACKING_REQ"]
    faceReq["XWalkLifecycleRequest"]
    faceHandler["XWALK_handlerFaceTracking"]
    faceAgent["XWalkFaceTracking and OpenCV provider"]
    faceOut["Camera, camera servos, and motor cleanup"]
    face --> faceBoot --> faceReq --> faceHandler --> faceAgent --> faceOut

    bull["bull-fight start or stop"]
    bullBoot["XWALK_BOOT_BULL_FIGHT_REQ"]
    bullReq["XWalkLifecycleRequest"]
    bullHandler["XWALK_handlerBullFight"]
    bullAgent["XWalkBullFight and OpenCV provider"]
    bullOut["Camera, camera and steering servos, and motor PWM"]
    bull --> bullBoot --> bullReq --> bullHandler --> bullAgent --> bullOut

    treasure["treasure-hunt"]
    treasureBoot["XWALK_BOOT_TREASURE_HUNT_REQ"]
    treasureReq["XWalkNoArgumentRequest"]
    treasureHandler["XWALK_handlerTreasureHunt"]
    treasureAgent["XWalkTreasureHunt, OpenCV, and Pico2Wave"]
    treasureOut["Camera, motors, steering, speaker, and audio process"]
    treasure --> treasureBoot --> treasureReq --> treasureHandler
    treasureHandler --> treasureAgent --> treasureOut

    record["record-video"]
    recordBoot["XWALK_BOOT_VIDEO_RECORDING_REQ"]
    recordReq["XWalkNoArgumentRequest"]
    recordHandler["XWALK_handlerVideoRecording"]
    recordAgent["XWalkVideoRecording and OpenCV provider"]
    recordOut["Configured camera and AVI output directory"]
    record --> recordBoot --> recordReq --> recordHandler --> recordAgent --> recordOut

    video["video-car"]
    videoBoot["XWALK_BOOT_VIDEO_CAR_REQ"]
    videoReq["XWalkNoArgumentRequest"]
    videoHandler["XWALK_handlerVideoCar"]
    videoAgent["XWalkVideoCar and OpenCV provider"]
    videoOut["Camera, photograph files, motors, and steering"]
    video --> videoBoot --> videoReq --> videoHandler --> videoAgent --> videoOut

    app["app-control start or stop"]
    appBoot["XWALK_BOOT_APP_CONTROL_REQ"]
    appReq["XWalkLifecycleRequest"]
    appHandler["XWALK_handlerAppControl"]
    appAgent["XWalkAppControl and WebSocket provider"]
    appOut["Configured socket, camera, audio, motors, and servos"]
    app --> appBoot --> appReq --> appHandler --> appAgent --> appOut
```

## Voice and language-model commands

Credentials are read only from the configured environment-variable names. Secret values never become request
DTO members and are not displayed by these paths.

```mermaid
flowchart LR
    chat["voice-chat start or stop"]
    chatBoot["XWALK_BOOT_VOICE_CHAT_REQ"]
    chatReq["XWalkLifecycleRequest"]
    chatHandler["XWALK_handlerVoiceChat"]
    chatAgent["XWalkLocalVoiceChatbot and XWalkVoiceAssistant"]
    chatOut["ALSA microphone, Vosk, local Ollama, Piper, and speaker"]
    chat --> chatBoot --> chatReq --> chatHandler --> chatAgent --> chatOut

    rolly["voice-active-car start or stop"]
    rollyBoot["XWALK_BOOT_VOICE_ACTIVE_CAR_REQ"]
    rollyReq["XWalkLifecycleRequest"]
    rollyHandler["XWALK_handlerVoiceActiveCar"]
    rollyAgent["XWalkVoiceActiveCar Rolly profile"]
    rollyOut["Mic, Vosk, OpenAI, TTS, camera, LED, sensors, motors, and servos"]
    rolly --> rollyBoot --> rollyReq --> rollyHandler --> rollyAgent --> rollyOut

    buddy["voice-active-car-gpt start or stop"]
    buddyBoot["XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ"]
    buddyReq["XWalkLifecycleRequest"]
    buddyHandler["XWALK_handlerVoiceActiveCar"]
    buddyAgent["XWalkVoiceActiveCar Buddy profile"]
    buddyOut["Mic, Vosk, OpenAI, Piper, camera, LED, sensors, motors, and servos"]
    buddy --> buddyBoot --> buddyReq --> buddyHandler --> buddyAgent --> buddyOut

    gpt["gpt-car start or stop with input flags"]
    gptBoot["XWALK_BOOT_GPT_CAR_REQ"]
    gptReq["XWalkGptCarRequest"]
    gptHandler["XWALK_handlerGptCar"]
    gptAgent["XWalkGptCar over voice-active and SelfDrive services"]
    gptOut["Keyboard or mic, OpenAI, camera, audio, LED, motors, and servos"]
    gpt --> gptBoot --> gptReq --> gptHandler --> gptAgent --> gptOut

    controlled["voice-controlled-car start or stop"]
    controlledBoot["XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ"]
    controlledReq["XWalkLifecycleRequest"]
    controlledHandler["XWALK_handlerVoiceControlledCar"]
    controlledAgent["XWalkVoiceControlledCar"]
    controlledOut["ALSA microphone, Vosk, motor PWM, and steering servo"]
    controlled --> controlledBoot --> controlledReq --> controlledHandler
    controlledHandler --> controlledAgent --> controlledOut

    prompt["voice-prompt-car start or stop"]
    promptBoot["XWALK_BOOT_VOICE_PROMPT_CAR_REQ"]
    promptReq["XWalkLifecycleRequest"]
    promptHandler["XWALK_handlerVoicePromptCar"]
    promptAgent["XWalkVoicePromptCar"]
    promptOut["Espeak, ALSA speaker, motor PWM, and steering servo"]
    prompt --> promptBoot --> promptReq --> promptHandler --> promptAgent --> promptOut

    story["storytelling-robot start or stop"]
    storyBoot["XWALK_BOOT_STORYTELLING_ROBOT_REQ"]
    storyReq["XWalkLifecycleRequest"]
    storyHandler["XWALK_handlerStorytellingRobot"]
    storyAgent["XWalkStorytellingRobot"]
    storyOut["Piper, audio playback, motor PWM, and steering servo"]
    story --> storyBoot --> storyReq --> storyHandler --> storyAgent --> storyOut

    textVision["text-vision-talk start or stop"]
    textVisionBoot["XWALK_BOOT_TEXT_VISION_TALK_REQ"]
    textVisionReq["XWalkLifecycleRequest"]
    textVisionHandler["XWALK_handlerTextVisionTalk"]
    textVisionAgent["XWalkTextVisionTalk"]
    textVisionOut["Still camera, image file, and local Ollama vision model"]
    textVision --> textVisionBoot --> textVisionReq --> textVisionHandler
    textVisionHandler --> textVisionAgent --> textVisionOut

    online["online-llm-test start or stop"]
    onlineBoot["XWALK_BOOT_ONLINE_LLM_TEST_REQ"]
    onlineReq["XWalkLifecycleRequest"]
    onlineHandler["XWALK_handlerOnlineLlmTest"]
    onlineAgent["XWalkOnlineLlmTest"]
    onlineOut["Configured HTTPS model endpoint; no vehicle hardware"]
    online --> onlineBoot --> onlineReq --> onlineHandler --> onlineAgent --> onlineOut
```

## Calibration command

Calibration uses the base PiCar-X boot graph. It may move actuators and motors, read sensors, and persist
validated values through `XWalkConfigStore`.

```mermaid
flowchart LR
    calibration["calibrate, calibrate grayscale, or calibrate servo-motor"]
    calibrationBoot["XWALK_BOOT_BASE_REQ"]
    calibrationReq["XWalkCalibrationRequest"]
    calibrationHandler["XWALK_handlerCalibration"]
    calibrationAgents["GrayscaleCalibration and ServoMotorCalibration"]
    calibrationHal["XWalkPicarx, ADC, PWM, servo, motor, and ConfigStore"]
    calibrationOut["Physical checks plus persisted calibration overrides"]

    calibration --> calibrationBoot --> calibrationReq --> calibrationHandler
    calibrationHandler --> calibrationAgents --> calibrationHal --> calibrationOut
```

The complete calibration flow is safety-sensitive. Motor-direction verification must be performed with the
vehicle raised so its wheels cannot contact the ground.

## Terminal outcomes

```mermaid
flowchart TD
    request["Validated command request"]
    available{"Selected service available?"}
    execute["Execute handler and endpoint operation"]
    unavailable["Return status 3: backend unavailable"]
    valid{"Operation completes successfully?"}
    success["Return status 0"]
    reported["Return command-specific nonzero status"]
    cleanup["Stop active work and destroy boot graph in reverse order"]

    request --> available
    available -- No --> unavailable
    available -- Yes --> execute
    execute --> valid
    valid -- Yes --> success
    valid -- No --> reported
    unavailable --> cleanup
    success --> cleanup
    reported --> cleanup
```

Exceptions from validation or backend failures propagate to the process boundary after automatic objects begin
their reverse-order cleanup. PiCar-X command paths additionally use the emergency-stop safety guard.
