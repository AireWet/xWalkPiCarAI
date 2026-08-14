# xWalkAdxl345 standalone simulation

The executable exercises `XWalkAdxl345` through an in-memory I2C backend. It demonstrates measurement
configuration, discarded samples, signed conversion, and axis ordering without opening `/dev/i2c-1`.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkAdxl345Simulation --parallel
./build-host/xWalkAdxl345Simulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkAdxl345Simulation.log`.
