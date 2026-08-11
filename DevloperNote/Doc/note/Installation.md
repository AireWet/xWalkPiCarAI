# Build and installation

xWalk HAL uses CMake 3.16 or newer and requires a C++17 compiler. The root
project builds all modules together and exposes separate host and Raspberry Pi
hardware configurations.

Both aggregate flags default to `OFF`. A default configuration builds every
production library without host tests, Linux hardware backends, or
hardware-labelled executables.

## Complete host build

The host configuration builds every production library and deterministic host
test without requiring attached hardware:

```sh
cmake --fresh --preset sanity
cmake --build --preset sanity --parallel
ctest --preset sanity
```

Only host and unit tests are registered in this build tree, so plain `ctest`
runs every host test defined by every submodule.

## Complete Raspberry Pi hardware build

The RPI configuration builds every production library, the Linux I2C and GPIO
backends, and all hardware-labelled executables. It can be compiled on Ubuntu
without a connected Robot HAT:

```sh
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

The last command only lists hardware tests. Run hardware tests later on the
correct Raspberry Pi and Robot HAT after reviewing actuator and power safety.
The Ubuntu RPI build requires the Linux GPIO, I2C, and SPI UAPI headers supplied by
the `linux-libc-dev` package.

The offline voice-car commands additionally require these Raspberry Pi runtime
components:

- the architecture-selected Vosk shared library from `xWalkLibrary`;
- the shared small English Vosk model from `xWalkLibrary`;
- the `espeak-ng` executable;
- the `pico2wave` executable from `libttspico-utils` for treasure hunt;
- working ALSA capture, PCM playback, and mixer devices.

Their paths and device names are configured in
`xWalkController/xWalkConfig/picar-x.conf`. Vosk is loaded dynamically, so
vendor development headers are not required to compile xWalk. Verify deployment
package names and model locations for the Raspberry Pi operating-system release.

The install target places the selected Vosk runtime at
`/usr/lib/xwalk/libvosk.so` and the model below
`/usr/share/xwalk/models/vosk`.

After the complete receiver, actuator, Raspberry Pi, and Robot HAT setup is
connected and verified safe, run every registered submodule hardware test:

```sh
ctest --test-dir xWalkHal/build-rpi --output-on-failure
```

Only hardware tests are registered in this build tree. Host and RPI flags are
mutually exclusive and must use separate build directories.

Applications can link the aggregate `xWalkHal` target or its `xWalk::Hal` alias
when this directory is included with `add_subdirectory()`.

## Command-line application

`xWalkController` is a standalone aggregate beside `xWalkAgent`. Its
`xWalkController` submodule provides the `xwalk-picarx-control` Raspberry Pi
application and imports the Agent coordinators through CMake.

Use the root sanity commands above; the workspace aggregate includes the CLI and all deterministic host tests.

Compile the Raspberry Pi composition and list its hardware-labelled tests
without executing them:

```sh
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

Stage the CLI, administrator configuration, profiles, media, provisioning tools,
systemd files, permissions template, and documentation without modifying the host:

```sh
cmake --fresh -S . -B build-host/cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build-host/cmake --parallel
DESTDIR="$PWD/build-host/deploy" cmake --install build-host/cmake
```

The administrator manifest is `/etc/xwalk/picar-x.conf`, with functional fragments
under `/etc/xwalk/picar-x.d`. Setup initializes the active manifest and fragment
tree under `/var/lib/xwalk` once. See the [deployment guide](Deployment%20Guide.md) for staging,
Debian packaging, provisioning, service setup, permissions, analysis, coverage, and hardware acceptance.

See the [xWalk Controller README](../../../xWalkController/README.md) for aggregate ownership
and the [xWalkApp README](../../../xWalkController/xWalkApp/README.md)
for every command, action, safety rule, and backend-composition detail.

## Clean generated build output

Preview every root and submodule build directory that will be removed:

```sh
xWalkTool/shell/clean-build.sh --dry-run
```

Remove all listed `build` and `build-*` directories, supported in-source CMake
output, and recognized Python-generated caches and package output:

```sh
xWalkTool/shell/clean-build.sh --yes
```

The cleaner never removes a source `CMakeLists.txt` or Python source file.
Removed generated output is not recoverable, but it can be regenerated with the
documented build and test commands.

## Build one module

Each module remains independently configurable. For example:

```sh
cmake -S xWalkHal/device/xWalkPwm -B xWalkHal/device/xWalkPwm/build
cmake --build xWalkHal/device/xWalkPwm/build --parallel
```

Enable host tests with the option documented by the module README:

```sh
cmake -S xWalkHal/device/xWalkPwm -B xWalkHal/device/xWalkPwm/build-host -DXWALK_PWM_BUILD_HOST_TESTS=ON
cmake --build xWalkHal/device/xWalkPwm/build-host --parallel
ctest --test-dir xWalkHal/device/xWalkPwm/build-host --output-on-failure
```

Build hardware tests only when the target headers and dependencies are present.
List hardware tests without executing them during normal verification:

```sh
ctest --test-dir xWalkHal/device/xWalkPwm/build-rpi -N -L hardware
```

`Doc/note` contains Markdown sources and `Doc/image` contains referenced image
assets. The documentation has no build or installation step.

## C++ build-screen image

TODO: Add a C++ CMake build screenshot when a project-owned image is available.
