# CMake Dependency Guide

This guide documents the dependencies that CMake needs to configure and build the xWalk C++ workspace. It
describes external development packages, project-target relationships, build-mode differences, and common
discovery failures. It does not document the Python dependency installer.

For the Python command-line interface, see
[Dependency Installer Script Flags](Dependency%20Installer%20Script%20Flags.md).

## Dependency model

The workspace uses target-based CMake dependencies. Project modules are imported with `add_subdirectory()`
and linked through named targets. External libraries are discovered only where their targets are needed.

Do not add global include paths, global linker paths, or manually assembled `-l` flags. Portable reviewed
packages may be installed under the selected `xWalkLibrary` prefix; otherwise install the matching system
development package. In both cases, allow CMake to provide the imported target.

The workspace uses a hybrid dependency model. `xWalk-rpi5/xWalkLibrary/XWalkDependencies.cmake` maps the CMake target
processor to `xWalk-rpi5/xWalkLibrary/x86_64` or `xWalk-rpi5/xWalkLibrary/aarch64`, prepends that prefix and `xWalk-rpi5/xWalkLibrary/common`
to `CMAKE_PREFIX_PATH`, and adds the native library directories to the build-tree RPATH. A compatible local
package is therefore preferred, while ordinary system discovery remains available for a portable dependency
that has not been reviewed into the repository.

Workspace-wide `xWalkLibraryCommon` public headers, architecture-independent models, and configuration live under
`xWalk-rpi5/xWalkLibrary/common`. Each native prefix uses the conventional `bin`, `include`, `lib`, and `share` layout.
Compilers, build tools, Linux kernel interfaces, ALSA integration, udev rules, camera tools, Device Tree
overlays, system services, and package managers remain system-installed. See
[`xWalk-rpi5/xWalkLibrary/README.md`](../../../../xWalk-rpi5/xWalkLibrary/README.md) for inventory and overrides.

The root build follows this dependency flow:

```text
MyPiCarX
├── xWalkLibrary
│   └── common
├── xWalkIW
├── xWalkHal modules and aggregate target
└── xWalkController
    └── xWalkAgent
        └── HAL targets
```

Individual modules import a project dependency only when its target does not already exist. This permits both
standalone module builds and aggregate builds without defining the same target twice.

## CMake and compiler requirements

| Requirement | Workspace role |
| --- | --- |
| CMake 3.25 or newer | Required by the checked-in `CMakePresets.json` schema |
| CMake 3.16 or newer | Minimum declared by individual project and module CMake files |
| C++17 compiler | Required by all public C++ targets |
| Ninja | Generator selected by every root configure preset |
| Python 3 | Runs root deployment tests and validates the xWalkIW schema |
| `pkg-config` | Supports transitive discovery used by the installed gRPC configuration |

GCC and Clang are the supported host compilers. The configured C++ result is authoritative; editor indexing
does not replace a successful CMake configure and build.

## External CMake dependencies

| Dependency | CMake discovery | Imported target or result | Required when |
| --- | --- | --- | --- |
| Protobuf | `find_package(Protobuf REQUIRED)` | `protobuf::libprotobuf` | Every build containing xWalkIW |
| gRPC | `find_package(gRPC CONFIG REQUIRED)` | `gRPC::grpc++` | Every build containing xWalkIW |
| Python 3 | `find_package(Python3)` | `Python3_EXECUTABLE` | xWalkIW and root tests |
| ALSA | `find_package(ALSA REQUIRED)` | `ALSA::ALSA` | Audio and speech targets |
| libcurl | `find_package(CURL REQUIRED)` | `CURL::libcurl` | LanguageModel tests or Ollama provider |
| Threads | `find_package(Threads REQUIRED)` | `Threads::Threads` | Linux GPIO backend or GPIO hardware tests |
| libsndfile | `find_path()` and `find_library()` | `xWalkSndFileDependency` | Native Music decoder is enabled |
| GoogleTest | `find_package(GTest CONFIG REQUIRED)` | `GTest::gtest` | Central HAL tests are enabled |
| json-c | `pkg_check_modules(JSON_C REQUIRED)` | `PkgConfig::JSON_C` | Runtime trace JSON configuration |
| TinyXML2 | `find_package(tinyxml2 CONFIG REQUIRED)` | `tinyxml2::tinyxml2` | Central HAL tests are enabled |
| yaml-cpp | `find_package(yaml-cpp CONFIG REQUIRED)` | `yaml-cpp::yaml-cpp` | YAML runners |
| Linux UAPI headers | `check_include_file_cxx()` | Configure-test results | Raspberry Pi aggregate build |

