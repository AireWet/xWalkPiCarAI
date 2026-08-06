# xWalk Servo and Motor Calibration

`xWalkServoMotorCalibration` is the Agent-level port of the supplied
`picar-x/example/1.cali_servo_motor.py` helper. It stores pending servo offsets and motor directions around a
caller-owned `XWalkPicarx`; Linux devices, terminal input, and operator policy remain outside the module.

The port provides:

- source-compatible servo reset and nine-position servo test sequences;
- pending steering, camera-pan, and camera-tilt offsets bounded to ±20 degrees;
- motor-direction preview and source-compatible 30-percent forward testing;
- explicit persistence of all pending values through `save()`;
- cancellation polling in slices no longer than 20 milliseconds;
- best-effort motor cleanup on cancellation and destruction.

The CLI composes this Agent for `xwalk-picarx-control calibrate servo-motor` and retains prompts, motor-balance
configuration, raised-wheel verification, and final persistence confirmation.

Run the device-free host verification from the workspace root:

```bash
cmake --build --preset host-debug --target xWalkServoMotorCalibrationTest --parallel
ctest --preset host-debug -R xWalkServoMotorCalibrationHostTest
```

Physical execution moves all three servos and can run both motors. Raise the wheels for motor verification,
clear every servo mechanism, and keep an immediate power-disconnect path available.
