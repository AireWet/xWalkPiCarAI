# `XWalkMotor` and `XWalkMotors`

`XWalkMotor` controls one motor using the PWM and direction dependencies selected
for the board mode. `XWalkMotors` coordinates two non-owning motor pointers.

## Public interface

See [xWalkMotor](../../../../xWalk-rpi5/xWalkHal/sensor/xWalkMotor/README.md) and its two public class headers.

Speed commands use the documented range from -100 to 100 percent. Validate
coordinated commands before changing either motor. Apply zero output before
destroying dependencies or changing hardware configuration.
