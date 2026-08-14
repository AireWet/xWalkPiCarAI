# xWalk HAL sensor group

The sensor group contains sensor and actuator components:

- `xWalkLineTracker`
- `xWalkMotor`
- `xWalkLed`
- `xWalkBuzzer`

## Group interaction test

`test` builds `xWalkSensorGroupTest`. A small test-only response policy composes
the existing public contracts without adding production architecture. The suite
checks centred and corrective line responses, lost-line safe stop, LED and
buzzer critical indication, recovery, and paired-motor command validation.

ADC, PWM, I2C, and GPIO behavior uses deterministic in-memory callbacks. The
test never drives physical motors, LEDs, or buzzers.

From the repository root:

```bash
cmake -S . -B build-host/group-tests -DBUILD_TESTING=ON -DXWALK_BUILD_RPI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host/group-tests --target xWalkSensorGroupTest --parallel
build-host/group-tests/xWalkHal/sensor/test/xWalkSensorGroupTest
ctest --test-dir build-host/group-tests -L sensor-group --output-on-failure
```
