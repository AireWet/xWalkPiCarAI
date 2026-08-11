# Recorded vision test assets

This directory contains small, reviewed media fixtures for deterministic x86 tests. The clips are test inputs, not a model-accuracy dataset. Detection and classification expectations use explicit deterministic doubles unless a test says otherwise.

`recorded_scenarios` contains five-frame, 320-by-240 MJPEG AVI clips. Most are transformed excerpts from OpenCV `vtest.avi`; the bicycle clip is derived from Visitor7's Wikimedia Commons photograph. The manifest records immutable source revisions, source checksums, licenses, transformations and checksums for every committed result. Expected behavior is stored separately in `manifests/annotations.json`.

OpenCV `vtest.avi` is distributed under Apache-2.0. `Bicyclist_Crossing_the_Street.jpg` is Copyright Visitor7 and licensed CC BY-SA 3.0; the derived bicycle clip retains that license and attribution.

Fetch and regenerate the fixtures with:

```bash
xWalkAgent/xWalkVision/test/assets/fetch-assets.sh
```

Verify committed files without network access with:

```bash
xWalkAgent/xWalkVision/test/assets/verify-assets.sh
```

All downloaded files are treated as untrusted data. The script verifies source SHA-256 values before invoking FFmpeg and never executes source content. `camera_interruption` represents a transport failure injected after valid frames; `end_of_video` represents clean decoder exhaustion. The clips labeled poor-lighting, motion-blur and partial-occlusion are deterministic test transformations, not natural-condition accuracy samples.

