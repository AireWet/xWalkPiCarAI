# xWalkAdxl345

C++17 ADXL345 accelerometer module for the xWalk Firmware HAL.

The submodule configures an ADXL345 accelerometer, reads its X, Y, and Z data
registers, converts little-endian signed samples, and reports acceleration in
units of standard gravity.

## Directory layout

```text
xWalkAdxl345/
├── include/xHal_Rpi5CarAdxl345.h
├── include/xHal_Rpi5CarAdxl345Types.h
├── src/
│   ├── xHal_Rpi5CarAdxl345.cpp
│   └── xHal_Rpi5CarAdxl345Lifecycle.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarAdxl345TraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── hardware/src/xHal_Rpi5CarAdxl345HardwareTest.cpp
│   ├── include/xHal_Rpi5CarAdxl345TestSupport.h
│   └── src/
│       ├── xHal_Rpi5CarAdxl345Test.cpp
│       └── xHal_Rpi5CarAdxl345TestSupport.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarAdxl345.h` | Public driver API, ownership, and measurement contract |
| `xHal_Rpi5CarAdxl345Types.h` | Axis enumeration and fixed three-axis result type |
| `xHal_Rpi5CarAdxl345.cpp` | Configuration, register acquisition, and signed scaling |
| `xHal_Rpi5CarAdxl345Lifecycle.cpp` | Axis validation, register mapping, and lifecycle |
| `xHal_Rpi5CarAdxl345Test.cpp` | Driver behavior, validation, and trace-selector coverage |
| `xHal_Rpi5CarAdxl345TestSupport.cpp` | Reusable named in-memory I2C test callbacks |

## Composition

The application creates the hardware backend and callback interface before the
accelerometer. The ADXL345 stores a non-owning I2C pointer, so the I2C object
must outlive it.

```cpp
XWalkHal::XWalkI2cLinux backend;
XWalkHal::XWalkI2c i2c(&backend, XHAL_I2C_PROBE_CALLBACK(XWalkHal::XWalkI2cLinux),
    XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux),
    XHAL_I2C_READ_CALLBACK(XWalkHal::XWalkI2cLinux),
    XHAL_I2C_READ_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux));
XWalkHal::XWalkAdxl345 accelerometer(i2c);
const XWalkHal::adxl345values acceleration = accelerometer.read();
```

## Ported behavior

- Default seven-bit I2C address `0x53`
- X, Y, and Z data registers `0x32`, `0x34`, and `0x36`
- Data-format register value zero and power-control measurement value `0x08`
- Two-byte little-endian signed samples scaled by 256 counts per gravity
- First sample discarded before each returned axis value to preserve the established sensor contract
- Invalid axes, incomplete samples, invalid addresses, and missing register-read callbacks rejected

Register reads are performed as one backend transaction. This prevents another
execution context from changing the selected device or register between the
register address and returned bytes.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkHal/device/xWalkAdxl345 -B xWalkHal/device/xWalkAdxl345/build-host -DXWALK_ADXL345_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAdxl345/build-host --parallel
ctest --test-dir xWalkAdxl345/build-host --output-on-failure
```

The host configuration runs the ADXL345 suite and its I2C dependency suite. It
does not access a physical I2C device.

Reusable callback state lives in `xwalk::hal::test::adxl345` instead of the test
body. The host suite also verifies persistent trace-selector behavior.

## Standalone host simulation and tracing

The standalone simulation composes the public ADXL345 and I2C interfaces with
an in-memory backend. It never opens `/dev/i2c-1`.

```bash
cmake -S xWalkHal/device/xWalkAdxl345/simulation -B build-adxl345-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-adxl345-simulation --parallel
./build-adxl345-simulation/xWalkAdxl345Simulation --trace RPI.enable
```

Trace selectors accept `RPI.<digits>.enable`, `RPI.enable`, `all.enable`, their
`.disable` counterparts, or a trace-update JSON path. Successful changes update
the generated XML and load automatically on the next run. Enabled traces appear
in the terminal and `build-adxl345-simulation/log/xWalkAdxl345Simulation.log`.

## Hardware compile and test discovery

```bash
cmake -S xWalkHal/device/xWalkAdxl345 -B xWalkHal/device/xWalkAdxl345/build-rpi -DXWALK_ADXL345_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkAdxl345/build-rpi --parallel
ctest --test-dir xWalkAdxl345/build-rpi -N -L hardware
```

These commands compile and list hardware tests without executing them. Running
the hardware executable requires `/dev/i2c-1` access and an ADXL345 at `0x53`.
