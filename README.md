# xWalk Raspberry Pi 5 PiCar-X

xWalk is a C++17 control and automation workspace for the SunFounder PiCar-X on Raspberry Pi 5. The repository
contains the complete product integration, host-safe simulation and tests, deployment configuration, documentation,
and development tooling.

Normal host builds use simulated or software backends and do not actuate physical hardware.

## Repository layout

```text
MyPiCarX/
├── xWalk-rpi5/                Integrated Raspberry Pi 5 product
│   ├── CMakeLists.txt         Product build entry point
│   ├── CMakePresets.json      Supported host and Raspberry Pi build presets
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
compile commands, and strict compiler warnings. The preset is loaded from the `xWalk-rpi5` product source tree.

```bash
cmake --fresh -S xWalk-rpi5 --preset sanity
cmake --build build-host/sanity --parallel
ctest --test-dir build-host/sanity --output-on-failure --no-tests=error
```

The generated files are written below `build-host/sanity`.

For an optimized host build:

```bash
cmake --fresh -S xWalk-rpi5 --preset host-release
cmake --build build-host/release --parallel
ctest --test-dir build-host/release --output-on-failure --no-tests=error
```

## Host-safe diagnostic

After building the `sanity` preset, validate the deployment configuration without opening hardware devices or
contacting external services:

```bash
build-host/sanity/xWalkController/xWalkApp/xwalk-picarx-control --deployment-config="$PWD/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf" --diagnose --no-hardware
```

The diagnostic must finish with `[SIMULATED]`. Do not remove `--no-hardware` during ordinary host validation.

## Installation

Create and test a staged Release installation without modifying the host system:

```bash
cmake --fresh -S xWalk-rpi5 --preset host-release
cmake --build build-host/release --parallel
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
cmake --fresh -S xWalk-rpi5 --preset rpi-release
cmake --build build-rpi/cmake --parallel
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

## Run Gerrit and Gerrit CI

### Start the local Gerrit server

Assess the local host before the first installation:

```bash
xWalkTool/py-agent/gerrit-tool/local-linux/gerrit-local.sh assess
```

Install and start a local Gerrit instance when it has not been installed previously:

```bash
xWalkTool/py-agent/gerrit-tool/local-linux/gerrit-local.sh install
```

Start an existing local Gerrit instance after a reboot or shutdown:

```bash
xWalkTool/py-agent/gerrit-tool/local-linux/gerrit-local.sh start
```

After installation, use the generated management commands to inspect and control the server:

```bash
"$HOME/bin/gerrit-status"
"$HOME/bin/gerrit-check"
"$HOME/bin/gerrit-logs"
"$HOME/bin/gerrit-stop"
"$HOME/bin/gerrit-start"
```

Print the configured browser URL:

```bash
git config --file "$HOME/gerrit-site/etc/gerrit.config" --get gerrit.canonicalWebUrl
```

The local profile normally exposes Gerrit SSH on port `29419`. Installation details and troubleshooting are in the
[local Gerrit guide](xWalkTool/py-agent/gerrit-tool/local-linux/README.md). Administrators operating the managed server profile
should use the separate [Gerrit administration guide](xWalkTool/py-agent/gerrit-tool/README.md).

### Run Gerrit Host Quality CI

Gerrit Host Quality CI is implemented by an external Zuul deployment. Gerrit does not execute the GitHub Actions
workflow, and this repository cannot start Zuul, Nodepool, or the executor. A Gerrit or Zuul administrator must
install and start those services, connect Zuul to Gerrit, provide the `ubuntu-24.04-xwalk-ci` node label, and grant
the Zuul service account permission to vote on `Verified`.

The repository supplies the job graph in [`.zuul.yaml`](.zuul.yaml), its playbooks under `xWalkTool/shell-agent/env-tool/playbooks/zuul`, and the
shared job implementation under `xWalkTool/shell-agent`. See the
[Gerrit CI configuration guide](xWalkTool/py-agent/gerrit-tool/DevloperNote/Doc/note/Gerrit%20CI%20Configuration.md) for the
required administrator-side setup and service checks. Do not run the legacy local Gerrit CI worker alongside Zuul,
because both services could vote on the same patch set.

Starting Gerrit does not start Host Quality CI. The Zuul scheduler, executor, web service, and Nodepool launcher are
administrator-managed services outside this repository. Their exact service commands depend on the external Zuul
deployment; confirm that they are running and connected to Gerrit before uploading an active patch set.

After Gerrit and Zuul are online, create a signed-off commit and upload an active patch set. The upload triggers
the Zuul `check` pipeline automatically:

```bash
git add <files>
git commit -s
git push gerrit HEAD:refs/for/master
```

Upload work in progress without triggering CI by adding `%wip`:

```bash
git push gerrit HEAD:refs/for/master%wip
```

Selecting **Mark As Active** in Gerrit triggers CI for the current WIP patch set. The administrator-configured gate
event runs the same Host Quality jobs before submission; repository configuration does not automatically submit a
change.

Run representative repository-owned checks locally before uploading:

```bash
xWalkTool/shell-agent/gerrit-tool/run-host-ci-job.sh shellcheck
xWalkTool/shell-agent/gerrit-tool/run-host-ci-job.sh deployment-scripts
xWalkTool/shell-agent/gerrit-tool/run-host-ci-job.sh build-and-test gcc Debug
```

These checks are host-safe and do not authorize physical hardware tests.

## License

See [LICENSE](LICENSE) for the repository license terms.
