# xWalkCommon

Workspace-level C++17 declarations and standard-library includes shared by
xWalk HAL, agent, CLI, and future application modules.

Public header:

```cpp
#include "xHal_Rpi5CarCommon.h"
```

Reusable non-member production functions are declared in:

```cpp
#include "xHal_Rpi5CarCommonFunctions.h"
```

They live in `namespace xwalk::hal::common`. Module source files call these
functions with the `common::` qualifier instead of defining anonymous or
module-local free helper functions. Class-specific behavior remains a class
method in its owning module.

Reusable filesystem operations are declared separately in:

```cpp
#include "xHal_Rpi5CarFileFunctions.h"
```

These functions live directly in `namespace xwalk::hal`, matching types such as
`uint32`. Modules call `filesystemEntryExists()`, `filesystemStatus()`,
`replaceFilesystemPermissions()`, and the other wrappers instead of calling
`std::filesystem` operations directly. File-open modes, permission options, and
line extraction are exposed through typed constants and `readFileLine()`.
Complete binary reads use `readFileContents()`, and direct-child directory
enumeration uses `listFilesystemEntryNames()`.

All standard-library headers used by the HAL modules are centralized in:

```cpp
#include "xHal_Rpi5CarStandardHeaders.h"
```

Submodules include their project header and do not include standard-library
headers directly.

Host tests verify rejected operations without exception handlers through:

```cpp
#include "xHal_Rpi5CarTestFunctions.h"
```

`xwalk::hal::test::expectFailure()` runs the operation in an isolated Linux
child process and asserts that it does not complete successfully.

Project namespace and shared data-type definitions are declared in:

```cpp
#include "xHal_Rpi5CarTypes.h"
```

The common type vocabulary includes aliases for standard-library data types
used outside the common boundary. These include containers, exceptions,
streams, filesystem paths and metadata, file-open modes, permission options,
synchronization objects, and error codes. Modules use aliases such as
`stringvector`, `logicerror`, `filesystempath`, `outputfilestream`,
`fileopenmode`, and `errorcode` instead of spelling the underlying standard type
directly. Standard-library qualification remains inside the common boundary.

Exception creation is isolated behind project macros declared in:

```cpp
#include "xHal_Rpi5CarExceptions.h"
```

The uppercase function-like macros `XHAL_THROW_INVALID_ARGUMENT`,
`XHAL_THROW_INVALID_ARGUMENT_DETAIL`, `XHAL_THROW_OUT_OF_RANGE`,
`XHAL_THROW_OUT_OF_RANGE_DETAIL`, `XHAL_THROW_LOGIC_ERROR`,
`XHAL_THROW_RUNTIME_ERROR`, and `XHAL_THROW_RUNTIME_ERROR_DETAIL` keep standard
exception facilities inside the common header.

Linux system headers needed only by optional hardware backends are centralized
in `xHal_Rpi5CarLinuxHeaders.h`. Host-only HAL sources do not include this
header.

Reusable context-to-backend I2C callback bridges include
`XHAL_I2C_PROBE_CALLBACK`, `XHAL_I2C_WRITE_REGISTER_CALLBACK`,
`XHAL_I2C_READ_CALLBACK`, and `XHAL_I2C_READ_REGISTER_CALLBACK` in
`xHal_Rpi5CarCommon.h`. Hardware backends bind these macros to their public
device-operation methods.

Standard math operations are exposed as uppercase function-like macros in
`xHal_Rpi5CarMath.h`. Submodules use these macros and contain no direct `std::`
references. Power and sine calculations use `XHAL_POWER` and `XHAL_SINE`.

The `xWalkCommon` CMake target is a header-only interface library. Linking it
propagates the common include directory and the C++17 requirement.

Shared hardware constants use uppercase, project-prefixed macros. PWM register
and clock definitions use the `XHAL_RPI5CAR_PWM_` prefix. Servo angle, pulse,
frame, frequency, and period definitions use the `XHAL_RPI5CAR_SERVO_` prefix
and include units in their names. Ultrasonic sound-speed, timing, attempt, and
status-result definitions use the `XHAL_RPI5CAR_ULTRASONIC_` prefix and remain
in `xHal_Rpi5CarCommon.h` rather than the sensor module header.
Line-tracker channel, threshold, adaptive-reference, weighting, position, and
rounding definitions use the `XHAL_RPI5CAR_LINE_TRACKER_` prefix in the same
shared header.
ADXL345 address, register, axis, sample-size, sign, and scaling definitions use
the `XHAL_RPI5CAR_ADXL345_` prefix in that shared header.
RGB LED channel indices, connection selectors, packed-color masks, shifts, and
conversion scales use the `XHAL_RPI5CAR_RGB_LED_` prefix in that shared header.
Buzzer duty-cycle and playback-duration conversion definitions use the
`XHAL_RPI5CAR_BUZZER_` prefix in that shared header.
LED blink-count, transition timing, duration conversion, and worker polling
definitions use the `XHAL_RPI5CAR_LED_` prefix in that shared header.
User-button active level, polling interval, long-press limits, and timing
conversion definitions use the `XHAL_RPI5CAR_USER_BUTTON_` prefix.
Music PCM format, theory, MIDI range, volume, rounding, and compatibility
definitions use the `XHAL_RPI5CAR_MUSIC_` prefix.
Speaker task-count, chunk-size, pause-polling, and invalid-slot definitions use
the `XHAL_RPI5CAR_SPEAKER_` prefix.
Device-tree root, node, property, UUID, board-pin, motor-mode, and hexadecimal
limits use the `XHAL_RPI5CAR_DEVICE_` prefix.
Utility volume limits, lazy-reader timing, and millisecond conversion definitions
use the `XHAL_RPI5CAR_UTILS_` prefix.

## Clean the build

Run these commands from the repository root. When `xWalkCommon` is built
separately, remove only the compiled outputs while keeping its configuration:

```bash
cmake --build xWalkCommon/build --target clean
```

For a completely clean configure, remove the entire generated build directory:

```bash
cmake -E remove_directory xWalkCommon/build
```

The `include`, `CMakeLists.txt`, and README source files are not removed.
After a full clean, configure and build again with:

```bash
cmake -S xWalkCommon -B xWalkCommon/build
cmake --build xWalkCommon/build --parallel
```
