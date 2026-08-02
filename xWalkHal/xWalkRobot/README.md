# xWalkRobot

C++17 coordinator for articulated multi-servo robots.

`XWalkRobot` registers up to 12 caller-created `XWalkServo` objects, loads calibration offsets through
`XWalkConfigStore`, applies origins and direction multipliers, interpolates synchronized movements, and stores
named multi-frame actions.

## Directory layout

```text
xWalkRobot/
├── include/xHal_Rpi5CarRobot.h
├── src/
│   ├── xHal_Rpi5CarRobotLifecycle.cpp
│   ├── xHal_Rpi5CarRobotMovement.cpp
│   └── xHal_Rpi5CarRobotPosition.cpp
├── test/src/xHal_Rpi5CarRobotTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarRobot.h` | Public bounded multi-servo robot API |
| `xHal_Rpi5CarRobotLifecycle.cpp` | Registration, offset parsing, and initialization |
| `xHal_Rpi5CarRobotMovement.cpp` | Interpolation and named action execution |
| `xHal_Rpi5CarRobotPosition.cpp` | Position transforms, calibration, and resets |
| `xHal_Rpi5CarRobotTest.cpp` | Simulated host behavior and validation coverage |

## Composition

The application creates dependencies in lifetime order and passes each project object by reference:

```cpp
XWalkConfigStore store("robot.config");
XWalkRobot robot(store, "walker");
robot.addServo(firstServo, 10.0);
robot.addServo(secondServo, -10.0);
robot.initialize();
```

The robot stores non-owning pointers. The configuration store and every registered servo must outlive it.
Registration is separate from initialization so a variable number of servos can be supplied by reference
without transferring ownership or constructing servo objects inside the robot.

## Ported behavior

- Initial logical angles and selectable initialization order
- Persisted `<robot-name>_servo_offset_list` values
- Offset clamping from -20 through 20 degrees
- Raw and origin/direction/offset-adjusted writes
- Speed- or BPM-based synchronized interpolation
- Maximum motion speed of 428 degrees per second
- Named action frames and repetition
- Calibration, reset, and soft-reset behavior

Malformed offset configuration and mismatched frame lengths are rejected with exceptions. At least one
interpolation step is used for very high BPM values, preventing division by zero.

## Host build and tests

```bash
cmake -S xWalkRobot -B xWalkRobot/build-host -DXWALK_ROBOT_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkRobot/build-host --parallel
ctest --test-dir xWalkRobot/build-host --output-on-failure
```

The host test uses callback-driven I2C simulation and writes configuration only below the module build
directory.

## Hardware compilation without execution

```bash
cmake -S xWalkRobot -B xWalkRobot/build-rpi -DXWALK_ROBOT_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkRobot/build-rpi --parallel
ctest --test-dir xWalkRobot/build-rpi -N -L hardware
```

The Robot option compiles Servo, PWM, and I2C hardware targets. It does not register a robot motion test,
because automatically moving an arbitrary articulated robot is not safe without application-specific limits.
