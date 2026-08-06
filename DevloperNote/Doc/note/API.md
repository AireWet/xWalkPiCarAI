# C++ API reference

The public C++ headers are authoritative. This index groups the xWalk HAL
modules by responsibility.

| Responsibility | Module documentation |
|---|---|
| Common types and free functions | [xWalk common library](../../../xWalkLibrary/common/README.md) |
| I2C | [xWalkI2c](../../../xWalkHal/xWalkI2c/README.md) |
| GPIO | [xWalkGpio](../../../xWalkHal/xWalkGpio/README.md) |
| ADC | [xWalkAdc](../../../xWalkHal/xWalkAdc/README.md) |
| PWM | [xWalkPwm](../../../xWalkHal/xWalkPwm/README.md) |
| Servo | [xWalkServo](../../../xWalkHal/xWalkServo/README.md) |
| Motors | [xWalkMotor](../../../xWalkHal/xWalkMotor/README.md) |
| Sensors | [Sensors and actuators](API%20Modules.md) |
| Robot coordination | [xWalkRobot](../../../xWalkHal/xWalkRobot/README.md) |
| Board services | [xWalkBoardControl](../../../xWalkHal/xWalkBoardControl/README.md) |
| Configuration | [xWalkConfig](../../../xWalkHal/xWalkConfig/README.md) |
| Audio and speech | [Music](API%20Music.md) and [speech](API%20TTS.md) |
| Diagnostics and utilities | [Trace](API%20Basic%20Class.md) and [utilities](API%20Utils.md) |

Create platform backends and component objects in `main()`. Pass dependencies
by reference; consuming classes retain documented non-owning pointers.
