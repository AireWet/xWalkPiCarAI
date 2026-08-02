# `XWalkServo`

`XWalkServo` translates angle and pulse-duration commands into a caller-created
`XWalkPwm` output.

## Public interface

See [xWalkServo](../../../xWalkHal/xWalkServo/README.md) and
[`xHal_Rpi5CarServo.h`](../../../xWalkHal/xWalkServo/include/xHal_Rpi5CarServo.h).

The default configuration uses 50 hertz, a 4095-count period, angles from -90
to 90 degrees, and pulse widths from 500 to 2500 microseconds. Confirm the
mechanical range before commanding an attached servo.
