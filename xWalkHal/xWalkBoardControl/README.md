# xWalkBoardControl

C++17 embedded-oriented Robot HAT board-services module combining board control,
Linux device-tree discovery, and firmware-version reporting.

The module retains three focused classes:

- `XWalkBoardControl` coordinates MCU reset, battery sensing, GPIO output, and
  speaker-power sequencing through caller-owned hardware dependencies.
- `XWalkDevice` discovers Robot HAT information from a Linux firmware device tree.
- `XWalkFirmwareInfo` reads the Robot HAT firmware version through injected I2C.

Each class remains in separate headers and implementation files while all three
are provided by the single `xWalkBoardControl` static library.

## Directory layout

```text
xWalkBoardControl/
├── include/
│   ├── xHal_Rpi5CarBoardControl.h
│   ├── xHal_Rpi5CarBoardControlTypes.h
│   ├── xHal_Rpi5CarDevice.h
│   ├── xHal_Rpi5CarDeviceTypes.h
│   ├── xHal_Rpi5CarFirmwareInfo.h
│   └── xHal_Rpi5CarFirmwareInfoTypes.h
├── src/
│   ├── xHal_Rpi5CarBoardControl.cpp
│   ├── xHal_Rpi5CarBoardControlLifecycle.cpp
│   ├── xHal_Rpi5CarDevice.cpp
│   ├── xHal_Rpi5CarDeviceLifecycle.cpp
│   ├── xHal_Rpi5CarFirmwareInfo.cpp
│   └── xHal_Rpi5CarFirmwareInfoLifecycle.cpp
├── test/
│   ├── hardware/src/
│   └── src/
├── CMakeLists.txt
└── README.md
```

## Composition and ownership

Create device discovery, Linux backends, GPIO interfaces, I2C, ADC, and board
control in `main()`. Device information selects the board-specific speaker GPIO
before `XWalkBoardControl` is constructed. Firmware information receives I2C by
reference and stores only a non-owning pointer.

```cpp
XWalkDevice device;
const XWalkDeviceInformation& deviceInformation = device.information();
XWalkFirmwareInfo firmwareInformation(i2c);
XWalkBoardControl boardControl(mcuResetGpio, speakerEnableGpio, batteryAdc,
    &audioBackend, &primeSpeakerOutput);
```

All injected GPIO, ADC, I2C, and callback contexts must outlive their consumers.
Direct dependency access requires external serialization.

## Board-control behavior

- MCU reset drives the reset line low and high using bounded delays.
- Battery acquisition reads ADC channel A4 and applies the hardware divider ratio.
- Speaker enable uses the board-specific active GPIO and a required prime callback.
- Speaker disable drives the selected power-control GPIO inactive.
- No shell command is used for GPIO or audio control.

## Device-discovery behavior

- Candidate direct-child device-tree node names must contain `hat`.
- Robot HAT v5 UUID `9daeea78-0000-076e-0032-582369ac3e02` is recognized.
- Product, vendor, identifier, version, and UUID properties are retained.
- Robot HAT v5 selects speaker GPIO 12 and motor mode 2.
- Missing supported UUIDs retain Robot HAT v4 defaults: GPIO 20 and motor mode 1.
- `refresh()` replaces prior information only after selected properties validate.

## Firmware-information behavior

- I2C address `0x14` is probed before `0x15`.
- Exactly three bytes are read from firmware register `0x05`.
- Major, minor, and patch components are represented as unsigned values.
- Firmware text uses `major.minor.patch` formatting.
- Robot HAT compatibility version `2.5.5` remains static metadata.

## Host build and tests

```bash
cmake -S xWalkBoardControl -B xWalkBoardControl/build-host -DXWALK_BOARD_CONTROL_BUILD_HOST_TESTS=ON
cmake --build xWalkBoardControl/build-host --parallel
ctest --test-dir xWalkBoardControl/build-host --output-on-failure
```

Host tests use in-memory hardware backends and a synthetic device tree. They do
not access physical GPIO, I2C, audio, or `/proc/device-tree` resources.

## Hardware compile and test discovery

```bash
cmake -S xWalkBoardControl -B xWalkBoardControl/build-rpi -DXWALK_BOARD_CONTROL_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkBoardControl/build-rpi --parallel
ctest --test-dir xWalkBoardControl/build-rpi -N -L hardware
```

These commands compile and list the board-control, firmware-read, and read-only
device-discovery tests without executing them. Physical hardware tests require
the correct Robot HAT safety setup and must not be run during normal verification.
