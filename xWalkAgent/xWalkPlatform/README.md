# xWalk Platform Agents

`xWalkPlatform` contains `xWalkBoot`, the process composition root for the host stub and Raspberry Pi
hardware graph.

Consumers can link `xWalk::AgentPlatform`. Boot composes the other Agent targets but does not absorb their
implementations or verification boundaries.

Set `XWALK_AGENT_PLATFORM_BUILD_HOST_TESTS=ON` to run the Boot module's GoogleTest case independently.
Set `XWALK_AGENT_PLATFORM_BUILD_HARDWARE_TESTS=ON` to compile its hardware-profile case.
