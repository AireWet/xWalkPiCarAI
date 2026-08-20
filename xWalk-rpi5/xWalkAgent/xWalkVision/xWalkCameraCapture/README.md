# xWalkCameraCapture

`xWalkCameraCapture` adapts one caller-owned `XWalkCamera` and one configured
JPEG destination to the image callback consumed by `XWalkVoiceActiveCar`.

The Agent owns no camera, process, device node, or image data. Each synchronous
capture overwrites the deployment-selected destination and returns its path to
the language-model pipeline. The voice callback treats capture as optional: a
backend failure emits a warning and returns an empty path so the current request
continues without image input. Direct `capture()` calls remain strict.

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarCameraCapture.h` | Public strict and optional capture adaptation |
| `src/xAgent_Rpi5CarCameraCapture.cpp` | HAL camera forwarding and voice fallback behavior |
| `test/include/xAgent_Rpi5CarCameraCaptureTestSupport.h` | Reusable fake-camera test state and callback |
| `test/src/xAgent_Rpi5CarCameraCaptureTestSupport.cpp` | Fake-camera callback implementation |
| `test/src/xAgent_Rpi5CarCameraCaptureTest.cpp` | Strict success and optional failure verification |

## Host verification

```bash
cmake -S xWalk-rpi5/xWalkAgent/xWalkVision/xWalkCameraCapture -B build-camera-agent -DXWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS=ON
cmake --build build-camera-agent --parallel
ctest --test-dir build-camera-agent --output-on-failure
```
