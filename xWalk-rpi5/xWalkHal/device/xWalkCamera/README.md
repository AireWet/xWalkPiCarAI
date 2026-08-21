# xWalkCamera

`xWalkCamera` provides bounded synchronous JPEG capture and encoded live-frame
capture through a device-free core and optional Linux backends.

The core stores a non-owning callback context, validates width, height, timeout,
and destination path, and owns no camera, process, or filesystem resource.
`capture()` treats a backend failure as a required-operation error, while
`tryCapture()` returns `false` for callers that explicitly permit no image.

## Directory layout

```text
xWalkCamera/
├── core/include/
├── core/src/
│   ├── xHal_Rpi5CarCamera.cpp
│   └── xHal_Rpi5CarCameraStream.cpp
├── hardware/include/
├── hardware/src/
│   ├── xHal_Rpi5CarCameraLinux.cpp
│   └── xHal_Rpi5CarCameraStreamOpenCv.cpp
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

`XWalkCameraStream` validates and forwards camera start, stop, and JPEG-frame
capture through caller-owned callbacks. The optional `XWalkCameraStreamOpenCv`
backend accepts either `v4l2` with an exact `/dev/videoN` source for an ordinary
USB camera, or `libcamera` with the exact source `csi` for a Raspberry Pi CSI
camera. It owns the camera handle while Agent owns MJPEG transport policy.

The `libcamera` selection constructs this pipeline internally from the already
bounded numeric width and height settings:

```text
libcamerasrc ! video/x-raw,width=<width>,height=<height> ! videoconvert ! video/x-raw,format=BGR ! appsink drop=true max-buffers=1 sync=false
```

Configuration cannot supply GStreamer elements or pipeline text. CSI streaming
requires OpenCV with GStreamer support and the GStreamer `libcamerasrc` and
`videoconvert` plugins. V4L2 streaming remains independent of libcamera.

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

Install `rpicam-apps` for CSI still capture or `ffmpeg` for USB still capture.
CSI streaming additionally requires OpenCV GStreamer support and
`libcamerasrc`. Configure the selected hardware test, compile it, and list it
before any approved physical execution:

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkCamera -B xWalk-rpi5/xWalkHal/device/xWalkCamera/build-rpi -DXWALK_CAMERA_BUILD_HARDWARE_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/device/xWalkCamera/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/device/xWalkCamera/build-rpi -N -L hardware
```
