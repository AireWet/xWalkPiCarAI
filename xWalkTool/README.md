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
├── clean-build.sh
├── eclipse-build.sh
├── provision-hardware.sh
├── run-host-coverage.sh
├── setup-rpi.sh
├── xHal_Rpi5CarIwGenerator
└── dtoverlays/
    ├── sunfounder-robothat5.dtbo
    └── sunfounder-servohat+.dtbo
```

The directory contains five Bash scripts, one Python generator, two compiled Device Tree blobs, and this
README. The applicable project and third-party license terms are in the workspace-root `LICENSE`; there is no
separate `xWalkTool/LICENSE` file.

## Detailed guides

- [xWalkTool overview](../DevloperNote/Doc/note/xWalkTool%20Overview.md)
- [Clean build script](../DevloperNote/Doc/note/Clean%20Build%20Script%20Guide.md)
- [Eclipse build script](../DevloperNote/Doc/note/Eclipse%20Build%20Script%20Guide.md)
- [Host coverage script](../DevloperNote/Doc/note/Host%20Coverage%20Script%20Guide.md)
- [Raspberry Pi setup script](../DevloperNote/Doc/note/Raspberry%20Pi%20Setup%20Script%20Guide.md)
- [Hardware provisioning script](../DevloperNote/Doc/note/Hardware%20Provisioning%20Script%20Guide.md)
- [Device Tree overlay assets](../DevloperNote/Doc/note/Device%20Tree%20Overlay%20Assets%20Guide.md)

## Responsibility summary

| Path | Responsibility | Host safety |
|---|---|---|
| `clean-build.sh` | Discovers and optionally deletes generated CMake output | Dry-run is non-destructive |
| `eclipse-build.sh` | Configures or cleans the Eclipse-oriented CLI host build | Does not access hardware |
| `run-host-coverage.sh` | Runs the root coverage preset, tests, and `gcovr` | Foreground and host-only |
| `setup-rpi.sh` | Inspects, plans, validates, or applies Raspberry Pi provisioning | Apply is privileged |
| `provision-hardware.sh` | Records selected GPIO, I2C, SPI, and board identity | Modifies one config file |
| `xHal_Rpi5CarIwGenerator` | Validates and generates xWalkIW Protobuf/gRPC C++ files | Currently inactive |
| `dtoverlays/` | Stores unchanged SunFounder Robot HAT and Servo HAT+ blobs | Raspberry Pi boot assets |

## Host maintenance

### Build cleanup

Preview every generated build target without deleting anything:

```sh
xWalkTool/clean-build.sh --dry-run
```

Interactive cleanup requires confirmation. Non-interactive cleanup requires the explicit `--yes` option:

```sh
xWalkTool/clean-build.sh --yes
```

Options:

| Option | Effect |
|---|---|
| `--dry-run` | Lists detected build output and exits without deleting it |
| `--yes` | Deletes the listed output without an interactive prompt |
| `--help`, `-h` | Prints usage and exits |

Cleanup is destructive only for directories named `build` or `build-*` and detected in-source CMake output.
Generated output can be rebuilt, but cleanup results cannot be recovered directly.

### Eclipse host build

Configure and build the established Eclipse CLI host directory:

```sh
xWalkTool/eclipse-build.sh
```

Clean that configured build without deleting its directory:

```sh
xWalkTool/eclipse-build.sh clean
```

With no argument, the script configures and builds. The only meaningful argument is `clean`; the script does
not currently provide a help option. It configures `xWalkCLI` with the host backend, a Debug build, and
`compile_commands.json`. It writes only below `xWalkCLI/build-eclipse-host` and does not select Raspberry Pi
backends.

### Coverage

Run coverage in the foreground:

```sh
xWalkTool/run-host-coverage.sh run
```

The required `run` action performs these steps in order:

1. Finds `gcovr`.
2. Configures the root CMake `coverage` preset with `cmake --fresh`.
3. Builds the coverage preset.
4. Runs its CTest preset.
5. Generates the reports defined by `gcovr.cfg`.

Use `xWalkTool/run-host-coverage.sh --help` to print usage. Any other action exits with status 2.

The script searches for a system `gcovr` executable and then
`build-host/tools/gcovr-venv/bin/gcovr`. It fails before configuring when neither exists. It never installs
packages, requests privileges, creates a detached process, or accesses physical hardware.

Coverage uses the root `coverage` preset and `gcovr.cfg`. Reports are written to:

```text
build-host/coverage/coverage.html
build-host/coverage/coverage.xml
```

## Raspberry Pi provisioning

`setup-rpi.sh` supports Debian-family Raspberry Pi OS and Ubuntu Server on a Raspberry Pi. It requires an
explicit Robot HAT profile, runtime user, and GPIO device. Inspection and planning must precede apply mode.

Show available options:

```sh
xWalkTool/setup-rpi.sh --help
```

Run a non-modifying plan after physically identifying the board revision:

```sh
xWalkTool/setup-rpi.sh --dry-run --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
```

Run required validation without changing the system:

```sh
xWalkTool/setup-rpi.sh --check --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
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
xWalkTool/provision-hardware.sh --help
```

Example after the target devices and board revision have been verified:

```sh
xWalkTool/provision-hardware.sh --profile robot_hat_v4 --config /var/lib/xwalk/picar-x.conf --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0
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
xWalkTool/xHal_Rpi5CarIwGenerator --help
xWalkTool/xHal_Rpi5CarIwGenerator --check
xWalkTool/xHal_Rpi5CarIwGenerator --generate-cpp
```

The default input root is `xWalkHal/xWalkIW`. That module is not present in the current repository, so
`--check` and `--generate-cpp` cannot succeed with their defaults. The generator is retained as an inactive
development asset; it must not be presented as a working build step until the matching schema module is
restored or explicit valid input paths are supplied.

Generation additionally requires `protoc`, `grpc_cpp_plugin`, the imported Protobuf schemas, and writable
output directories. Generated files belong under the selected module's `auto-gen/include` and `auto-gen/src`
directories and must not be treated as handwritten production sources.

## Device Tree overlay assets

The two `.dtbo` files are compiled Raspberry Pi Device Tree blobs, not executable host tools and not C++
libraries. They are not installed or activated by `setup-rpi.sh`, the CMake install rules, or host CI.

| File | Board role |
|---|---|
| `sunfounder-robothat5.dtbo` | SunFounder Robot HAT 5 boot-time configuration |
| `sunfounder-servohat+.dtbo` | SunFounder Servo HAT+ boot-time configuration |

Never infer Robot HAT v4 from failed v5 discovery. Never activate both overlays: they configure overlapping
I2C, SPI, I2S, and audio resources. Installing or selecting an overlay is separate target-administration work
and requires physical board verification followed by a reboot and hardware acceptance.

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
bash -n xWalkTool/*.sh
xWalkTool/clean-build.sh --dry-run
xWalkTool/run-host-coverage.sh --help
xWalkTool/setup-rpi.sh --help
xWalkTool/provision-hardware.sh --help
xWalkTool/xHal_Rpi5CarIwGenerator --help
bash deployment/test/setup-rpi-test.sh
```

Do not run `setup-rpi.sh --apply`, direct overlay installation, hardware-labelled CTest targets, or actuator
commands during ordinary host verification.
