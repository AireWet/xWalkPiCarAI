# xWalkTool

`xWalkTool` contains repository maintenance, host verification, Raspberry Pi provisioning, interface
generation, and SunFounder Device Tree overlay assets. It is not a runtime HAL or Agent module.

No command in this directory is run automatically by a normal host build. Hardware-affecting commands remain
explicit and must not be used until the Raspberry Pi model, Robot HAT revision, wiring, and safety state are
confirmed.

## Directory inventory

```text
xWalkTool/
├── README.md
├── apt-packages.txt
├── deployment/
│   ├── debian/
│   ├── systemd/
│   ├── test/
│   ├── tmpfiles/
│   └── udev/
├── dtoverlays/
│   ├── sunfounder-robothat5.dtbo
│   └── sunfounder-servohat+.dtbo
├── environment/
│   ├── .clang-tidy
│   ├── .cppcheck-suppressions
│   └── gcovr.cfg
├── python/
│   ├── xHal_Rpi5CarDependencyInstaller
│   └── xHal_Rpi5CarIwGenerator
└── shell/
    ├── clean-build.sh
    ├── eclipse-build.sh
    ├── provision-hardware.sh
    ├── run-host-coverage.sh
    └── setup-rpi.sh
```

The directory contains five Bash scripts, one package manifest, two executable
Python tools, three quality-tool configurations, eight deployment assets and
tests, two compiled Device Tree blobs, and this README. The applicable project
and third-party license terms are in the workspace-root `LICENSE`; there is no
separate `xWalkTool/LICENSE` file.

## Detailed guides

- [xWalkTool overview](../DevloperNote/Doc/note/xWalkTool%20Overview.md)
- [Clean build script](../DevloperNote/Doc/note/Clean%20Build%20Script%20Guide.md)
- [Eclipse build script](../DevloperNote/Doc/note/Eclipse%20Build%20Script%20Guide.md)
- [Host coverage script](../DevloperNote/Doc/note/Host%20Coverage%20Script%20Guide.md)
- [CMake dependencies](../DevloperNote/Doc/note/Dependency%20Installer%20Guide.md)
- [Dependency installer flags](../DevloperNote/Doc/note/Dependency%20Installer%20Script%20Flags.md)
- [Raspberry Pi setup script](../DevloperNote/Doc/note/Raspberry%20Pi%20Setup%20Script%20Guide.md)
- [Hardware provisioning script](../DevloperNote/Doc/note/Hardware%20Provisioning%20Script%20Guide.md)
- [Device Tree overlay assets](../DevloperNote/Doc/note/Device%20Tree%20Overlay%20Assets%20Guide.md)

## Responsibility summary

| Path | Responsibility | Host safety |
|---|---|---|
| `apt-packages.txt` | Lists and maps host, Raspberry Pi, quality, packaging, and optional packages | Read-only |
| `python/xHal_Rpi5CarDependencyInstaller` | Installs dependencies and configures verified v5 boot | Privileged |
| `shell/clean-build.sh` | Finds and optionally deletes generated output | Dry-run is non-destructive |
| `shell/eclipse-build.sh` | Configures or cleans the Eclipse-oriented CLI host build | Does not access hardware |
| `shell/run-host-coverage.sh` | Runs the root coverage preset, tests, and `gcovr` | Foreground and host-only |
| `shell/setup-rpi.sh` | Inspects, plans, validates, or applies Raspberry Pi provisioning | Apply is privileged |
| `shell/provision-hardware.sh` | Records selected GPIO, I2C, SPI, and board identity | Modifies one config file |
| `python/xHal_Rpi5CarIwGenerator` | Validates and generates xWalkIW Protobuf/gRPC sources | Host-only generation |
| `environment/` | Configures Clang-Tidy, Cppcheck suppressions, and gcovr reports | Host-only configuration |
| `deployment/` | Stores packaging, service, permission, and test assets | Host validation and installation |
| `dtoverlays/` | Stores unchanged SunFounder Robot HAT and Servo HAT+ blobs | Raspberry Pi boot assets |

