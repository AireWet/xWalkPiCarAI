# xWalkPwm standalone simulation

The executable exercises `XWalkPwm` through an in-memory I2C backend. It demonstrates address selection, timer
setup, duty-cycle output, and fail-safe output without opening `/dev/i2c-1` or accessing a Robot HAT.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkPwmSimulation --parallel
./build-host/xWalkPwmSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkPwmSimulation.log`.
