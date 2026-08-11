# xWalk HAL device group

The device group contains hardware device abstractions:

- `xWalkPwm`
- `xWalkAdc`
- `xWalkServo`
- `xWalkAdxl345`
- `xWalkUltrasonic`
- `xWalkCamera`
- `xWalkUserButton`

## Group interaction test

`test` builds `xWalkDeviceGroupTest`. The suite complements the individual
module tests by checking Servo-to-PWM-to-I2C mapping, ADC and accelerometer
decoding, GPIO pulse measurement, camera capture, active-low button behavior,
boundary values, and lower-level failure propagation.

I2C, GPIO, clocks, and camera capture use deterministic in-memory callbacks.
The test does not access a Raspberry Pi, Robot HAT, or camera device.

From the repository root:

```bash
cmake -S . -B build-host/group-tests -DBUILD_TESTING=ON -DXWALK_BUILD_RPI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host/group-tests --target xWalkDeviceGroupTest --parallel
build-host/group-tests/xWalkHal/device/test/xWalkDeviceGroupTest
ctest --test-dir build-host/group-tests -L device-group --output-on-failure
```
