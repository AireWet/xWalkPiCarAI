# xWalkVideoStreaming

`xWalkVideoStreaming` provides the deterministic bounded core and pump-driven
non-blocking HTTP transport for MJPEG streaming. It validates loopback-only
defaults, frames JPEG data as HTTP
multipart records, assigns monotonic sequence numbers, and maintains a bounded
independent queue for each logical client. A slow client drops its oldest frame
without blocking capture, inference, motor control, or another client.

`XWalkMjpegHttpServer` opens a non-blocking IPv4 listener but starts no worker
thread. The owner calls `pumpMjpegHttpServer()` from a non-safety event loop.
The server exposes only `GET /stream`, `GET /health`, and `GET /status`; it has
no actuator endpoint. Request bytes, clients, pending output, timeouts and
per-client frame queues are bounded. Queue saturation drops the oldest frame
for that client. Header, idle, and slow-client deadlines use caller-provided
monotonic time.

`XWalkVideoStreaming` connects a caller-owned HAL `XWalkCameraStream` to that
transport. Raspberry Pi composition supplies the HAL
`XWalkCameraStreamOpenCv` backend, and the foreground Controller command pumps
capture and networking until SIGINT or SIGTERM requests graceful shutdown. The
camera-only cancellation path does not require a PiCar-X emergency-stop target.

The deployed CSI profile selects `libcamera` with source `csi`. The HAL builds
a fixed `libcamerasrc` GStreamer pipeline with NV12 source caps and validated
dimensions, then passes it directly to OpenCV; configuration cannot inject
pipeline text. A USB camera can instead select `v4l2` with an exact
`/dev/videoN` source.

The default bind is `127.0.0.1`. A non-loopback bind requires explicit
`allowExternalBind`, a non-empty secret-store reference, and a caller-provided
bearer-token authentication callback. The reference is not a credential and
the token is neither retained nor included in `/status`. No default credential
exists. IPv6 transport is not implemented; `::1` remains accepted by the
transport-independent queue configuration but is rejected when the IPv4 HTTP
listener attempts to start.

Camera loss clears every pending frame, rejects later publication, and prevents
stale imagery from being replayed. Shutdown and repeated start/stop operations
are deterministic and idempotent. Camera startup, first-frame, and HTTP startup
failures close every resource retained up to that point and emit trace errors.
Host tests also publish concurrently from two threads and verify that sequence
order and the configured queue bound hold.
Socket tests bind only to loopback and cover endpoint responses, multipart
boundaries, request limits, timeouts, client limits, port collision, camera
loss, connected-client shutdown and repeated stop. Raspberry Pi networking and
external-interface security remain unverified.

Build and run the host test:

```bash
cmake --preset host-debug
cmake --build build-host/cmake --target xWalkVideoStreamingTest
ctest --test-dir build-host/cmake -R xWalkVideoStreamingHostTest --output-on-failure
```

Loopback smoke examples after embedding and starting the server are:

```bash
curl --fail http://127.0.0.1:8080/health
```

```bash
ffplay http://127.0.0.1:8080/stream
```

Run the integrated Raspberry Pi command with:

```bash
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control video-stream
```

Open `http://127.0.0.1:8080/stream` on the Pi desktop. Forward the same
loopback-only stream to an SSH client with:

```bash
ssh -L 8080:127.0.0.1:8080 <pi-user>@<pi-address>
```
