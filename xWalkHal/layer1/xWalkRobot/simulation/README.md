# xWalkRobot standalone simulation

The device-free executable exercises registration, initialization, calibration, offset persistence, reset,
interpolation, and action playback through an in-memory servo bus and build-local configuration file.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkRobotSimulation --parallel
./build-host/xWalkRobotSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkRobotSimulation.log`.
