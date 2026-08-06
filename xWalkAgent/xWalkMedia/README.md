# xWalk Media Agents

`xWalkMedia` contains `xWalkSoundBackgroundMusic`, which coordinates sound effects and background music
through the shared music HAL.

Consumers can link `xWalk::AgentMedia`. Audio device ownership remains in the HAL and Raspberry Pi
composition rather than this Agent group.

Set `XWALK_AGENT_MEDIA_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_MEDIA_BUILD_HARDWARE_TESTS=ON` to compile the matching hardware-profile cases.