## Dependency installation and host maintenance

### Apt package manifest

[`apt-packages.txt`](apt-packages.txt) separates the minimum host build/test packages from optional quality,
packaging, deployment-validation, Raspberry Pi, camera, and inactive gRPC-generator dependencies. It also
maps every xWalk HAL source module to its internal libraries and operating-system packages. It is a
reviewable manifest and contains the machine-readable package-manager catalog used by
`xHal_Rpi5CarDependencyInstaller`.

Check every automatically selected dependency without changing the machine:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --check
```

Preview installation after detecting the operating system and device:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --dry-run
```

Install all supported catalog scopes for the automatically detected platform:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller
```

The default includes required, native-audio, quality, release, generator, and external scopes. The installer
does not download Vosk models or install Ollama through an unverified remote pipeline; missing external
components are reported with a cross and an explanation. Use `--required-only` for the minimum normal
build/runtime package set. Use `--include quality`, `--include release`, `--include generator`, or
`--include external` with `--required-only` to add selected optional scopes.

Specify a platform and device when automatic detection is not appropriate:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os ubuntu --device host --required-only
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os fedora --device host --required-only
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os arch --device host --required-only
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os macos --device host --required-only
```

Supported package families are Debian/Ubuntu/Raspberry Pi OS through APT, Fedora/RHEL through DNF,
Arch/Manjaro through Pacman, and macOS through Homebrew. The macOS report intentionally marks ALSA and Linux
hardware dependencies unavailable because the current CMake host aggregate requires Linux ALSA. It does not
claim that the complete firmware builds on macOS.

Normal Linux package installation uses root or `sudo`; Homebrew is never run through `sudo`. The final table
contains a tick for installed packages and a cross for missing, failed, external, or unsupported dependencies.
The script returns non-zero after `--check` or installation when selected dependencies remain unavailable.

`--device auto` is the default and uses the local Device Tree to distinguish a Raspberry Pi from a host.
`--target` remains an alias for compatibility. `--device host` skips every `config.txt` and overlay operation.

