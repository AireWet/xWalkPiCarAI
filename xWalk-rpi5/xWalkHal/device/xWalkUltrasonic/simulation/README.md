# xWalkUltrasonic standalone simulation

The executable exercises `XWalkUltrasonic` through an in-memory GPIO backend. It demonstrates trigger sequencing,
echo timing, and distance conversion without opening `/dev/gpiochip*` or driving physical pins.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkUltrasonicSimulation --parallel
./build-host/xWalkUltrasonicSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkUltrasonicSimulation.log`.
