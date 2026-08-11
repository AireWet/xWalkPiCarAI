# xWalkAdc

C++17 Robot HAT analog-to-digital converter module for the xWalk Firmware HAL.

`XWalkAdc` accepts a caller-created `XWalkI2c` by reference and stores a non-owning pointer. It supports
numeric channels 0 through 7 and names `A0` through `A7`. Automatic addressing probes `0x14` then `0x15`.

`xWalkLineTracker` receives three caller-created `XWalkAdc` objects by reference and stores bounded
non-owning pointers ordered left, middle, and right.

## Directory layout

```text
xWalkAdc/
├── include/
├── src/
├── simulation/
│   ├── config/xHal_Rpi5CarAdcTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── hardware/src/xHal_Rpi5CarAdcHardwareTest.cpp
│   ├── include/xHal_Rpi5CarAdcTestSupport.h
│   └── src/
│       ├── xHal_Rpi5CarAdcTest.cpp
│       └── xHal_Rpi5CarAdcTestSupport.cpp
├── CMakeLists.txt
└── README.md
```

Reusable test state and callbacks live in the named
`xwalk::hal::test::adc` support component instead of the test body.

## Ported behavior

| Hardware contract | C++ behavior |
|---|---|
| Channel 0 through 7 or A0 through A7 | Preserved with validated constructor overloads |
| Hardware channel mapping `7 - channel` | Preserved |
| Command flag `0x10` | Preserved |
| Two-byte MSB-first sample | Preserved |
| Voltage conversion `value * 3.3 / 4095` | Preserved with named floating-point intermediates |
| No detected candidate address | Falls back to the default Robot HAT address `0x14` |

## Host build and tests

These tests use in-memory callbacks and do not open a physical I2C device.

```bash
cmake -S xWalkHal/device/xWalkAdc -B xWalkHal/device/xWalkAdc/build-host -DXWALK_ADC_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAdc/build-host --parallel
ctest --test-dir xWalkAdc/build-host --output-on-failure
```

The host suite also verifies persistent trace selector parsing and application.

## Standalone host simulation and tracing

The standalone simulation exercises the public ADC and I2C interfaces with an
in-memory backend. It never opens `/dev/i2c-1` or accesses a Robot HAT.

```bash
cmake -S xWalkHal/device/xWalkAdc/simulation -B build-adc-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-adc-simulation --parallel
./build-adc-simulation/xWalkAdcSimulation --trace RPI.enable
```

Trace selectors accept `RPI.<digits>.enable`, `RPI.enable`, `all.enable`, their
`.disable` counterparts, or a trace-update JSON path. Successful changes update
the generated XML and load automatically on the next run. Enabled traces appear
in the terminal and `build-adc-simulation/log/xWalkAdcSimulation.log`.

## Hardware compilation without execution

```bash
cmake -S xWalkHal/device/xWalkAdc -B xWalkHal/device/xWalkAdc/build-rpi -DXWALK_ADC_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkAdc/build-rpi --parallel
ctest --test-dir xWalkAdc/build-rpi -N -L hardware
```

The final command lists the hardware tests only. Do not execute them without a connected Robot HAT and a
safe analog input within the hardware voltage range.
