# Voice-active-car configuration assets

`voice-active-car.jpg` is the deterministic host image used by the Rolly
voice-active-car sequence. It is copied from the upstream Robot HAT camera
documentation asset at `robot-hat/docs/source/img/camera.jpg`.

CMake exposes this directory through
`XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY`. Host verification reads the JPEG
from that configured directory and validates its file signature before passing
the path to the simulated language model. Raspberry Pi captures remain writable
and continue to use the deployment-controlled `camera_output` path.
