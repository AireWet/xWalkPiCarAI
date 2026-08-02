# xWalk C++ Documentation Index

This index links the current C++ architecture and module documentation. Module-specific contracts, build
options, backend ownership, safety constraints, and verification commands remain in each module README.

## Workspace architecture

- [xWalkCommon](../xWalkCommon/README.md): shared types, constants, exceptions, and reusable facilities.
- [xWalkHal](../xWalkHal/README.md): aggregate hardware-abstraction architecture and build options.
- [xWalkAgent](../xWalkAgent/README.md): application coordinators and aggregate Agent verification.
- [xWalkCLI](../xWalkCLI/README.md): standalone CLI aggregate and Controller composition.
- [High-level architecture](Doc/note/HAL%20Arcithure.md): workspace layers and ownership rules.
- [Hardware architecture](Doc/note/HAL%20Hardware%20Architecture.md): devices and backends.
- [CLI architecture](Doc/note/CLI%20Architecture.md): command coverage and implementation plan.
- [Raspberry Pi deployment guide](Doc/note/Deployment%20Guide.md): install layout, idempotent setup, board-profile
  checks, and narrowly scoped device permissions.
- [Raspberry Pi setup script guide](Doc/note/Raspberry%20Pi%20Setup%20Script%20Guide.md): setup options,
  safe dry-run, privileged apply behavior, validation, and troubleshooting.
- [xWalkTool overview](Doc/note/xWalkTool%20Overview.md): tool inventory, safety classes, and detailed links.
- [Dependency installer guide](Doc/note/Dependency%20Installer%20Guide.md): cross-platform package checks,
  guarded Robot HAT v5 boot setup, status tables, and safety limits.
- [Clean build script guide](Doc/note/Clean%20Build%20Script%20Guide.md): cleanup discovery and confirmation.
- [Eclipse build script guide](Doc/note/Eclipse%20Build%20Script%20Guide.md): host build and clean behavior.
- [Hardware provisioning guide](Doc/note/Hardware%20Provisioning%20Script%20Guide.md): board and Linux device
  identity validation and configuration persistence.
- [Host coverage script guide](Doc/note/Host%20Coverage%20Script%20Guide.md): foreground workflow and reports.
- [Device Tree overlay guide](Doc/note/Device%20Tree%20Overlay%20Assets%20Guide.md): asset roles and inspection.
- [Release acceptance checklist](Doc/note/Release%20Acceptance%20Checklist.md): independent host, ARM package,
  plug-and-run, and physical-safety evidence gates.
- [Host production readiness work](Doc/note/Host%20Production%20Readiness%20Work.md): host sanitizer, stress,
  coverage, clean-environment, and staged-install verification.
- [Language-model provider configuration](Doc/note/Language%20Model%20Provider%20Configuration.md): Ollama,
  ChatGPT, Gemini, Claude, credential handling, and model selection.

## Robot HAT board diagram

![Robot HAT board pinout](Doc/image/robot_hat_pinout.png)

The diagram identifies the Robot HAT power, motor, PWM, ADC, digital, I2C, SPI, UART, button, and speaker
connections. See [Hardware introduction](Doc/note/Hardware%20Introduction.md) for the corresponding hardware
notes.

## Agent modules

- [xWalkController](../xWalkCLI/xWalkController/README.md): commands, backend boot, and RPi composition.
- [xWalkBoot](../xWalkAgent/xWalkBoot/README.md): host stub and RPi process hardware ownership.
- [xWalkLineTracking](../xWalkAgent/xWalkLineTracking/README.md): bounded line-following coordination.
- [xWalkPicarx](../xWalkAgent/xWalkPicarx/README.md): movement, calibration, and sensor coordination.
- [xWalkSelfDrive](../xWalkAgent/xWalkSelfDrive/README.md): preset gestures, sounds, and action queue.
- [xWalkSpiTransfer](../xWalkAgent/xWalkSpiTransfer/README.md): bounded SPI transaction coordination.
- [xWalkVoiceActiveCar](../xWalkAgent/xWalkVoiceActiveCar/README.md): sensor, wake-word, and spoken-demo cars.

## HAL modules

- [xWalkAdc](../xWalkHal/xWalkAdc/README.md): Robot HAT analog-to-digital conversion.
- [xWalkAdxl345](../xWalkHal/xWalkAdxl345/README.md): ADXL345 acceleration sensing.
- [xWalkAudio](../xWalkHal/xWalkAudio/README.md): shared ALSA PCM, mixer, and device ownership.
- [xWalkBoardControl](../xWalkHal/xWalkBoardControl/README.md): board reset, discovery, and firmware data.
- [xWalkBuzzer](../xWalkHal/xWalkBuzzer/README.md): active and passive buzzer control.
- [xWalkConfig](../xWalkHal/xWalkConfig/README.md): configuration parsing and persistence.
- [xWalkGpio](../xWalkHal/xWalkGpio/README.md): GPIO abstraction and Linux backend.
- [xWalkGPT](../xWalkHal/xWalkGPT/README.md): speech coordination plus Vosk and Espeak providers.
- [xWalkI2c](../xWalkHal/xWalkI2c/README.md): I2C abstraction and Linux backend.
- [xWalkLanguageModel](../xWalkHal/xWalkLanguageModel/README.md): provider-neutral language-model access.
- [xWalkLed](../xWalkHal/xWalkLed/README.md): GPIO and PWM LED control.
- [xWalkLineTracker](../xWalkHal/xWalkLineTracker/README.md): grayscale line-position estimation.
- [xWalkMotor](../xWalkHal/xWalkMotor/README.md): single and paired motor control.
- [xWalkMusic](../xWalkHal/xWalkMusic/README.md): musical tones and injected audio playback.
- [xWalkPwm](../xWalkHal/xWalkPwm/README.md): Robot HAT PWM output.
- [xWalkRobot](../xWalkHal/xWalkRobot/README.md): coordinated multi-servo robot control.
- [xWalkServo](../xWalkHal/xWalkServo/README.md): calibrated servo positioning.
- [xWalkSpeaker](../xWalkHal/xWalkSpeaker/README.md): decoded audio-file playback.
- [xWalkSpi](../xWalkHal/xWalkSpi/README.md): bounded SPI abstraction and Linux backend.
- [xWalkTrace](../xWalkHal/xWalkTrace/README.md): filtered callback-based diagnostics.
- [xWalkUltrasonic](../xWalkHal/xWalkUltrasonic/README.md): ultrasonic distance measurement.
- [xWalkUserButton](../xWalkHal/xWalkUserButton/README.md): button events and press timing.
- [xWalkUtils](../xWalkHal/xWalkUtils/README.md): platform utilities and bounded lazy caching.
- [xWalkVoiceAssistant](../xWalkHal/xWalkVoiceAssistant/README.md): speech and model backend composition.

## Safety and verification

Use host tests for ordinary verification. Raspberry Pi hardware tests are opt-in and must only run when the
correct Raspberry Pi and Robot HAT are connected, the robot is secured, and physical execution is approved.
Use `ctest -N -L hardware` to list hardware tests without executing them.
