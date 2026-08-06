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

The OpenCV hardware provider captures from one configured absolute Linux camera device path,
uses HSV segmentation for source-compatible color selection, uses a configured
Haar cascade for frontal faces, and uses OpenCV QR decoding. Unlike Vilib's web
display, this provider does not start a network listener; the CLI reports
detection results explicitly and stores requested JPEG files locally.

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
