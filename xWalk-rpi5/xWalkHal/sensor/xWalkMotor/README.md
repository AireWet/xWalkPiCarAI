# xWalkMotor

C++17 single and paired Robot HAT motor control for the xWalk Firmware HAL.

The module contains:

- `XWalkMotor`, which controls one motor in PWM-and-direction or dual-PWM mode.
- `XWalkMotors`, which assigns two existing motors to left and right roles.
- `XWalkMotorsConfiguration`, which carries role, reversal, watchdog, and injectable-clock data without
  owning persistent storage.

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
├── simulation/
│   ├── config/xHal_Rpi5CarMotorTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── hardware/src/xHal_Rpi5CarMotorHardwareTest.cpp
│   ├── include/xHal_Rpi5CarMotorTestSupport.h
│   └── src/
│       ├── xHal_Rpi5CarMotorTest.cpp
│       └── xHal_Rpi5CarMotorTestSupport.cpp
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
| `xHal_Rpi5CarMotorTestSupport.cpp` | Named in-memory I2C and GPIO callbacks |

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

Motor construction validates and binds dependencies but performs no PWM or GPIO operation. A single motor
must complete explicit, idempotent `initialize()` before speed or brake commands are accepted. Paired
`arm()` initializes both motors, establishes zero output on both sides, and remains disarmed if either
initialization fails. Shutdown before initialization is a no-op at the hardware boundary.

The C++ implementation validates speed before changing hardware. Commands must be finite and within -100.0
through 100.0 percent. Paired commands validate both values before changing either motor.

Paired motors start disarmed. Call `arm()` only after configuration and shutdown handling are ready. Movement
and electrically active dual-PWM braking are rejected while disarmed. Every non-zero movement or brake command
starts or refreshes a configurable watchdog deadline; `heartbeat()` refreshes an active command and reports
disarmed use through the ordinary exception boundary. Coordinators that must remain non-throwing use
`heartbeatSafely()` and handle its Boolean status. Expiry stops both channels and disarms the controller.
Invalid commands do not refresh the deadline. Tests can inject a fake
clock and disable the background worker, while deployment keeps the non-blocking condition-variable worker
enabled. The optional pre-thread-start callback is an injectable test boundary; production leaves it null.
It permits deterministic startup-failure coverage without exhausting process resources or creating a thread.

The watchdog stores the last valid refresh timestamp rather than a future
absolute deadline. A backward clock discontinuity is treated as immediate
expiry, while subtraction-based elapsed time remains defined for a large
forward jump. Neither discontinuity can preserve an old movement command.

`stopSafely()` is the non-throwing actuator-cleanup primitive. A dual-PWM motor attempts both PWM channels
even if the first write fails, and a paired controller attempts both motors independently. `stop()` uses the
same complete attempt but reports an incomplete shutdown to ordinary callers. Motor and paired-motor
destructors call the non-throwing operation as a final lifecycle safeguard. The safe path uses explicit
Boolean PWM and I2C status operations and contains no exception-handling statement.

This process-local watchdog cannot stop motors after `SIGKILL`, a kernel failure, power-path failure, or a
complete process stall. Initial physical testing therefore requires raised wheels, a reachable power cut-off,
and preferably an independent hardware watchdog. A destructor is a final fallback, not the normal stop path.

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
cmake -S xWalk-rpi5/xWalkHal/sensor/xWalkMotor -B xWalk-rpi5/xWalkHal/sensor/xWalkMotor/build-host -DXWALK_MOTOR_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkHal/sensor/xWalkMotor/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/sensor/xWalkMotor/build-host --output-on-failure
```

The Motor flag also enables PWM, I2C, and GPIO host dependency tests.
The tests inject failures into individual PWM writes and verify that later channels and the second motor still
receive zero-output attempts.

Reusable callback state lives in `xwalk::hal::test::motor`. Trace formatting
and output remain outside non-throwing fail-safe paths.

## Standalone host simulation and tracing

```bash
cmake -S xWalk-rpi5/xWalkHal/sensor/xWalkMotor/simulation -B build-motor-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-motor-simulation --parallel
./build-motor-simulation/xWalkMotorSimulation --trace RPI.enable
```

The simulation uses in-memory I2C and GPIO callbacks and never moves a motor.
Trace changes persist in generated XML and load automatically on the next run.
Enabled messages appear in the terminal and
`build-motor-simulation/log/xWalkMotorSimulation.log`.

## Hardware compilation without execution

```bash
cmake -S xWalk-rpi5/xWalkHal/sensor/xWalkMotor -B xWalk-rpi5/xWalkHal/sensor/xWalkMotor/build-rpi -DXWALK_MOTOR_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalk-rpi5/xWalkHal/sensor/xWalkMotor/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/sensor/xWalkMotor/build-rpi -N -L hardware
```

The final command only lists hardware tests. Do not execute them unless the vehicle is raised, wheels cannot
contact surrounding objects, power is controlled, and the Robot HAT motor-driver mode is confirmed.
