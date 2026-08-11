# xWalkI2c standalone simulation

The executable runs the public I2C API and Linux backend through an in-memory device stub. It demonstrates probing,
register writes, fail-safe writes, and sequential reads without opening `/dev/i2c-*`.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DXWALK_I2C_SIMULATION_BACKEND=stub -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkI2cSimulation --parallel
./build-host/xWalkI2cSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkI2cSimulation.log`.
