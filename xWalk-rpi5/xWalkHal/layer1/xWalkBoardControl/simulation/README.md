# xWalkBoardControl standalone simulation

The device-free executable exercises board reset, battery conversion, speaker priming, firmware acquisition, and
synthetic Robot HAT discovery through in-memory callbacks and a build-local device-tree fixture.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkBoardControlSimulation --parallel
./build-host/xWalkBoardControlSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkBoardControlSimulation.log`.
