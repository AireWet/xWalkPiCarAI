# xWalkGpio standalone simulation

The executable runs the public `XWalkGpio` API through the production Linux request-building logic. The `stub`
backend mirrors chip metadata, line configuration, reads, and writes without opening a physical GPIO controller.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DXWALK_GPIO_SIMULATION_BACKEND=stub -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkGpioSimulation --parallel
./build-host/xWalkGpioSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkGpioSimulation.log`.
