# xWalk Raspberry Pi 5 PiCar-X

xWalk is a C++17 control and automation workspace for the SunFounder PiCar-X on Raspberry Pi 5. The repository
contains the complete product integration, host-safe simulation and tests, deployment configuration, documentation,
and development tooling.

Normal host builds use simulated or software backends and do not actuate physical hardware.

## Repository layout

```text
MyPiCarX/
├── CMakeLists.txt             Root build entry point
├── CMakePresets.json          Supported host and Raspberry Pi build presets
├── xWalk-rpi5/                Integrated Raspberry Pi 5 product
│   ├── xWalkAgent/            Product behavior and feature agents
│   ├── xWalkAudioResources/   Versioned sound and music resources
│   ├── xWalkController/       CLI and application composition
│   ├── xWalkHal/              Hardware abstraction and simulation backends
│   ├── xWalkIW/               Interface schemas and generated bindings
│   ├── xWalkLibrary/          Shared libraries and external dependencies
│   ├── xWalkTrace/            Shared tracing implementation
│   ├── devloper-note/         Architecture, build, and deployment documentation
│   └── cmake/                 Shared CMake modules and toolchains
└── xWalkTool/                 CI, Gerrit, deployment, quality, and maintenance tools
```

## Prerequisites

The supported host workflow requires Linux, CMake 3.25 or newer, Ninja, a C++17 compiler, Python 3, and the
development libraries used by the complete product.

On Ubuntu or Debian, install the core host dependencies with:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake ninja-build pkg-config python3 libasound2-dev libcurl4-openssl-dev libprotobuf-dev libgrpc++-dev libgtest-dev libjson-c-dev libtinyxml2-dev libyaml-cpp-dev
```

See the [dependency guide](xWalk-rpi5/devloper-note/Doc/note/Dependency%20Installer%20Guide.md) for optional
quality tools, generators, Raspberry Pi packages, and dependency troubleshooting.

## Build the complete repository

Run all commands from the repository root. The `sanity` preset enables the complete Debug host build, tests,
compile commands, and strict compiler warnings.

```bash
cmake --fresh --preset sanity
cmake --build --preset sanity --parallel
ctest --preset sanity
```

The generated files are written below `build-host/sanity`.

For an optimized host build:

```bash
cmake --fresh --preset host-release
cmake --build --preset host-release --parallel
ctest --preset host-release
```

## Host-safe diagnostic

After building the `sanity` preset, validate the deployment configuration without opening hardware devices or
contacting external services:

```bash
build-host/sanity/xWalk-rpi5/xWalkController/xWalkApp/xwalk-picarx-control --deployment-config="$PWD/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf" --diagnose --no-hardware
```

The diagnostic must finish with `[SIMULATED]`. Do not remove `--no-hardware` during ordinary host validation.

## Installation

Create and test a staged Release installation without modifying the host system:

```bash
cmake --fresh --preset host-release
cmake --build --preset host-release --parallel
DESTDIR="$PWD/build-host/deploy" cmake --install build-host/release
```

The staged filesystem is created under `build-host/deploy`. After reviewing that layout, an administrator may
install the same build into the configured `/usr` prefix:

```bash
sudo cmake --install build-host/release
```

System installation does not authorize hardware tests or actuator operation. Raspberry Pi setup, permissions,
services, configuration, and rollback are documented in the
[deployment guide](xWalk-rpi5/devloper-note/Doc/note/Deployment%20Guide.md).

## Raspberry Pi build

On a compatible Raspberry Pi build host, configure and compile the product with:

```bash
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

The final command lists hardware tests; it does not execute them. Run hardware-labelled tests only after explicitly
confirming the Raspberry Pi model, Robot HAT revision, wiring, power, clear movement area, and emergency-stop plan.

## Additional documentation

- [Documentation index](xWalk-rpi5/devloper-note/index.md)
- [Build and installation guide](xWalk-rpi5/devloper-note/Doc/note/Installation.md)
- [Controller and CLI overview](xWalk-rpi5/xWalkController/README.md)
- [Development and maintenance tools](xWalkTool/README.md)
- [Repository instructions](AGENTS.md)

## Gerrit development workflow

Create signed-off commits and upload them to Gerrit for review. Do not push repository changes directly to GitHub.

```bash
git add <files>
git commit -s
git push gerrit HEAD:refs/for/master
```

## License

See [LICENSE](LICENSE) for the repository license terms.
