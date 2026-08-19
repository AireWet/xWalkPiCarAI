# xWalk Vehicle Agents

`xWalkVehicle` groups movement and autonomous vehicle coordinators. It owns the
`xWalkPicarx`, `xWalkLineTracking`, `xWalkMoveExample`, `xWalkKeyboardControl`,
`xWalkObstacleAvoidance`, `xWalkCliffDetection`, and `xWalkSelfDrive` module directories.

Consumers can link `xWalk::AgentVehicle`. Every child remains an independent CMake target with its existing
public headers, ownership rules, and host or hardware verification boundary.

Set `XWALK_AGENT_VEHICLE_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_VEHICLE_BUILD_HARDWARE_TESTS=ON` to compile the matching hardware-profile cases.

## Tracing

Every Vehicle child module owns a registered `RPIAGENT` lifecycle or bounded-action trace. Use the authoritative
[Agent trace table](../README.md#runtime-tracing) to select one child. Repeated control-loop samples and
fail-safe cleanup remain trace-free.
