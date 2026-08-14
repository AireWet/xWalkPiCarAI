# xWalkLed standalone simulation

The executable composes the GPIO, I2C, PWM, single-color LED, and RGB LED APIs over an in-memory backend. It never
opens `/dev/gpiochip*` or `/dev/i2c-*` and cannot change a physical light output.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkLedSimulation --parallel
./build-host/xWalkLedSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkLedSimulation.log`.
