# xWalkServoZeroing

`xWalkServoZeroing` ports `example/servo_zeroing.py` as a hardware-independent
Agent. It commands Robot HAT servo channels 0–11 in order, applies the source
10-degree pulse for 100 milliseconds, returns each channel to zero for another
100 milliseconds, and then retains the process until cancellation.

The Raspberry Pi composition owns the MCU reset plus twelve PWM and Servo
objects. The Agent observes only synchronous angle, delay, and cancellation
callbacks. Delays are sliced to at most 20 milliseconds so SIGINT and SIGTERM
remain responsive. Physical execution can move every connected servo and must
be explicitly approved on a safe Robot HAT setup.

## Source layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarServoZeroingTypes.h` | Callback and configuration contracts |
| `include/xAgent_Rpi5CarServoZeroing.h` | Public coordinator contract |
| `src/xAgent_Rpi5CarServoZeroing.cpp` | Ordered pulse, zero, and idle behavior |
| `src/xAgent_Rpi5CarServoZeroingLifecycle.cpp` | Validation and cancellable timing |
| `test/src/xAgent_Rpi5CarServoZeroingTest.cpp` | Device-free source-order verification |
