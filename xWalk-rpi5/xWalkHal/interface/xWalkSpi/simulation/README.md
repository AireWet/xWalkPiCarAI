# xWalkSpi standalone simulation

The executable runs the public SPI API and Linux backend through an in-memory device stub. It demonstrates a
bounded four-byte transfer without opening `/dev/spidev*`.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DXWALK_SPI_SIMULATION_BACKEND=stub -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkSpiSimulation --parallel
./build-host/xWalkSpiSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkSpiSimulation.log`.
