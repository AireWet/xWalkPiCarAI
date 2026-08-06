# xWalkKeyboardControl

`xWalkKeyboardControl` ports the actuator behavior from upstream
`example/3.keyboard_control.py`. It maps `w`, `s`, `a`, and `d` to 80-percent
movement pulses and maps `i`, `k`, `j`, and `l` to five-degree camera steps
bounded between minus 30 and plus 30 degrees.

## Layout and responsibilities

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarKeyboardControl.h` | Public key handling and cleanup API |
| `include/xAgent_Rpi5CarKeyboardControlTypes.h` | Callback aliases and key result type |
| `src/xAgent_Rpi5CarKeyboardControl.cpp` | Key mapping, camera bounds, pulse, and centering behavior |
| `src/xAgent_Rpi5CarKeyboardControlLifecycle.cpp` | Dependency validation, cancellation, and destruction |
| `test/src/xAgent_Rpi5CarKeyboardControlTest.cpp` | Device-free Agent behavior verification |

The Agent observes caller-owned `XWalkPicarx` and scheduling callbacks. Terminal
input remains in `xWalkController`; use `xwalk-picarx-control keyboard-control`, enter
one key per prompt, and use `q` to finish. SIGINT, SIGTERM, cancellation, and
destruction stop the motors. Normal completion also centers all three servos and
preserves the upstream final 200-millisecond delay.

The host test uses only an in-memory HAL graph. The Raspberry Pi command
physically drives the car and moves steering and camera servos, so it requires a
clear travel area and an approved Robot HAT setup.
