# xWalkCamera standalone simulation

The executable exercises `XWalkCamera` through an in-memory capture backend. It demonstrates capture forwarding and
bounded settings without opening a camera, starting a process, or creating an image file.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkCameraSimulation --parallel
./build-host/xWalkCameraSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkCameraSimulation.log`.
