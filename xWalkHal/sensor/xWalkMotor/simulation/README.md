# xWalkMotor standalone simulation

The executable composes the I2C, PWM, GPIO, and Motor APIs over an in-memory backend. It never opens
`/dev/i2c-*` or `/dev/gpiochip*` and cannot move a physical motor.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkMotorSimulation --parallel
./build-host/xWalkMotorSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkMotorSimulation.log`.
