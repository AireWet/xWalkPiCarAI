# xWalk Vision Agents

`xWalkVision` groups `xWalkComputerVision`, `xWalkFaceTracking`, `xWalkBullFight`, `xWalkTreasureHunt`,
`xWalkVideoRecording`, `xWalkVideoCar`, `xWalkCameraCapture`, and
`xWalkRoadUserSafety`, plus the bounded `xWalkVideoStreaming` MJPEG core.

Consumers can link `xWalk::AgentVision`. Portable coordinators remain separate from optional OpenCV and
physical-camera providers.

`xWalkRoadUserSafety` provides validated detector and classifier interfaces plus
fail-safe motion-stop behavior. Its deterministic scenarios do not constitute a
trained YOLO or Random Forest implementation; production models and physical
stop verification remain separate deployment work.

`xWalkVideoStreaming` validates and queues JPEG frames for multiple logical
clients with bounded memory and drop-oldest backpressure. It deliberately does
not contain a socket listener. Network transport, authentication, and physical
camera delivery remain deployment integration work.

Set `XWALK_AGENT_VISION_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_VISION_BUILD_HARDWARE_TESTS=ON` to compile the matching hardware-profile cases.
