# xWalkConfig standalone simulation

The executable exercises `XWalkConfig` and `XWalkConfigStore` through their production filesystem behavior. It
writes only beneath `build-host/simulation-data` and does not inspect or modify deployed application configuration.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkConfigSimulation --parallel
./build-host/xWalkConfigSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkConfigSimulation.log`.
