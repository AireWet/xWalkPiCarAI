# `XWalkRobot`

`XWalkRobot` coordinates caller-created servo objects, named positions, and
multi-servo action frames. Persistent configuration remains in `XWalkConfig`.

## Public interface

See [xWalkRobot](../../../../xWalk-rpi5/xWalkHal/layer1/xWalkRobot/README.md) and
[`xHal_Rpi5CarRobot.h`](../../../../xWalk-rpi5/xWalkHal/layer1/xWalkRobot/include/xHal_Rpi5CarRobot.h).

Create servo objects in `main()` and pass validated non-owning pointers through
the robot configuration. Keep every servo and its PWM dependency alive longer
than the robot object.
