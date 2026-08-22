# xWalkIW

C++17 Protobuf definitions and gRPC services for xWalk I2C and Controller
requests.

The signal schema defines stable message and trace identifiers, while the enum
schema defines I2C and Controller operation selections. Separate request,
confirmation, and rejection schemas define the I2C payloads and mirror the plain Controller types declared in
`../xWalkLibrary/common/include/xWalkControllerConfigTypes.h`. The service
schema defines the typed Controller command RPCs.

## Directory layout

```text
xWalk-rpi5/xWalkIW/
├── auto-gen/
│   ├── include/                  Generated Protobuf and gRPC headers
│   └── src/                      Generated Protobuf and gRPC C++ sources
├── config/
│   ├── xHal_Rpi5CarGrpcSigReq.xml  Request signal mapping
│   ├── xHal_Rpi5CarGrpcSigCfm.xml  Confirmation signal mapping
│   └── xHal_Rpi5CarGrpcSigRej.xml  Rejection signal mapping
├── proto/
│   ├── xHal_Rpi5CarXIwSignal.proto
│   ├── xHal_Rpi5CarXIwEnum.proto
│   ├── xHal_Rpi5CarXIwMessageReq.proto
│   ├── xHal_Rpi5CarXIwMessageCfm.proto
│   ├── xHal_Rpi5CarXIwMessageRej.proto
│   └── xHal_Rpi5CarXIwGrpc.proto
├── CMakeLists.txt
└── README.md
```

The executable source generator lives at
`../../xWalkTool/py-agent/dev-tool/xHal_Rpi5CarIwGenerator`. Generated headers and sources
must not be edited manually. CMake validates the source schemas on every build.

## Protocol contract

### Request envelope

`XWalkSignal` carries a `sig_no`, a byte payload, and the originating
`XClientAddress`. The signal number must identify one concrete request or
configuration message through that message's `(cxx_signal)` option and the
request XML registry. The payload is the complete Protobuf wire encoding
produced by serializing that concrete message, including all of its arguments.

`XClientAddress` mirrors the C++ `xClientAddress` transport structure with a
mailbox ID, client address text, process-local xWalk index, and numeric module
type. A producer must ensure the address fits `XWALK_CLIENT_ADDRESS_SIZE` in
the receiving C++ character array, including the terminating null character.

The envelope does not have its own signal number. A receiver must reject an
unknown or mismatched `sig_no` before parsing `payload` as the selected message
type. An empty payload is valid only when the selected concrete message has no
encoded fields, such as `XWalkNoArgumentRequest`.

### Trace rejection message

`XWalkTraceRej` transports the error or warning values exposed by the trace
callback. Its `severity` is `XWALK_TRACE_SEVERITY_ERROR` or
`XWALK_TRACE_SEVERITY_WARNING`, matching the corresponding public
`XWalkTraceLevel` numeric value. Its `error_signal` mirrors the stable
`xwalk::hal::XWalkErrorSignalNumber` selector value, while `message` contains
the fully formatted callback text. Error selector values are independent of
platform POSIX signal numbers. This transport-only diagnostic has no request
signal binding.

### I2C messages

| Category | Message | Fields | Signal |
|---|---|---|---|
| Request | `XWalkI2cRequestPayload` | Operation, address, register, length, data | `0x1081` |
| Success | `XWalkI2cCfmPayload` | Data, responding, flag, message | `0x1082` |
| Rejection | `XWalkI2cRejPayload` | Data, responding, reason, detail, error selector | `0x1083` |

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
`0x208F`. Every message carries the same value through its typed `(cxx_signal)`
option and the request XML signal registry. Symbolic values follow the
`CXX_XWALK_<SHORT_NAME>_REQ`, `CXX_XWALK_<SHORT_NAME>_CFM`, and
`CXX_XWALK_<SHORT_NAME>_REJ` convention while preserving the existing numeric
wire values.

Every message whose name contains `Request` in
`xHal_Rpi5CarXIwMessageReq.proto` has a dedicated acknowledgement in
`xHal_Rpi5CarXIwMessageCfm.proto` and a dedicated rejection in
`xHal_Rpi5CarXIwMessageRej.proto`. The counterpart name replaces `Request`
with `Cfm` or `Rej`. Every confirmation carries a Boolean `flag` and a
human-readable `message`. Every rejection carries a numeric `reason`,
human-readable `detail`, and stable `XWalkErrorSignalNumber` selector identifying
the diagnostic category. Both counterpart categories also carry returned `data`
and a `responding` state.

Separate request, confirmation, and rejection XML registries map every message
to its stable signal. I2C retains `0x1081`, `0x1082`, and `0x1083`. Shared
Controller DTO confirmations use `0x2183` through `0x218E`,
and their rejections use `0x2283` through `0x228E`. Command confirmations use
`0x2100` through `0x2120`, and command rejections use `0x2200` through
`0x2220`.

The Controller enumeration values preserve the C++ declaration order. The
command field remains an unsigned integer because the Controller source owns
those signals as `XWALK_CNTRL_*_REQ` macros rather than a C++ enum. The macros
and their command-specific request messages share the contiguous signal range
`0x2000` through `0x2020`.

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
message in the `0x2000` through `0x2020` range. Command method names match the
message names without the `XWalk` prefix or `CommandRequest` suffix.

Successful Controller dispatch returns the command-specific `Cfm` message.
Invalid input, unavailable services, and execution failures return a non-OK
gRPC status with the matching `Rej` message available for structured details.
Runtime service objects are not serialized into the public transport contract.

The Vision service includes `VideoStream`, which accepts
`XWalkVideoStreamCommandRequest` (`0x2020`) and returns
`XWalkVideoStreamCommandCfm` (`0x2120`). Structured rejection uses
`XWalkVideoStreamCommandRej` (`0x2220`).

Proto3 optional presence is retained for fields whose C++ defaults are not the
scalar wire defaults. A protocol adapter must apply the documented C++ defaults
when those fields are absent and validate every value before invoking a handler.

## Validate and generate

Run these commands from the workspace root after changing a Protobuf or XML
input:

```bash
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarIwGenerator --check
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarIwGenerator --generate-cpp
```

Generation requires `protoc`, `grpc_cpp_plugin`, and the Protobuf development
schemas. The generator writes only below `auto-gen/include` and `auto-gen/src`.

## Build separately

Required dependencies are CMake, Python 3, a C++17 compiler, Protobuf development
files, gRPC development files, `protoc`, and `grpc_cpp_plugin`.

```bash
cmake -S xWalkIW -B xWalk-rpi5/xWalkIW/build-host -DXWALK_IW_BUILD_HOST_TESTS=ON
cmake --build xWalk-rpi5/xWalkIW/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkIW/build-host --output-on-failure
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
