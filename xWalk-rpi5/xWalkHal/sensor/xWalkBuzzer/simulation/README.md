# xWalkBuzzer standalone simulation

The executable composes the GPIO, I2C, PWM, active-buzzer, and passive-buzzer APIs over an in-memory backend. It
never opens `/dev/gpiochip*` or `/dev/i2c-*` and cannot produce physical sound.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkBuzzerSimulation --parallel
./build-host/xWalkBuzzerSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkBuzzerSimulation.log`.
