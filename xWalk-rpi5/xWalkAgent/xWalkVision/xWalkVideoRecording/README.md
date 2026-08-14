# xWalkVideoRecording

`xWalkVideoRecording` ports `example/9.record_video.py` into a callback-driven
Agent. Camera and encoder ownership remain with the provider. The Agent keeps
the upstream 800-millisecond warm-up, `Q` start/pause/continue transitions, `E`
stop behavior, timestamped AVI naming, and 100-millisecond post-key delay.

The portable host test uses only in-memory callbacks. A separate OpenCV host
test programmatically generates a small MJPEG AVI, opens it as finite input,
checks missing-source and configuration failures, and verifies idempotent
shutdown without opening a camera device.

The OpenCV provider accepts the same configured V4L2, GStreamer, video-file,
image-sequence, and automatic source families as computer vision. It validates
source type, absolute path requirements, pipeline line breaks, resolution,
frame rate, a 1 through 60000 millisecond best-effort read timeout, and output
directory before opening resources. Finite video/image-sequence exhaustion is
retained separately from live-camera failure. This provides recorded-video
testing without treating it as CSI-camera verification.

OpenCV timeout and cancellation support varies by backend. FFmpeg and
GStreamer may honor the requested timeout; a V4L2 driver can still block inside
`VideoCapture::read()`. The current provider joins its worker during shutdown,
so bounded V4L2 shutdown remains a Raspberry Pi 5 verification item and is not
claimed from x86 recorded-media results.

The hardware-labelled test records approximately one second from `/dev/video0`
into `/tmp/xwalk-video-recording-hardware-test`. Build discovery is safe, but
run it only after confirming the correct camera, destination, privacy, and disk
capacity. Ordinary host verification never executes it.
