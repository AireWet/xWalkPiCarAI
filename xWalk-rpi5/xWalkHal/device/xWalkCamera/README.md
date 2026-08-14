# xWalkCamera

`xWalkCamera` provides bounded synchronous JPEG capture through a device-free
core and an optional Linux backend.

The core stores a non-owning callback context, validates width, height, timeout,
and destination path, and owns no camera, process, or filesystem resource.

## Directory layout

```text
xWalkCamera/
├── core/include/
├── core/src/xHal_Rpi5CarCamera.cpp
├── hardware/include/
├── hardware/src/xHal_Rpi5CarCameraLinux.cpp
├── hardware/test/src/xHal_Rpi5CarCameraHardwareTest.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarCameraTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/include/xHal_Rpi5CarCameraTestSupport.h
├── test/src/
│   ├── xHal_Rpi5CarCameraTest.cpp
│   └── xHal_Rpi5CarCameraTestSupport.cpp
├── CMakeLists.txt
└── README.md
```

The Linux backend supports two deployment-selected connections:

| Connection | Linux provider | Default device |
| --- | --- | --- |
| `csi` | `rpicam-still` for the Raspberry Pi Camera Serial Interface | Camera selected by rpicam |
| `usb` | `ffmpeg` using Video4Linux2 | `/dev/video0` |

CSI is the Raspberry Pi camera connector. DSI is the display connector and is
not used for camera capture. Both providers are executed directly without a
shell. A successful operation must create a regular output file from four bytes
through 32 MiB.

## Host verification

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkCamera -B xWalk-rpi5/xWalkHal/device/xWalkCamera/build-host -DXWALK_CAMERA_BUILD_HOST_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/device/xWalkCamera/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/device/xWalkCamera/build-host --output-on-failure
```

Reusable callback state lives in `xwalk::hal::test::camera`. The host suite
also verifies persistent trace-selector behavior.

## Standalone host simulation and tracing

The simulation uses the public Camera API with an in-memory callback. It never
opens a camera, starts `rpicam-still` or `ffmpeg`, or creates an image file.

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkCamera/simulation -B build-camera-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-camera-simulation --parallel
./build-camera-simulation/xWalkCameraSimulation --trace RPI.enable
```

Selectors accept `RPI.<digits>.enable`, `RPI.enable`, `all.enable`, their
`.disable` counterparts, or a trace-update JSON path. Successful changes update
the generated XML and load automatically on the next run. Enabled traces appear
in the terminal and `build-camera-simulation/log/xWalkCameraSimulation.log`.

## Raspberry Pi verification

Install `rpicam-apps` for CSI or `ffmpeg` for USB. Configure the selected
hardware test, compile it, and list it before any approved physical execution:

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkCamera -B xWalk-rpi5/xWalkHal/device/xWalkCamera/build-rpi -DXWALK_CAMERA_BUILD_HARDWARE_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/device/xWalkCamera/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/device/xWalkCamera/build-rpi -N -L hardware
```
