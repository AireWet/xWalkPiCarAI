# xWalkServo standalone simulation

The executable exercises `XWalkServo` through in-memory PWM and I2C composition. It demonstrates timer setup, angle
conversion, and pulse output without opening `/dev/i2c-1` or moving a physical servo.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkServoSimulation --parallel
./build-host/xWalkServoSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkServoSimulation.log`.
