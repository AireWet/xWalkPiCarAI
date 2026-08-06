# xWalk Calibration Agents

`xWalkCalibration` groups the `xWalkGrayscaleCalibration`, `xWalkServoMotorCalibration`, and
`xWalkServoZeroing` module directories.

Consumers can link `xWalk::AgentCalibration`. Calibration remains explicit and bounded; physical sensor,
motor, and servo verification still requires the documented hardware safety approval.

Set `XWALK_AGENT_CALIBRATION_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_CALIBRATION_BUILD_HARDWARE_TESTS=ON` to compile the hardware-profile cases.
