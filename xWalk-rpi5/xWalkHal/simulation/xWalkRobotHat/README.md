# Robot HAT simulator

`xWalkRobotHatSimulation` is a host-only, device-free backend for the production
`XWalkI2c`, `XWalkGpio`, and `XWalkCamera` callback interfaces. It never opens a
Linux device, starts a camera process, contacts a network service, or reads wall
clock time. The normal Raspberry Pi build does not select it.

The simulator models the Robot HAT register space, PWM writes, eight ADC
channels, conventional A0/A1/A2 grayscale values, battery voltage, GPIO levels,
a configured ultrasonic distance, camera availability and encoded frame
sequences, and I2C address presence. Every operation receives a contiguous
sequence number and deterministic logical time. Configured logical delays
advance that time without sleeping. Tests can inspect an owned event snapshot
without physical timing.

Failures are injected by operation and target. An I2C-write target is the
register address, an I2C read/probe target is the device address, and a GPIO
target is the Linux line offset. Camera capture uses target zero. Failure counts
are consumed deterministically, making one-channel and repeated failures
reproducible.

Build and run the simulator tests from the workspace root:

```bash
cmake --preset host-debug
cmake --build build-host/cmake --target xWalkRobotHatSimulationTest xWalkPicarxSimulationTest
ctest --test-dir build-host/cmake -L simulation --output-on-failure
```

The PiCar-X simulator composition uses the Robot HAT v5 dual-PWM motor mapping:
P12/P13 for motor one and P14/P15 for motor two. It verifies software mapping and
failure behavior only. Physical port assignment, polarity, servo calibration,
electrical behavior, and Raspberry Pi 5 timing still require the raised-wheel
commissioning procedure on the actual kit.