### GoogleTest, TinyXML2, and yaml-cpp

When `BUILD_TESTING=ON` in a normal host build, `xWalk-rpi5/xWalkHal/xWalkTest/xGoogleTest`
creates the single HAL unit-test executable. GoogleTest supplies test
registration and reporting; TinyXML2 validates the suite/case enablement file.
yaml-cpp loads board, AI, example, and hardware runtime values. GoogleTest and
TinyXML2 are test-only. An RPI build also requires yaml-cpp for the example
launcher, including when testing is disabled.

### Protobuf and gRPC

The top-level `xWalkIW` module is part of the `xWalkHal` aggregate, so normal workspace builds require the
Protobuf and gRPC C++ development libraries. CMake compiles the checked-in generated sources and links them to
`protobuf::libprotobuf` and `gRPC::grpc++`.

The Protobuf compiler and gRPC C++ plugin are not required merely to compile those checked-in sources. They
are required after a schema change when regenerating the `xWalk-rpi5/xWalkIW/auto-gen` tree:

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarIwGenerator --generate-cpp
```

### ALSA

ALSA is required when any of these conditions enables an audio or speech backend:

- `XWALK_AUDIO_BUILD_HOST_TESTS`
- `XWALK_AUDIO_BUILD_HARDWARE_TESTS`
- `XWALK_AUDIO_BUILD_LINUX_BACKEND`
- `XWALK_MUSIC_BUILD_HOST_TESTS`
- `XWALK_MUSIC_BUILD_HARDWARE_TESTS`
- `XWALK_MUSIC_BUILD_ALSA_BACKEND`
- the corresponding xWalkGPT host, hardware, ALSA, Vosk, or Espeak path

The aggregate host suite enables device-free ALSA software tests, so ALSA development headers are part of the
normal host build requirements even though the tests do not open physical audio devices.

### libcurl

The LanguageModel module discovers libcurl when its host tests, hardware tests, or Ollama provider are
enabled. Host tests inject controlled transport behavior; finding and linking libcurl does not contact an
Ollama endpoint.

### Threads and Linux headers

The Linux GPIO backend links `Threads::Threads`. This is normally supplied by the compiler and operating
system rather than a separate package.

The Raspberry Pi aggregate checks for these Linux UAPI headers before adding hardware targets:

- `linux/gpio.h`
- `linux/i2c-dev.h`
- `linux/i2c.h`
- `linux/spi/spidev.h`

The check is compile-only and does not open a GPIO, I2C, or SPI device.

### libsndfile

The native Music decoder searches for `sndfile.h` and the `sndfile` library only when
`XWALK_MUSIC_BUILD_SNDFILE_DECODER=ON`. That option also requires the ALSA adapter. The Raspberry Pi aggregate
enables both; the normal root host build does not enable the libsndfile decoder.

## Debian and Ubuntu packages

Install the dependencies for the complete root host build and test suite:

```sh
sudo apt-get install build-essential cmake ninja-build pkg-config python3 libasound2-dev libcurl4-openssl-dev libprotobuf-dev libgrpc++-dev libgtest-dev libjson-c-dev libtinyxml2-dev libyaml-cpp-dev
```

For the full Raspberry Pi-compatible CMake build, also install the Linux headers and libsndfile development
package:

```sh
sudo apt-get install linux-libc-dev libsndfile1-dev
```

To regenerate the checked-in Protobuf and gRPC sources, additionally install:

```sh
sudo apt-get install protobuf-compiler protobuf-compiler-grpc
```

Runtime utilities such as `alsa-utils`, `espeak-ng`, `libttspico-utils`, `i2c-tools`, `gpiod`,
`rpicam-apps`, `ffmpeg`, Ollama, and Vosk are deployment dependencies, not CMake library-discovery
requirements. See
[`xWalkTool/apt-packages.txt`](../../../../xWalkTool/apt-packages.txt) for the complete build and runtime package
map.

The package commands above are the system fallback. GoogleTest, json-c, TinyXML2, yaml-cpp, Protobuf, gRPC,
and libsndfile can instead be supplied under the selected native `xWalkLibrary`
prefix. Vosk is already present there. APT packages are not extracted into `xWalkLibrary`, because their
transitive dependencies, absolute paths, post-install scripts, and metadata can require the system prefix.

## Root build modes

| Preset | Additional dependency behavior |
| --- | --- |
| `host-debug` | Full host tests: Protobuf, gRPC, Python, ALSA, curl, GoogleTest, json-c, TinyXML2, and yaml-cpp |
| `host-release` | Uses the same dependency surface as `host-debug` |
| `sanity` | Uses host dependencies and enables strict compiler warnings |
| `clang-tidy` | Adds the external `clang-tidy` executable and compilation database |
| `sanitizers` | Requires compiler AddressSanitizer and UndefinedBehaviorSanitizer support |
| `thread-sanitizer` | Requires compiler ThreadSanitizer support in a separate build tree |
| `coverage` | Requires GCC-compatible coverage instrumentation and the external `gcovr` tool |
| `rpi-release` | Adds Linux UAPI headers, libsndfile, RPi backends, and Debian packaging |

Configure, build, and test the normal host tree from the workspace root:

```sh
cmake --fresh --preset host-debug
cmake --build --preset host-debug --parallel
ctest --preset host-debug
```

The Raspberry Pi preset builds hardware-labelled executables but hardware tests remain opt-in. Listing them
is safe:

```sh
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

