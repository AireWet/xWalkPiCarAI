# xWalkComputerVision

`xWalkComputerVision` ports the interactive behavior of
`example/7.computer_vision.py` into a C++17 Agent submodule. The portable Agent
owns detector state and key mapping while caller-owned callbacks provide camera
acquisition, color and face detection, QR decoding, photograph storage, timing,
and cancellation.

## Behavior

- `q` captures a timestamped JPEG photograph;
- `1` through `6` select red, orange, yellow, green, blue, or purple detection;
- `0` disables color detection;
- `f` toggles frontal-face detection;
- `r` toggles QR decoding and reports newly changed non-empty QR text;
- `s` reports enabled color and face detector geometry;
- every key preserves the source's 500-millisecond delay using cancellable
  slices no longer than 20 milliseconds.

The OpenCV provider selects a configured V4L2 device, GStreamer pipeline, video
file, image sequence, or automatic OpenCV source. It uses HSV segmentation for source-compatible color selection, uses a configured
Haar cascade for frontal faces, and uses OpenCV QR decoding. Unlike Vilib's web
display, this provider does not start a network listener; the CLI reports
detection results explicitly and stores requested JPEG files locally.

The source is never implicitly fixed to `/dev/video0`. Device and file paths
must be absolute, pipelines reject line breaks, dimensions and frame rate must
be positive, and finite video end-of-file is distinguished from live-camera
failure. A validated 1 through 60000 millisecond read timeout is requested from
OpenCV, although support depends on the selected OpenCV backend.
Recorded MJPEG video exercises acquisition on x86 without a camera. The
`libcamera`/`rpicam` CSI path remains an unverified integration route, normally
through a reviewed GStreamer pipeline; no shell command is constructed.

OpenCV's FFmpeg and GStreamer adapters commonly honor the timeout property;
V4L2 support is backend and driver dependent. This repository therefore does
not yet claim that every live V4L2 read is forcibly cancellable. That limitation
must be verified on Raspberry Pi 5, and safety logic must treat acquisition
failure or timeout as a stop condition.

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarComputerVision.h` | Interactive state-machine contract |
| `include/xAgent_Rpi5CarComputerVisionTypes.h` | Colors, results, observations, and callbacks |
| `src/xAgent_Rpi5CarComputerVision.cpp` | Key mapping, observations, and QR change tracking |
| `src/xAgent_Rpi5CarComputerVisionLifecycle.cpp` | Validation, lifecycle, and bounded timing |
| `hardware/include/xAgent_Rpi5CarComputerVisionOpenCv.h` | OpenCV provider contract |
| `hardware/src/xAgent_Rpi5CarComputerVisionOpenCv.cpp` | Linux camera and detector implementation |
| `hardware/test/src/xAgent_Rpi5CarComputerVisionHardwareTest.cpp` | Opt-in physical-camera verification |
| `hardware/test/src/xAgent_Rpi5CarComputerVisionOpenCvTest.cpp` | Recorded-media backend and failure tests |
| `test/src/xAgent_Rpi5CarComputerVisionTest.cpp` | Deterministic callback-driven host test |

## Safety and privacy

Camera capture can record people, QR contents, and private surroundings. Use it
only with authorization, keep the camera indicator and storage destination
visible, and protect or remove saved images according to local policy. Hardware
tests remain opt-in and must not run during ordinary host verification.

The hardware test opens `/dev/video0`, processes live frames, writes one
temporary JPEG, and removes that file after verification. Discover it with
`ctest -N -L hardware`; run it only after camera placement, consent, and storage
safety have been confirmed.
