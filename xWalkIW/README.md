# xWalkIW

C++17 Protobuf definitions and gRPC services for xWalk I2C and Controller
requests.

The enum schema defines stable I2C and Controller selections. Separate request,
confirmation, and rejection schemas define the I2C payloads and mirror the
plain Controller types declared in
`../xWalkLibrary/common/include/xWalkControllerConfigTypes.h`. The service
schema defines the typed Controller command RPCs.

## Directory layout

```text
xWalkIW/
├── auto-gen/
│   ├── include/                  Generated Protobuf and gRPC headers
│   └── src/                      Generated Protobuf and gRPC C++ sources
├── config/
│   ├── xHal_Rpi5CarGrpcSigReq.xml  Request signal mapping
│   ├── xHal_Rpi5CarGrpcSigCfm.xml  Confirmation signal mapping
│   └── xHal_Rpi5CarGrpcSigRej.xml  Rejection signal mapping
├── proto/
│   ├── xHal_Rpi5CarXIwEnum.proto
│   ├── xHal_Rpi5CarXIwMessageReq.proto
│   ├── xHal_Rpi5CarXIwMessageCfm.proto
│   ├── xHal_Rpi5CarXIwMessageRej.proto
│   └── xHal_Rpi5CarXIwGrpc.proto
├── CMakeLists.txt
└── README.md
```

The executable source generator lives at
`../xWalkTool/python/xHal_Rpi5CarIwGenerator`. Generated headers and sources
must not be edited manually. CMake validates the source schemas on every build.

## Protocol contract

### I2C messages

| Category | Message | Fields | Signal |
|---|---|---|---|
| Request | `XWalkI2cRequestPayload` | Operation, address, register, length, data | `0x1081` |
| Success | `XWalkI2cCfmPayload` | Data, responding, flag, message | `0x1082` |
| Rejection | `XWalkI2cRejPayload` | Data, responding, reason, detail | `0x1083` |

The schema uses package `xwalk.iw.v1` and proto3 defaults. Public request fields
use address `1`, register address `2`, length `3`, data `4`, and operation `5`.
The three message signal identifiers are stable protocol values.

The I2C request, confirmation, and rejection payloads remain available as
transport DTOs. `XWalkControllerService.Execute` accepts
`XWalkI2cRequestPayload` and returns `XWalkI2cCfmPayload`; rejected operations
use a non-OK gRPC status with `XWalkI2cRejPayload` available for details.

### Controller messages

The following Protobuf messages mirror the Controller-owned structures. They
are transport DTOs and do not contain callbacks, service pointers, hardware
objects, or runtime state.

| C++ structure | Protobuf message | Signal |
|---|---|---|
| `XWalkAppConfig` | `XWalkAppConfig` | `0x2081` |
| `XWalkControllerApplicationArguments` | `XWalkControllerApplicationArguments` | `0x2082` |
| `XWalkControllerCommandRequest` | `XWalkControllerCommandRequest` | `0x2083` |
| `XWalkNoArgumentRequest` | `XWalkNoArgumentRequest` | `0x2084` |
| `XWalkLifecycleRequest` | `XWalkLifecycleRequest` | `0x2085` |
| `XWalkMoveRequest` | `XWalkMoveRequest` | `0x2086` |
| `XWalkTurnRequest` | `XWalkTurnRequest` | `0x2087` |
| `XWalkCameraRequest` | `XWalkCameraRequest` | `0x2088` |
| `XWalkSensorRequest` | `XWalkSensorRequest` | `0x2089` |
| `XWalkSelfDriveRequest` | `XWalkSelfDriveRequest` | `0x208A` |
| `XWalkSpiRequest` | `XWalkSpiRequest` | `0x208B` |
| `XWalkGptCarRequest` | `XWalkGptCarRequest` | `0x208C` |
| `XWalkCalibrationRequest` | `XWalkCalibrationRequest` | `0x208D` |
| `XWalkSoundRequest` | `XWalkSoundRequest` | `0x208E` |
| `XWalkServoCalibrationConfig` | `XWalkServoCalibrationConfig` | `0x208F` |

