# xWalkPicarx

`xWalkPicarx` is a C++17 application coordinator. It delegates physical I/O to
the existing motor, servo, grayscale, ultrasonic, and configuration HAL modules and owns none of those
dependencies.

## Behavior

- loads and persists the upstream `picarx_*`, `line_reference`, and `cliff_reference` keys;
- exposes non-persistent servo-offset and motor-direction previews for calibration Agents;
- clamps steering to -30 through 30 degrees, pan to -90 through 90 degrees, and tilt to -35 through 65
  degrees;
- preserves the upstream non-zero motor scaling from 50 through 100 percent before applying calibration and
  the final-output safety limit;
- limits the final applied motor PWM magnitude to no more than 20 percent until the calibration workflow
  records successful raised-wheel motor-direction, steering-center, and motor-balance checks;
- provides latched emergency actuator suppression and scope-bound non-throwing paired-motor shutdown;
- reduces the inside wheel according to the current steering angle while using the HAL's logical motor
  direction;
- exposes raw grayscale data, threshold classification, cliff detection, and ultrasonic distance in
  centimeters;
- resets all logical actuator commands during `close()` and cancels ultrasonic interrupt registrations.

The upstream constructor resets the Robot HAT MCU before motor GPIO 5 is claimed. This port performs that step
in the RPi composition root so the temporary reset GPIO backend can be destroyed before the right motor claims
the same physical line. Applications must follow the same ordering before constructing `XWalkPicarx`.

`picarx_max_motor_output_percent` stores the post-verification deployment limit.
`picarx_calibration_verified` must be exactly `true` or `false`; a missing or false value keeps the effective
limit at the smaller of 20 percent and the configured deployment limit.
`picarx_motor_speed_calibration` stores one finite correction from -100 through
100 percentage points. Positive values reduce the left side and negative values
reduce the right side. Missing values default to zero; malformed and out-of-range
persisted values are rejected during construction.

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarPicarx.h` | Public coordinator contract and non-owning dependencies |
| `include/xAgent_Rpi5CarPicarxConfiguration.h` | Build-time config-path declaration and fallback |
| `include/xAgent_Rpi5CarPicarxSafetyGuard.h` | Scope-bound emergency-stop contract |
| `src/*Lifecycle.cpp` | Dependency binding and persisted configuration loading |
| `src/*Drive.cpp` | Motor scaling, steering, stop, reset, and close behavior |
| `src/*SafetyGuard.cpp` | Command-scope emergency-stop cleanup |
| `src/*Calibration.cpp` | Servo and motor calibration persistence |
| `src/*Sensing.cpp` | Grayscale, cliff, and ultrasonic delegation |
| `src/*Validation.cpp` | Finite numeric and persisted-list validation |
| `test/src/*Test.cpp` | Deterministic host behavior tests |
| `test/hardware/src/*HardwareTest.cpp` | Opt-in physical reset-and-stop smoke test |

Use `XWALK_PICARX_BUILD_HOST_TESTS=ON` for standalone host verification or
`XWALK_PICARX_BUILD_HARDWARE_TESTS=ON` for the Linux/RPi compile path. Hardware tests remain off by default.
Official CMake targets replace the source-visible relative configuration path
with the absolute `XWALK_PICARX_CONFIG_FILE` cache value.
