# xWalkUtils

`xWalkUtils` provides generic services without placing terminal, shell, process, network, environment, or
descriptor ownership
inside the hardware-independent embedded core.

The module provides:

- `XWalkUtils` for colored output, clamped system volume, synchronous command
  dispatch, executable checks, IPv4 lookup, username lookup, and linear mapping.
- `XWalkLazyReader<ValueType>` for bounded-rate callback acquisition and caching
  of value-like data or non-owning pointers.
- `XWalkStderrGuard` for scoped standard-error redirection.

The application creates platform services in `main()` and injects callbacks.
Command callbacks must validate untrusted input; `XWalkUtils` never constructs
or invokes `sudo`, `amixer`, `which`, or another shell command itself. Terminal
color rendering is similarly owned by the output callback.

`XWalkUtilsLinux` is the optional Raspberry Pi Linux backend. It provides ANSI
output through the standard-output descriptor, PCM volume through `amixer`,
shell-compatible command execution with combined output, direct `PATH`
executable lookup, interface enumeration, effective-user lookup, and RAII
standard-error redirection. It never invokes `sudo`. Command text retains the
upstream shell interpretation, so applications must validate untrusted values
before composing a command.

The deprecated `reset_mcu()`, `get_battery_voltage()`, `set_pin()`,
`enable_speaker()`, and `disable_speaker()` wrappers remain represented by
`XWalkBoardControl`, where their current hardware operations belong.

An empty IP address or username represents a not-found result. Executable checks share one typed backend
operation because only the executable-search mechanism differs.

## Host build and test

```sh
cmake -S xWalkUtils -B build-utils-host -DXWALK_UTILS_BUILD_HOST_TESTS=ON
cmake --build build-utils-host --parallel
ctest --test-dir build-utils-host --output-on-failure
```

On Linux, this runs both the backend-neutral callback test and the safe Linux
backend software test. The software test does not invoke `amixer` or change any
hardware or mixer state.

## Linux backend build

Applications can build the backend without registering tests:

```sh
cmake -S xWalkUtils -B build-utils-linux -DXWALK_UTILS_BUILD_LINUX_BACKEND=ON
cmake --build build-utils-linux --parallel
```

Link `xWalkUtilsLinux`, create the backend before `XWalkUtils`, and pass
`backend.utilityCallbacks()` with `&backend` as the callback context. The same
backend provides the paired callbacks for `XWalkStderrGuard`.

## Raspberry Pi hardware test

```sh
cmake -S xWalkUtils -B build-utils-rpi -DXWALK_UTILS_BUILD_HARDWARE_TESTS=ON
cmake --build build-utils-rpi --parallel
ctest --test-dir build-utils-rpi -N -L hardware
```

The final command lists the hardware test without executing it. The test writes
one ANSI terminal record, checks the shell, loopback interface, username, and
`amixer`, then changes PCM playback volume to fifty percent. Execute it only on
an approved Raspberry Pi setup.

## Module layout

```text
include/                                      Core public interfaces and callback types
src/                                          Hardware-independent utility behavior
test/src/                                     In-memory core host tests
hardware/include/xHal_Rpi5CarUtilsLinux.h     Raspberry Pi Linux backend interface
hardware/src/xHal_Rpi5CarUtilsLinux.cpp       Linux platform operations
hardware/src/xHal_Rpi5CarUtilsLinuxCallbacks.cpp Callback composition and bridges
hardware/test/src/xHal_Rpi5CarUtilsLinuxTest.cpp Safe Linux software test
test/hardware/src/xHal_Rpi5CarUtilsHardwareTest.cpp Opt-in Raspberry Pi hardware test
```

| File | Responsibility |
| --- | --- |
| `xHal_Rpi5CarUtilsLinux.h` | Declares the non-owning Linux callback composition API. |
| `xHal_Rpi5CarUtilsLinux.cpp` | Owns each bounded Linux platform operation. |
| `xHal_Rpi5CarUtilsLinuxCallbacks.cpp` | Bridges callback contexts to the backend. |
| `xHal_Rpi5CarUtilsLinuxTest.cpp` | Tests safe Linux behavior without mixer changes. |
| `xHal_Rpi5CarUtilsHardwareTest.cpp` | Verifies Raspberry Pi platform and mixer behavior. |
