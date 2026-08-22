# xWalk Firmware Robot HAT documentation

This directory is the workspace-level C++ documentation set for the xWalk HAL. It follows the
page structure of the Robot HAT source documentation while describing only the
current C++ implementation.

## Reference index

### Hardware

- [Features](Features.md)
- [Hardware introduction](Hardware%20Introduction.md)
- [Battery](Battery.md)
- [Onboard MCU](Onboard%20MCU.md)
- [Speaker deployment](Install%20I2S%20for%20Speaker.md)
- [Safety and troubleshooting](FAQ.md)

### C++ API

- [API index](API.md)
- [ADC](API%20ADC.md)
- [I2C](API%20I2C.md)
- [PWM](API%20PWM.md)
- [Servo](API%20Servo.md)
- [GPIO](API%20Pin.md)
- [Motor](API%20Motor.md)
- [Robot](API%20Robot.md)
- [Sensors and actuators](API%20Modules.md)
- [Music](API%20Music.md)
- [Configuration store](API%20Filedb.md)
- [Trace](API%20Basic%20Class.md)
- [Speech](API%20TTS.md)
- [Utilities](API%20Utils.md)

### Build and examples

- [Build and installation](Installation.md)
- [CMake dependencies](Dependency%20Installer%20Guide.md)
- [Dependency installer flags](Dependency%20Installer%20Script%20Flags.md)
- [Project index](Projects.md)
- [Motor and servo control](Project%20Control%20Motor%20Servo.md)
- [Line-following car](Project%20DIY%20Car.md)
- [Photoresistor](Project%20Photoresistor.md)
- [Plant monitor](Project%20Plant%20Monitor.md)
- [Text to speech](Project%20Say%20Something.md)
- [Security system](Project%20Security.md)
- [Ultrasonic distance](Project%20Ultrasonic.md)
- [Community adaptations](Community%20Tutorials.md)

### Tooling and deployment

- [PiCar-X Controller command reference](PiCar-X%20Controller%20Command%20Reference.md)
- [Jarvis and Gemini configuration](Jarvis%20and%20Gemini%20Configuration.md)
- [xWalk licence installation](xWalk%20Licence%20Installation.md)
- [xWalkTool overview](xWalkTool%20Overview.md)
- [Add a user to a Gerrit repository](Add%20a%20User%20to%20a%20Gerrit%20Repository.md)
- [Create Gerrit and configure xWalk CI](Create%20Gerrit%20and%20Configure%20xWalk%20CI.md)
- [Licence-key workflow](License%20Key%20Workflow.md)
- [xWalk licence tool](xWalk%20Licence%20Tool%20Guide.md)
- [xWalk environment loader](xWalk%20Environment%20Loader%20Guide.md)
- [Raspberry Pi deployment](Deployment%20Guide.md)

## Image: Robot HAT overview

![Robot HAT overview](../image/robot_hat_pic.png)

## Documentation authority

Public headers define exact signatures, ranges, ownership, exceptions, and
callback contracts. Module READMEs define build options and verification steps.
Update the matching page here whenever either source changes.
