# xWalkCameraCapture

`xWalkCameraCapture` adapts one caller-owned `XWalkCamera` and one configured
JPEG destination to the image callback consumed by `XWalkVoiceActiveCar`.

The Agent owns no camera, process, device node, or image data. Each synchronous
capture overwrites the deployment-selected destination and returns its path to
the language-model pipeline.

## Host verification

```bash
cmake -S xWalkAgent/xWalkVision/xWalkCameraCapture -B build-camera-agent -DXWALK_CAMERA_CAPTURE_BUILD_HOST_TESTS=ON
cmake --build build-camera-agent --parallel
ctest --test-dir build-camera-agent --output-on-failure
```
