# xWalk Vision Agents

`xWalkVision` groups `xWalkComputerVision`, `xWalkFaceTracking`, `xWalkBullFight`, `xWalkTreasureHunt`,
`xWalkVideoRecording`, `xWalkVideoCar`, and `xWalkCameraCapture`.

Consumers can link `xWalk::AgentVision`. Portable coordinators remain separate from optional OpenCV and
physical-camera providers.

Set `XWALK_AGENT_VISION_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_VISION_BUILD_HARDWARE_TESTS=ON` to compile the matching hardware-profile cases.
