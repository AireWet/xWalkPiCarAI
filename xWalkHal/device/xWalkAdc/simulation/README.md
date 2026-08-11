# xWalkAdc standalone simulation

The executable exercises `XWalkAdc` through an in-memory I2C backend. It demonstrates address selection, sample
acquisition, and voltage conversion without opening `/dev/i2c-1` or accessing a Robot HAT.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkAdcSimulation --parallel
./build-host/xWalkAdcSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkAdcSimulation.log`.
