# C++ API reference

The public C++ headers are authoritative. This index groups the xWalk HAL
modules by responsibility.

| Responsibility | Module documentation |
|---|---|
| Common types and free functions | [xWalk common library](../../../xWalkLibrary/common/README.md) |
| I2C | [xWalkI2c](../../../xWalkHal/interface/xWalkI2c/README.md) |
| GPIO | [xWalkGpio](../../../xWalkHal/interface/xWalkGpio/README.md) |
| ADC | [xWalkAdc](../../../xWalkHal/device/xWalkAdc/README.md) |
| PWM | [xWalkPwm](../../../xWalkHal/device/xWalkPwm/README.md) |
| Servo | [xWalkServo](../../../xWalkHal/device/xWalkServo/README.md) |
| Motors | [xWalkMotor](../../../xWalkHal/sensor/xWalkMotor/README.md) |
| Sensors | [Sensors and actuators](API%20Modules.md) |
| Robot coordination | [xWalkRobot](../../../xWalkHal/layer1/xWalkRobot/README.md) |
| Board services | [xWalkBoardControl](../../../xWalkHal/layer1/xWalkBoardControl/README.md) |
| Configuration | [xWalkConfig](../../../xWalkHal/interface/xWalkConfig/README.md) |
| Audio and speech | [Music](API%20Music.md) and [speech](API%20TTS.md) |
| Diagnostics and utilities | [Trace](API%20Basic%20Class.md) and [utilities](API%20Utils.md) |

Create platform backends and component objects in `main()`. Pass dependencies
by reference; consuming classes retain documented non-owning pointers.
