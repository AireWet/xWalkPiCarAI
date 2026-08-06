# xWalk CLI GoogleTest runner

`xCliGoogleTest` is the independent process entry point for CLI unit tests.
Existing unit-test entry points remain in their owning modules and are compiled
with renamed functions. Each one executes in an isolated child process so an
assertion failure becomes one GoogleTest failure.

## Layout and responsibilities

| Path | Responsibility |
| --- | --- |
| `config/test_config.xml` | Complete enabled Controller inventory by functional group |
| `config/hardware_test_config.xml` | Complete disabled Controller inventory for hardware selection |
| `include/xCliTestConfig.h` | XML selection types and loader contract |
| `src/xCliTestConfig.cpp` | Strict TinyXML2 validation |
| `src/main.cpp` | Unit-test process entry point, registration, filtering, and child isolation |

## Registered group inventory

| Suite | Responsibility |
| --- | --- |
| `XWalkControllerGroup` | Controller unit, generic command sequence, and help |
| `XWalkAgentPlatformGroup` | Deployment Doctor |
| `XWalkAgentCalibrationGroup` | Servo zeroing and calibration |
| `XWalkAgentVehicleGroup` | Movement, driving, sensing, line tracking, and self-drive |
| `XWalkAgentVisionGroup` | Camera, detection, tracking, recording, and video driving |
| `XWalkAgentConnectivityGroup` | App control and SPI |
| `XWalkAgentMediaGroup` | Sound and background music |
| `XWalkAgentVoiceGroup` | Voice, GPT, storytelling, vision talk, and online LLM cases |

The XML configuration must contain every compiled Controller suite and case
exactly once. The sequence runner owns a corresponding strict inventory under
[`../xSequenceTest/config`](../xSequenceTest/config). A standard
`--gtest_filter=<pattern>` argument overrides XML enablement for one run.

## Host verification

From the repository root:

```bash
cmake --fresh --preset host-debug
cmake --build --preset host-debug --parallel
./build-host/cmake/xCliGoogleTest
ctest --preset host-debug
```

The runner uses only in-memory callbacks and simulated HAL composition. It does
not open a microphone, speaker endpoint, camera, network, or physical bus. The
hardware configuration is disabled and is not registered. Runnable examples
are not part of the CLI GoogleTest inventory and are not GoogleTest cases.

The independent [`../xSequenceTest`](../xSequenceTest) executable owns bounded
controller-to-HAL command sequences. The `Host quality` workflow runs both
executables as separate required steps for every GCC/Clang Debug/Release matrix
entry.
