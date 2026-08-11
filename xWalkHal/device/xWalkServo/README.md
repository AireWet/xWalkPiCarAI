# xWalkServo

C++17 Robot HAT servo control for the xWalk Firmware HAL.

`XWalkServo` receives a caller-created `XWalkPwm` by reference and stores a
non-owning pointer to it. The application or test entry point creates objects in
dependency order, so the Servo is destroyed before its PWM dependency.
Multiple caller-created Servo objects can be registered by reference with
`xWalkRobot` for synchronized articulated motion and persisted calibration.

```cpp
xwalk::hal::XWalkI2cLinux backend;
xwalk::hal::XWalkI2c i2c(
    &backend,
    XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
    XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
    XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux));
xwalk::hal::XWalkPwmTimerState timerState;
xwalk::hal::XWalkPwm pwm(i2c, 0U, {}, timerState);
xwalk::hal::XWalkServo servo(pwm);
if (!servo.initialize())
{
    XWALK_HAL_ERROR(XWALK_RUNTIME, "Servo PWM initialization failed");
}
servo.setAngle(0.0);
```

Construction validates and stores the calibration but does not initialize PWM or
send a position command. `initialize()` is explicit, idempotent, and does not
move the servo. `setAngle()`, `setPulseWidthMicroseconds()`, and
`moveToSafePosition()` reject use before successful initialization. Applications
must request any centre or safe-position movement explicitly.

## Directory layout

```text
xWalkServo/
├── include/
│   └── xHal_Rpi5CarServo.h
├── src/
│   ├── xHal_Rpi5CarServo.cpp
│   └── xHal_Rpi5CarServoLifecycle.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarServoTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── hardware/src/
│   │   └── xHal_Rpi5CarServoHardwareTest.cpp
│   ├── include/
│   │   ├── xHal_Rpi5CarServoTestFunctions.h
│   │   └── xHal_Rpi5CarServoTestI2c.h
│   └── src/
│       ├── xHal_Rpi5CarServoTestMain.cpp
│       ├── xHal_Rpi5CarServoTestI2c.cpp
│       ├── xHal_Rpi5CarServoTestI2cLifecycle.cpp
│       ├── xHal_Rpi5CarServoTestInitialization.cpp
│       ├── xHal_Rpi5CarServoTestAngle.cpp
│       ├── xHal_Rpi5CarServoTestPulse.cpp
│       ├── xHal_Rpi5CarServoTestValidation.cpp
│       └── xHal_Rpi5CarServoTestTrace.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarServo.h` | Public Servo API and non-owning PWM dependency |
| `xHal_Rpi5CarServoLifecycle.cpp` | Dependency binding and timer initialization |
| `xHal_Rpi5CarServo.cpp` | Calibrated angle mapping, pulse validation, and count conversion |

## Ported behavior

| Servo contract | C++ behavior |
|---|---|
| PWM period 4095 | Preserved |
| 50 Hz frame | Rounded prescaler 352 |
| Angles below -90 or above +90 degrees | Clamped to -90 or +90 degrees |
| Pulse duration 500 through 2500 microseconds | Preserved |
| Floating timer count | Truncated through the existing PWM conversion |
| Non-finite command | Rejected with `std::invalid_argument` |

The default reference output counts are 102 at -90 degrees, 307 at 0 degrees,
and 511 at +90 degrees. Steering, camera pan, and camera tilt may instead supply
different validated minimum, centre, maximum, pulse calibration, and inversion values. Inversion mirrors an
angle around that servo's calibrated centre before applying its mechanical clamp. The Raspberry Pi vehicle
composition uses independent mechanical angle ranges: steering -30 through +30 degrees, pan -90 through +90
degrees, and tilt -35 through +65 degrees.
Servo construction performs no PWM write. Explicit initialization configures
the shared timer without selecting a position.

## Host build and tests

The host suite uses callback-driven in-memory I2C recording and never opens a
physical device. Run the following commands from the repository root.

```bash
cmake -S xWalkHal/device/xWalkServo -B xWalkHal/device/xWalkServo/build-host -DXWALK_SERVO_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkHal/device/xWalkServo/build-host --parallel
ctest --test-dir xWalkHal/device/xWalkServo/build-host --output-on-failure
```

The Servo flag also enables PWM and I2C host dependency tests. The Servo test
executable accepts `initialization`, `angle`, `pulse`, and `validation`
selectors, plus `trace` for persistent trace-selection verification.

## Standalone host simulation and tracing

The standalone simulation composes Servo, PWM, and I2C with an in-memory
backend. It never opens `/dev/i2c-1` or moves a physical servo.

```bash
cmake -S xWalkHal/device/xWalkServo/simulation -B build-servo-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-servo-simulation --parallel
./build-servo-simulation/xWalkServoSimulation --trace RPI.enable
```

Trace selectors accept `RPI.<digits>.enable`, `RPI.enable`, `all.enable`, their
`.disable` counterparts, or a trace-update JSON path. Successful changes update
the generated XML and load automatically on the next run. Enabled traces appear
in the terminal and `build-servo-simulation/log/xWalkServoSimulation.log`.

## Hardware compilation without execution

```bash
cmake -S xWalkHal/device/xWalkServo -B xWalkHal/device/xWalkServo/build-rpi -DXWALK_SERVO_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkHal/device/xWalkServo/build-rpi --parallel
ctest --test-dir xWalkHal/device/xWalkServo/build-rpi -N -L hardware
```

Enabling the Servo hardware target also compile-checks PWM and Linux I2C
hardware targets. The final command lists the tests only; it does not access
`/dev/i2c-1`. Do not execute hardware tests without a connected Robot HAT and a
mechanically safe servo setup.
