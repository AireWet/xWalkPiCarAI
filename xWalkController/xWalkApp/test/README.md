# xWalkApp Application Tests

This directory owns the GoogleTest application coverage registered by
`xWalkApp/CMakeLists.txt`. The sibling `xWalkHandler/test` directory owns
the direct in-memory `XWalkController` implementation test.

The `xWalkAppTest` executable combines direct, device-free unit coverage with
isolated host-CLI process checks. `xControllerAppTest.cpp` covers application
composition, shared DTOs, command selection, and the host executable.
`xControllerParsingTest.cpp` covers every typed request parser and result
formatter declared by `xControllerParsing.h`.

| GoogleTest case | Verification |
| --- | --- |
| `XWalkAppGroup.ControllerUsageFunction` | Calls the application usage free function directly |
| `XWalkAppGroup.ApplicationSupportDefaults` | Verifies callback context defaults and missing audio |
| `XWalkAppGroup.ApplicationOperationRequest` | Verifies operation reset and signal-stop transitions |
| `XWalkAppGroup.ControllerBootModes` | Verifies every command mapping and Base fallback |
| `XWalkAppGroup.ControllerRunnerDoctor` | Verifies boot-service dispatch through the Doctor path |
| `XWalkAppGroup.Help` | Accepts generated help without a hardware backend |
| `XWalkAppGroup.InvalidDeploymentConfiguration` | Rejects a relative deployment-configuration path |
| `XWalkAppGroup.HardwareCommandUnavailable` | Runs host-stub boot and rejects unavailable hardware |
| `XWalkAppParsingGroup.HexadecimalPayload` | Parses and formats bounded SPI bytes |
| `XWalkAppParsingGroup.VehicleRequests` | Parses vehicle and sensor request families |
| `XWalkAppParsingGroup.SpecializedRequests` | Parses sound, SPI, GPT-car, and calibration requests |
| `XWalkAppParsingGroup.OptionsAndNumbers` | Verifies shared option and numeric validation |
| `XWalkAppParsingGroup.ResultFormatting` | Verifies sensor, tracking, and detection output formatting |

CTest registers the complete GoogleTest executable as `xWalkAppHostTest` with
the `host;deployment;app` labels.

Build and run the host controller tests through the aggregate:

```bash
cmake -S xWalkController -B xWalkController/build-host -DXWALK_CLI_BUILD_HOST=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkController/build-host --parallel
ctest --test-dir xWalkController/build-host --output-on-failure -R '^(xWalkApp|xWalkController)'
```

These tests redirect child output to `/dev/null` and do not access physical
hardware. Raspberry Pi tests remain discovery-only until the required board
and Robot HAT safety checks are confirmed.
