# xWalkUserButton standalone simulation

The executable exercises a short active-low press through an in-memory GPIO backend. It does not open
`/dev/gpiochip*` or claim a physical line.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkUserButtonSimulation --parallel
./build-host/xWalkUserButtonSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkUserButtonSimulation.log`.
