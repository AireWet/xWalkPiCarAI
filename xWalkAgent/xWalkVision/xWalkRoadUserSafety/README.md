# xWalkRoadUserSafety

`xWalkRoadUserSafety` is the hardware-independent safety boundary for a future
camera → YOLO detector → feature extraction → Random Forest classifier flow.
It validates the supported person, car, bicycle, bus, and motorbike classes;
rejects non-finite or out-of-range output; aggregates Safe, Warning, and
Dangerous decisions; and requests stop-and-disarm after danger or any pipeline
failure.

No trained detector, Random Forest, accuracy result, ONNX runtime, or model file
is included. Production integration must supply bounded callbacks and report
missing/corrupt models, invalid tensors, inference timeouts, camera loss, and
classifier failures through `XWalkRoadSafetyStatus`. A non-Ok status or provider
exception becomes `FailSafeStop`; recovery and motor rearming remain explicit
outside this module.

The host test includes a generated four-frame MJPEG AVI scenario. OpenCV reads
the complete file in order, a deterministic image detector extracts two person
rectangles, the normal feature and risk path transitions through Safe, Warning,
and Dangerous, and danger activates simulated red-LED and buzzer outputs. A
missing grayscale line and normal end-of-video both request stop-and-disarm.
Additional numeric scenarios cover multiple road users, low-confidence output,
camera loss, model failure, and invalid output. Tests use no physical device,
network, trained model, or committed binary fixture.

Build and run the focused test from the workspace root:

```bash
cmake --preset host-debug
cmake --build build-host/cmake --target xWalkRoadUserSafetyTest
ctest --test-dir build-host/cmake -R xWalkRoadUserSafetyHostTest --output-on-failure
```

This module is verified on x86 with deterministic doubles only. A production
backend, target ARM64 build, Raspberry Pi 5 camera path, model evaluation, and
physical PiCar-X stop response all remain required.
