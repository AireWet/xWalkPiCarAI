# xWalkObstacleAvoidance

`xWalkObstacleAvoidance` ports the decision behavior from upstream
`example/4.avoiding_obstacles.py`. One caller-supplied ultrasonic sample selects
straight forward motion at or above 40 centimeters, a 100-millisecond right
turn from 20 through less than 40 centimeters, or a 500-millisecond reverse-left
action below 20 centimeters. All movement uses 50-percent requested power.

Unlike the Python source, non-finite, zero, timeout, and invalid-pulse results
stop the motors instead of entering the less-than-20-centimeter reverse branch.

## Layout and responsibilities

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarObstacleAvoidance.h` | Public bounded decision and stop API |
| `include/xAgent_Rpi5CarObstacleAvoidanceTypes.h` | Callback aliases and decision result type |
| `src/xAgent_Rpi5CarObstacleAvoidance.cpp` | Distance bands, vehicle commands, and safety behavior |
| `src/xAgent_Rpi5CarObstacleAvoidanceLifecycle.cpp` | Dependency validation and cancellation polling |
| `test/src/xAgent_Rpi5CarObstacleAvoidanceTest.cpp` | Device-free threshold and cleanup verification |

The CLI command is `xwalk-picarx-control avoid-obstacles start`; use SIGINT or
SIGTERM to stop the foreground loop. `avoid-obstacles stop` performs an immediate
motor stop. Physical execution requires a clear reverse path, raised first-run
verification where appropriate, and an approved Raspberry Pi/Robot HAT setup.