Do not execute the hardware tests unless the correct Raspberry Pi and Robot HAT are connected, the robot is
secured, and physical execution is explicitly approved.

## Standalone module dependency behavior

Standalone production-only modules usually need only a C++17 compiler and their internal project targets.
Enabling a backend or test option can expand the external dependency surface.

| Module | Option or mode | Added external requirement |
| --- | --- | --- |
| `xWalkIW` | Any configuration | Protobuf, gRPC, and Python 3 |
| `xWalkAudio` | Host tests, hardware tests, or Linux backend | ALSA |
| `xWalkMusic` | Host tests, hardware tests, or ALSA backend | ALSA |
| `xWalkMusic` | `XWALK_MUSIC_BUILD_SNDFILE_DECODER=ON` | libsndfile and ALSA |
| `xWalkGPT` | Host tests, hardware tests, or speech/provider backends | ALSA and Linux |
| `xWalkLanguageModel` | Tests or Ollama provider | libcurl |
| `xWalkGpio` | Hardware tests or Linux backend | Threads and Linux GPIO headers |
| Hardware-dependent HAL modules | Hardware-test option | Linux and inherited backend dependencies |

Project-library dependencies such as `xWalkLibraryCommon`, `xWalkI2c`, `xWalkPwm`, `xWalkGpio`, and `xWalkAudio`
are imported by the owning CMake files. Do not install them as operating-system packages.

## Quality and packaging tools

These executables support workspace workflows but are not linked C++ libraries:

| Tool | Used by |
| --- | --- |
| Clang-Tidy | `clang-tidy` preset build |
| Cppcheck | `cppcheck` and `cppcheck-full` custom targets when detected |
| gcovr | Coverage report and enforced threshold generation |
| ShellCheck | GitHub host-quality workflow for shell scripts |
| CPack and Debian tools | `rpi-release` packaging and release validation |

The root CMake project adds Cppcheck targets only when `cppcheck` is found and compilation-database export is
enabled. Absence of Cppcheck does not break an ordinary host build.

## CMake discovery troubleshooting

### Protobuf not found

Install a reviewed Protobuf build under the selected `xWalkLibrary` prefix or install the system development
package. The compiler package alone is insufficient because CMake needs the headers and link library.

### gRPC configuration not found

Install the gRPC C++ development package that provides `gRPCConfig.cmake`. Adding only a runtime gRPC library
does not provide the imported `gRPC::grpc++` target.

### ALSA not found

Install ALSA development headers or disable the option that requested an ALSA backend. The aggregate host
suite intentionally enables ALSA software tests.

### CURL not found

Install the libcurl development package or configure a production-only LanguageModel target without tests or
the Ollama provider.

### libsndfile development files missing

Install the libsndfile development package or disable `XWALK_MUSIC_BUILD_SNDFILE_DECODER`. Do not leave the
decoder enabled while disabling its required ALSA adapter.

### Linux UAPI header check failed

Install the target Linux userspace headers and confirm that the selected compiler or cross-compiling sysroot
contains GPIO, I2C, and SPI headers. Do not bypass the check with cached result variables.

### Generated xWalkIW file missing

Install `protoc` and `grpc_cpp_plugin`, regenerate the checked-in files from the reviewed schemas, and rerun a
fresh configure. Do not hand-edit generated output.

## Verification boundary

CMake dependency discovery and compilation do not establish that runtime services, models, device nodes, or
physical hardware are available. Host tests remain the default verification path. Hardware execution requires
the separate deployment and safety procedures.
