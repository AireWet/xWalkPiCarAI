# `XWalkPwm` and `XWalkPwmTimerState`

`XWalkPwm` controls one MCU PWM channel. `XWalkPwmTimerState` coordinates the
frequency and period shared by channels assigned to one hardware timer.

## Public interface

See [xWalkPwm](../../../../xWalk-rpi5/xWalkHal/device/xWalkPwm/README.md),
[`xHal_Rpi5CarPwm.h`](../../../../xWalk-rpi5/xWalkHal/device/xWalkPwm/include/xHal_Rpi5CarPwm.h), and
[timer-state header](../../../../xWalk-rpi5/xWalkHal/device/xWalkPwm/include/xHal_Rpi5CarPwmTimerState.h).

Create one timer-state object before all PWM channels that use it. A frequency
change that conflicts with an active sibling channel is rejected. Pulse values
must remain within the configured period.
