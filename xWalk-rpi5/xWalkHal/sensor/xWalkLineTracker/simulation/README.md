# xWalkLineTracker standalone simulation

The executable composes the I2C, ADC, grayscale, and line-tracker APIs over an in-memory bus without opening
`/dev/i2c-*`.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkLineTrackerSimulation --parallel
./build-host/xWalkLineTrackerSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkLineTrackerSimulation.log`.
