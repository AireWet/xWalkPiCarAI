# xWalkSpi

`xWalkSpi` provides bounded, hardware-independent full-duplex SPI transactions
and an optional Linux `spidev` owner. The core stores one non-owning callback
context and never opens a platform device.

One transfer accepts 1 through 256 bytes. The Linux backend configures standard
SPI mode 0 through 3, a positive clock frequency, and 1 through 32 bits per
word. Linux owns chip-select assertion for the complete ioctl transaction.

## Layout

```text
xWalkSpi/
├── core/
│   ├── include/
│   │   ├── xHal_Rpi5CarSpi.h
│   │   └── xHal_Rpi5CarSpiTypes.h
│   └── src/
│       ├── xHal_Rpi5CarSpi.cpp
│       └── xHal_Rpi5CarSpiLifecycle.cpp
├── hardware/
│   ├── include/
│   │   ├── xHal_Rpi5CarSpiDevice.h
│   │   ├── xHal_Rpi5CarSpiDeviceLinux.h
│   │   └── xHal_Rpi5CarSpiLinux.h
│   ├── src/
│   │   ├── xHal_Rpi5CarSpiDeviceLinux.cpp
│   │   ├── xHal_Rpi5CarSpiLinux.cpp
│   │   └── xHal_Rpi5CarSpiLinuxLifecycle.cpp
│   └── test/src/
│       └── xHal_Rpi5CarSpiLinuxHardwareTest.cpp
├── simulation/
│   ├── config/
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md
```

| Path | Responsibility |
|---|---|
| `core/include` | Configuration, callback types, and public SPI contract |
| `core/src` | Validation, callback forwarding, lifecycle, and diagnostics |
| `hardware/include` | Linux backend and injectable device-operation contracts |
| `hardware/src` | Production spidev system calls, configuration, and transfer logic |
| `simulation/include` | Arguments, handler, host mirror, factory, and runner configuration |
| `simulation/src` | Standalone runner and build-selected device composition |
| `test/include` | Google Test fixture declaration and owned test dependencies |
| `test/src` | Individual SPI and simulation-argument tests |
| `hardware/test/src` | Opt-in one-byte physical transfer smoke test |

`XWalkSpiLinux` owns the descriptor and observes one injected
`XWalkSpiDevice`. Production composition uses `XWalkSpiDeviceLinux`; host
composition uses `XWalkSpiHostStub`. This lets host tests execute the real
configuration and `SPI_IOC_MESSAGE` request-building path without opening
`/dev/spidev*`.

Core, Linux, host-stub, test, and simulation operations use filtered xWalk
trace identifiers. Validation failures also emit unfiltered error and numeric
assertion diagnostics before the normal exception boundary.

## Host verification

Run from the repository root:

```bash
cmake -S xWalkHal/interface/xWalkSpi -B xWalkHal/interface/xWalkSpi/build-host -DXWALK_SPI_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkHal/interface/xWalkSpi/build-host --parallel
ctest --test-dir xWalkHal/interface/xWalkSpi/build-host --output-on-failure
```

The host suite uses the production Linux backend with the device-free host
mirror. It verifies configuration, transmit and receive bytes, payload bounds,
callback and response-length validation, and trace-option parsing.

## Standalone simulation

The stub is the safe default. It mirrors Linux device operations in memory:

```bash
cmake -S xWalkHal/interface/xWalkSpi/simulation -B xWalkHal/interface/xWalkSpi/simulation/build-simulation -DXWALK_SPI_SIMULATION_BACKEND=stub
cmake --build xWalkHal/interface/xWalkSpi/simulation/build-simulation --target xWalkSpiSimulation --parallel
./xWalkHal/interface/xWalkSpi/simulation/build-simulation/xWalkSpiSimulation
```

The runner accepts `--help` or one `--trace` selector. New traces are disabled,
while successful selectors persist in XML and load on later runs. See
[`simulation/README.md`](simulation/README.md) for the
complete runner and hardware-selection contract.

## Raspberry Pi compilation

```bash
cmake -S xWalkHal/interface/xWalkSpi -B xWalkHal/interface/xWalkSpi/build-rpi -DXWALK_SPI_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkHal/interface/xWalkSpi/build-rpi --parallel
ctest --test-dir xWalkHal/interface/xWalkSpi/build-rpi -N -L hardware
```

The hardware test transmits one zero byte. Do not execute it until the selected
`/dev/spidev*` node, chip select, peripheral protocol, wiring, voltage, and power
state have been reviewed and approved.
