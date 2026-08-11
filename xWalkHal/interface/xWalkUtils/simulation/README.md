# xWalkUtils standalone simulation

The executable runs `XWalkUtils` through an in-memory callback backend. It mirrors output, volume, command,
executable, network, and username operations without changing the mixer, executing commands, redirecting
descriptors, or querying host network and user state.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkUtilsSimulation --parallel
./build-host/xWalkUtilsSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkUtilsSimulation.log`.