On a Raspberry Pi with a physically verified Robot HAT v5, review the boot plan before installation:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os raspbian --device rpi --profile robot_hat_v5 --camera csi --required-only --dry-run
```

The Raspberry Pi path requires the supported v5 UUID before changing anything. It verifies the bundled
overlay checksum, rejects an active Servo HAT+ overlay or disabled I2C/SPI setting, backs up `config.txt`,
backs up a different existing destination blob, installs `sunfounder-robothat5.dtbo`, and idempotently adds:

```text
dtparam=i2c_arm=on
dtparam=spi=on
dtoverlay=sunfounder-robothat5
```

It detects `/boot/firmware/config.txt` with `/boot/firmware/overlays` or `/boot/config.txt` with
`/boot/overlays`. It never reboots automatically. Reboot and complete passive diagnostics and physical
hardware acceptance before movement.

There is no verified Robot HAT v4 overlay in `xWalkTool/dtoverlays`. The Servo HAT+ blob must not be used as
a v4 substitute. Selecting `--device rpi --profile robot_hat_v4` therefore fails without changing the boot
configuration.

Raspberry Pi provisioning should still use `setup-rpi.sh --dry-run` before `--apply`. The guarded v5 path can
configure boot interfaces and its overlay, but it does not configure runtime groups, udev rules, exact device
identity, or mutable deployment configuration.

### Build cleanup

Preview every generated build target without deleting anything:

```sh
xWalkTool/shell/clean-build.sh --dry-run
```

Interactive cleanup requires confirmation. Non-interactive cleanup requires the explicit `--yes` option:

```sh
xWalkTool/shell/clean-build.sh --yes
```

Options:

| Option | Effect |
|---|---|
| `--dry-run` | Lists detected CMake and Python output and exits without deleting it |
| `--yes` | Deletes the listed output without an interactive prompt |
| `--help`, `-h` | Prints usage and exits |

Cleanup is restricted to named CMake build output, detected in-source CMake
output, and recognized Python caches, coverage data, and package-build output.
Generated output can be rebuilt, but cleanup results cannot be recovered directly.

### Eclipse host build

Configure and build the established Eclipse CLI host directory:

```sh
xWalkTool/shell/eclipse-build.sh
```

Clean that configured build without deleting its directory:

```sh
xWalkTool/shell/eclipse-build.sh clean
```

With no argument, the script configures and builds. The only meaningful argument is `clean`; the script does
not currently provide a help option. It configures `xWalkCLI` with the host backend, a Debug build, and
`compile_commands.json`. It writes only below `xWalkCLI/build-eclipse-host` and does not select Raspberry Pi
backends.

### Coverage

Run coverage in the foreground:

```sh
xWalkTool/shell/run-host-coverage.sh run
```

The required `run` action performs these steps in order:

1. Finds `gcovr`.
2. Configures the root CMake `coverage` preset with `cmake --fresh`.
3. Builds the coverage preset.
4. Runs its CTest preset.
5. Generates the reports defined by `xWalkTool/environment/gcovr.cfg`.

Use `xWalkTool/shell/run-host-coverage.sh --help` to print usage. Any other action exits with status 2.

The script searches for a system `gcovr` executable and then
`build-host/tools/gcovr-venv/bin/gcovr`. It fails before configuring when neither exists. It never installs
packages, requests privileges, creates a detached process, or accesses physical hardware.

Coverage uses the root `coverage` preset and
`xWalkTool/environment/gcovr.cfg`. Reports are written to:

```text
build-host/coverage/coverage.html
build-host/coverage/coverage.xml
```

The report command is also a gate. It fails below 79 percent total line
coverage or 40 percent total branch coverage.

## Raspberry Pi provisioning

`setup-rpi.sh` supports Debian-family Raspberry Pi OS and Ubuntu Server on a Raspberry Pi. It requires an
explicit Robot HAT profile, runtime user, and GPIO device. Inspection and planning must precede apply mode.

Show available options:

```sh
xWalkTool/shell/setup-rpi.sh --help
```

Run a non-modifying plan after physically identifying the board revision:

```sh
xWalkTool/shell/setup-rpi.sh --dry-run --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
```

Run required validation without changing the system:

```sh
xWalkTool/shell/setup-rpi.sh --check --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
```

The complete option set is:

| Option | Required/default | Effect |
|---|---|---|
| `--profile robot_hat_v4\|robot_hat_v5` | Required | Selects an explicitly verified HAT profile |
| `--runtime-user USER` | Required | Selects an existing unprivileged runtime user |
| `--gpio-device DEVICE` | Required | Selects one `/dev/gpiochipN` controller |
| `--i2c-device DEVICE` | `/dev/i2c-1` | Selects one `/dev/i2c-N` controller |
| `--spi-device DEVICE` | `/dev/spidev0.0` | Selects one `/dev/spidevN.N` controller |
| `--config FILE` | `/var/lib/xwalk/picar-x.conf` | Selects mutable deployment configuration |
| `--camera csi\|usb` | `csi` | Selects `rpicam-apps` or `ffmpeg` as the camera dependency |
| `--with-vosk` | Disabled | Reports configured Vosk library and model availability |
| `--with-ollama` | Disabled | Reports Ollama executable and model-manifest availability |
| `--dry-run` | Default | Prints the plan without changing the system |
| `--check`, `--validate` | Optional mode | Validates required target state without changing it |
| `--apply` | Optional mode | Performs the reported privileged changes |
| `--help`, `-h` | Optional | Prints usage and exits |

The setup workflow:

- identifies the operating system and Raspberry Pi model;
- refuses an unverified Robot HAT v5 profile;
- rejects a v4 profile when the supported v5 UUID is detected;
- reports required and optional packages separately;
- validates I2C, SPI, GPIO, configuration, camera, voice, and model prerequisites;
- preserves existing boot configuration and rejects conflicting disabled interfaces;
- plans narrowly scoped groups and udev rules for selected device nodes;
- never installs, enables, disables, or guesses a Robot HAT overlay.

`--apply` may install packages, enable base I2C and SPI interfaces, create groups, update membership, install a
generated udev rule, and update deployment configuration. It requires root or `sudo` and must be reviewed from
the preceding dry-run output. It must not be invoked by host tests or CI.

### Hardware identity provisioning

`provision-hardware.sh` is the focused configuration updater used by setup after devices are available. It
requires an explicit v4/v5 profile and an existing writable configuration file:

```sh
xWalkTool/shell/provision-hardware.sh --help
```

Example after the target devices and board revision have been verified:

```sh
xWalkTool/shell/provision-hardware.sh --profile robot_hat_v4 --config /var/lib/xwalk/picar-x.conf --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0
```

`--profile` and `--config` are mandatory. The device options select exact device nodes. If
`--gpio-device` is omitted, automatic selection succeeds only when exactly one GPIO chip exists. An omitted
I2C or SPI option leaves that configuration value absent or blank instead of guessing a device.

It validates selected device-path syntax, requires `gpiodetect`, checks that the selected GPIO chip has a
usable label and at least 28 lines, and replaces the configuration atomically. It never moves an actuator or
claims GPIO, I2C, SPI, PWM, servo, or motor outputs.

The Raspberry Pi deployment guide remains the authoritative end-to-end procedure:
[Deployment Guide](../DevloperNote/Doc/note/Deployment%20Guide.md).

## xWalkIW generator status

`xHal_Rpi5CarIwGenerator` is an executable Python 3 tool for validating xWalkIW Protobuf/YAML contracts and
generating routed C++ Protobuf and gRPC sources. Its supported actions are:

```sh
xWalkTool/python/xHal_Rpi5CarIwGenerator --help
xWalkTool/python/xHal_Rpi5CarIwGenerator --check
xWalkTool/python/xHal_Rpi5CarIwGenerator --generate-cpp
```

The default input root is `xWalkIW`. The aggregate build validates its
schema contract and compiles the generated C++ library. The module README is the
authoritative protocol, generation, build, and test guide.

Generation additionally requires `protoc`, `grpc_cpp_plugin`, the imported Protobuf schemas, and writable
output directories. Generated files belong under the selected module's `auto-gen/include` and `auto-gen/src`
directories and must not be treated as handwritten production sources.

## Device Tree overlay assets

The two `.dtbo` files are compiled Raspberry Pi Device Tree blobs, not executable host tools and not C++
libraries. They are not activated by `setup-rpi.sh`, the CMake install rules, or host CI. The dependency
installer can install and activate only the Robot HAT v5 blob under the guarded workflow described above.

| File | Board role |
|---|---|
| `sunfounder-robothat5.dtbo` | SunFounder Robot HAT 5 boot-time configuration |
| `sunfounder-servohat+.dtbo` | SunFounder Servo HAT+ boot-time configuration |

Never infer Robot HAT v4 from failed v5 discovery. Never activate both overlays: they configure overlapping
I2C, SPI, I2S, and audio resources. Overlay installation requires physical board verification followed by a
reviewed dry-run, reboot, and hardware acceptance.

## Installation behavior

The release installation includes only the two target provisioning scripts:

```text
/usr/lib/xwalk/setup-rpi.sh
/usr/lib/xwalk/provision-hardware.sh
```

The cleanup, Eclipse, coverage, generator, and overlay files remain source-development assets. They are not
required by the installed CLI. Mutable runtime configuration remains under `/etc/xwalk` and `/var/lib/xwalk`,
not under this source directory.

## Safe verification

The following checks do not access physical hardware:

```sh
bash -n xWalkTool/shell/*.sh
xWalkTool/shell/clean-build.sh --dry-run
xWalkTool/shell/run-host-coverage.sh --help
xWalkTool/shell/setup-rpi.sh --help
xWalkTool/shell/provision-hardware.sh --help
xWalkTool/python/xHal_Rpi5CarIwGenerator --help
bash xWalkTool/deployment/test/setup-rpi-test.sh
```

Do not run `setup-rpi.sh --apply`, direct overlay installation, hardware-labelled CTest targets, or actuator
commands during ordinary host verification.