The shared Controller DTO signals occupy the contiguous range `0x2081` through
`0x208F`. Every message carries the same value through its `(xwalkSignal)`
option and the request XML signal registry.

Every message whose name contains `Request` in
`xHal_Rpi5CarXIwMessageReq.proto` has a dedicated acknowledgement in
`xHal_Rpi5CarXIwMessageCfm.proto` and a dedicated rejection in
`xHal_Rpi5CarXIwMessageRej.proto`. The counterpart name replaces `Request`
with `Cfm` or `Rej`. Every confirmation carries a Boolean `flag` and a
human-readable `message`. Every rejection carries a numeric `reason` and
human-readable `detail`. Both counterpart categories also carry returned
`data` and a `responding` state.

Separate request, confirmation, and rejection XML registries map every message
to its stable signal. I2C retains `0x1081`, `0x1082`, and `0x1083`. Shared
Controller DTO confirmations use `0x2183` through `0x218E`,
and their rejections use `0x2283` through `0x228E`. Command confirmations use
`0x2100` through `0x211F`, and command rejections use `0x2200` through
`0x221F`.

The Controller enumeration values preserve the C++ declaration order. The
command field remains an unsigned integer because the Controller source owns
those signals as `XWALK_CNTRL_*_REQ` macros rather than a C++ enum. The macros
and their command-specific request messages share the contiguous signal range
`0x2000` through `0x201F`.

### Controller services

The gRPC API groups requests by the same functionality boundaries used by the
Controller handlers and Agent modules.

| Service | Responsibility |
|---|---|
| `XWalkCtrlI2cService` | Generic I2C transaction |
| `XWalkCtrlVehicleService` | Movement, sensing, and autonomous driving |
| `XWalkCtrlVisionService` | Camera and computer-vision operations |
| `XWalkCtrlVoiceService` | Speech and language-model operations |
| `XWalkCtrlMediaService` | Sound and background music |
| `XWalkCtrlConnectivityService` | SPI and mobile-app control |
| `XWalkCtrlCalibrationService` | Servo zeroing and calibration |
| `XWalkCtrlPlatformService` | Unknown commands, help, and diagnostics |

Together the command services expose one unary RPC for every command-specific
message in the `0x2000` through `0x201F` range. Command method names match the
message names without the `XWalk` prefix or `CommandRequest` suffix.

Successful Controller dispatch returns the command-specific `Cfm` message.
Invalid input, unavailable services, and execution failures return a non-OK
gRPC status with the matching `Rej` message available for structured details.
Runtime service objects are not serialized into the public transport contract.

Proto3 optional presence is retained for fields whose C++ defaults are not the
scalar wire defaults. A protocol adapter must apply the documented C++ defaults
when those fields are absent and validate every value before invoking a handler.

## Validate and generate

Run these commands from the workspace root after changing a Protobuf or XML
input:

```bash
xWalkTool/python/xHal_Rpi5CarIwGenerator --check
xWalkTool/python/xHal_Rpi5CarIwGenerator --generate-cpp
```

Generation requires `protoc`, `grpc_cpp_plugin`, and the Protobuf development
schemas. The generator writes only below `auto-gen/include` and `auto-gen/src`.

## Build separately

Required dependencies are CMake, Python 3, a C++17 compiler, Protobuf development
files, gRPC development files, `protoc`, and `grpc_cpp_plugin`.

```bash
cmake -S xWalkIW -B xWalkIW/build-host -DXWALK_IW_BUILD_HOST_TESTS=ON
cmake --build xWalkIW/build-host --parallel
ctest --test-dir xWalkIW/build-host --output-on-failure
```

| CMake target | Responsibility |
|---|---|
| `xWalkIwSchemaCheck` | Validates Protobuf and XML source consistency |
| `xWalkIwGrpc` | Builds the generated public messages and gRPC service API |
| `xWalkIW` | Provides the public interface target |
| `xWalk::IW` | Provides the namespaced target alias |

## Test and hardware safety

`xWalkIW` performs no hardware access. Its host test validates the schema and
signal mapping; compilation verifies generated C++ compatibility. It does not
open an I2C device or validate physical Robot HAT hardware.
