# xWalkIW

C++17 Protobuf definitions for xWalk Controller requests and the `xWalkI2c`
gRPC service.

The enum schema defines stable I2C and Controller selections. The message schema
defines the I2C payloads and mirrors the plain Controller types declared in
`../xWalkLibrary/common/include/xWalkControllerConfigTypes.h`. The service
schema defines one generic I2C request-confirmation RPC.

## Directory layout

```text
xWalkIW/
├── auto-gen/
│   ├── include/                  Generated Protobuf and gRPC headers
│   └── src/                      Generated Protobuf and gRPC C++ sources
├── config/
│   └── xHal_Rpi5CarGrpcSig.yml  Stable signal mapping
├── proto/
│   ├── xHal_Rpi5CarXIwEnum.proto
│   ├── xHal_Rpi5CarXIwMessage.proto
│   └── xHal_Rpi5CarXIwGrpc.proto
├── CMakeLists.txt
└── README.md
```

The executable source generator lives at
`../xWalkTool/python/xHal_Rpi5CarIwGenerator`. Generated headers and sources
must not be edited manually. CMake validates the source schemas on every build.

## Protocol contract

### I2C service

| Category | Message | Fields | Signal |
|---|---|---|---|
| Request | `XWalkI2cRequestPayload` | Operation, address, register, length, data | `0x1081` |
| Success | `XWalkI2cConfirmPayload` | Data, responding | `0x1082` |
| Rejection | `XWalkI2cRejectPayload` | Reason, detail | `0x1083` |

The schema uses package `xwalk.iw.v1` and proto3 defaults. Public request fields
use address `1`, register address `2`, length `3`, data `4`, and operation `5`.
The three message signal identifiers are stable protocol values.

`XWalkI2cService.Execute` is the only RPC. It accepts
`XWalkI2cRequestPayload` and returns `XWalkI2cConfirmPayload` after a successful
probe, direct read, register read, or register write. Rejections use a non-OK
gRPC status, with `XWalkI2cRejectPayload` available for structured details.

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
option and the YAML signal registry.

The Controller enumeration values preserve the C++ declaration order. The
command field remains an unsigned integer because the Controller source owns
those signals as `XWALK_CNTRL_*_REQ` macros rather than a C++ enum. The macros
and their command-specific request messages share the contiguous signal range
`0x2000` through `0x201F`.

Proto3 optional presence is retained for fields whose C++ defaults are not the
scalar wire defaults. A protocol adapter must apply the documented C++ defaults
when those fields are absent and validate every value before invoking a handler.

## Validate and generate

Run these commands from the workspace root after changing a proto or YAML input:

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
| `xWalkIwSchemaCheck` | Validates Protobuf and YAML source consistency |
| `xWalkIwGrpc` | Builds the generated public messages and gRPC service API |
| `xWalkIW` | Provides the public interface target |
| `xWalk::IW` | Provides the namespaced target alias |

## Test and hardware safety

`xWalkIW` performs no hardware access. Its host test validates the schema and
signal mapping; compilation verifies generated C++ compatibility. It does not
open an I2C device or validate physical Robot HAT hardware.
