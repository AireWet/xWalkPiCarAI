# xWalkCamera

`xWalkCamera` provides bounded synchronous JPEG capture through a device-free
core and an optional Linux backend.

The core stores a non-owning callback context, validates width, height, timeout,
and destination path, and owns no camera, process, or filesystem resource.

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
cmake -S xWalkHal/xWalkCamera -B xWalkHal/xWalkCamera/build-host -DXWALK_CAMERA_BUILD_HOST_TESTS=ON
cmake --build xWalkHal/xWalkCamera/build-host --parallel
ctest --test-dir xWalkHal/xWalkCamera/build-host --output-on-failure
```

## Raspberry Pi verification

Install `rpicam-apps` for CSI or `ffmpeg` for USB. Configure the selected
hardware test, compile it, and list it before any approved physical execution:

```bash
cmake -S xWalkHal/xWalkCamera -B xWalkHal/xWalkCamera/build-rpi -DXWALK_CAMERA_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkHal/xWalkCamera/build-rpi --parallel
ctest --test-dir xWalkHal/xWalkCamera/build-rpi -N -L hardware
```
