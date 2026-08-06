# xWalkVideoRecording

`xWalkVideoRecording` ports `example/9.record_video.py` into a callback-driven
Agent. Camera and encoder ownership remain with the provider. The Agent keeps
the upstream 800-millisecond warm-up, `Q` start/pause/continue transitions, `E`
stop behavior, timestamped AVI naming, and 100-millisecond post-key delay.

The host test uses only in-memory callbacks and never opens a camera or writes
video. Physical recording is opt-in through the Raspberry Pi composition.

The hardware-labelled test records approximately one second from `/dev/video0`
into `/tmp/xwalk-video-recording-hardware-test`. Build discovery is safe, but
run it only after confirming the correct camera, destination, privacy, and disk
capacity. Ordinary host verification never executes it.
