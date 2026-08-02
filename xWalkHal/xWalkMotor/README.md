# xWalkMotor

C++17 single and paired Robot HAT motor control for the xWalk Firmware HAL.

The module contains:

- `XWalkMotor`, which controls one motor in PWM-and-direction or dual-PWM mode.
- `XWalkMotors`, which assigns two existing motors to left and right roles.
- `XWalkMotorsConfiguration`, which carries role and reversal data without owning persistent storage.

All PWM, GPIO, and motor dependencies are passed by reference and stored as non-owning pointers. The
application creates dependencies in lifetime order and destroys the motor objects before their dependencies.

## Directory layout

```text
xWalkMotor/
├── include/xHal_Rpi5CarMotor.h
├── include/xHal_Rpi5CarMotors.h
├── src/
│   ├── xHal_Rpi5CarMotor.cpp
│   ├── xHal_Rpi5CarMotorLifecycle.cpp
│   ├── xHal_Rpi5CarMotors.cpp
│   └── xHal_Rpi5CarMotorsLifecycle.cpp
├── test/
│   ├── hardware/src/xHal_Rpi5CarMotorHardwareTest.cpp
│   └── src/xHal_Rpi5CarMotorTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarMotor.h` | Single-motor driver modes and public API |
| `xHal_Rpi5CarMotors.h` | Paired-motor configuration and coordinated-control API |
| `xHal_Rpi5CarMotorLifecycle.cpp` | Validation, dependency binding, and PWM initialization |
| `xHal_Rpi5CarMotor.cpp` | Signed speed, direction, stop, reversal, and brake behavior |
| `xHal_Rpi5CarMotorsLifecycle.cpp` | Paired validation, role binding, and lifecycle behavior |
| `xHal_Rpi5CarMotors.cpp` | Left/right role assignment and coordinated movement |

## Driver modes

| Command | PWM-and-direction mode | Dual-PWM mode |
|---|---|---|
| Forward | Primary PWM at speed; GPIO high | Forward PWM at speed; reverse PWM zero |
| Reverse | Primary PWM at speed; GPIO low | Forward PWM zero; reverse PWM at speed |
| Stop | Primary PWM zero | Both PWM inputs zero |
| Brake | Not supported | Both PWM inputs at 100 percent |

Direction reversal exchanges forward and reverse without changing the requested signed speed magnitude.

## Ported behavior

| Motor contract | C++ behavior |
|---|---|
| Default PWM frequency of 100 Hertz | Preserved |
| Signed speed selects direction | Preserved |
| Absolute speed controls duty cycle | Preserved |
| Motor modes 1 and 2 | Preserved as typed constructor overloads |
| Forward, backward, left-turn, right-turn, and stop | Preserved |
| Runtime direction reversal | Preserved |
| Mode-two electrical brake | Exposed as `brake()` |
| Last requested speed | `speed()` reports the last successful command |
| Fail-safe shutdown | Destructors independently attempt every available zero-output path |
| File-backed motor IDs and reversal values | Replaced by explicit caller-owned configuration |

The C++ implementation validates speed before changing hardware. Commands must be finite and within -100.0
through 100.0 percent. Paired commands validate both values before changing either motor.

`stopSafely()` is the non-throwing actuator-cleanup primitive. A dual-PWM motor attempts both PWM channels
even if the first write fails, and a paired controller attempts both motors independently. `stop()` uses the
same complete attempt but reports an incomplete shutdown to ordinary callers. Motor and paired-motor
destructors call the non-throwing operation as a final lifecycle safeguard. The safe path uses explicit
Boolean PWM and I2C status operations and contains no exception-handling statement.

## Persistent configuration boundary

`XWalkMotors` does not open files. The application supplies `XWalkMotorsConfiguration` and may save the value
returned by
`configuration()` through `XWalkConfigStore` from `xWalkConfig`. This keeps filesystem ownership outside the
module and allows deterministic host testing. The application converts persisted strings to validated typed
motor values
before constructing `XWalkMotors`.

## Host build and tests

The host suite uses callback-driven I2C and GPIO simulations and does not open physical devices.

```bash
cmake -S xWalkMotor -B xWalkMotor/build-host -DXWALK_MOTOR_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkMotor/build-host --parallel
ctest --test-dir xWalkMotor/build-host --output-on-failure
```

The Motor flag also enables PWM, I2C, and GPIO host dependency tests.
The tests inject failures into individual PWM writes and verify that later channels and the second motor still
receive zero-output attempts.

## Hardware compilation without execution

```bash
cmake -S xWalkMotor -B xWalkMotor/build-rpi -DXWALK_MOTOR_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkMotor/build-rpi --parallel
ctest --test-dir xWalkMotor/build-rpi -N -L hardware
```

The final command only lists hardware tests. Do not execute them unless the vehicle is raised, wheels cannot
contact surrounding objects, power is controlled, and the Robot HAT motor-driver mode is confirmed.
