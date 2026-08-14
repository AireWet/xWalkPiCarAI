# xWalk Connectivity Agents

`xWalkConnectivity` groups `xWalkAppControl` and `xWalkSpiTransfer` for externally controlled vehicle and
bounded SPI transaction workflows.

Consumers can link `xWalk::AgentConnectivity`. Network and Linux SPI ownership remains in optional providers
and the Raspberry Pi composition.

Set `XWALK_AGENT_CONNECTIVITY_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_CONNECTIVITY_BUILD_HARDWARE_TESTS=ON` to compile the hardware-profile cases.
