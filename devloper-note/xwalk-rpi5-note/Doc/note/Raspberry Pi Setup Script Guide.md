# Raspberry Pi Setup Script Guide

This guide explains how to use
[Raspberry Pi setup script](../../../../xWalkTool/shell-agent/deploy-tool/setup-rpi.sh) to inspect,
plan, validate, and provision a Raspberry Pi for xWalk. The script prepares operating-system dependencies and
device access. It does not prove that actuators are wired correctly or physically safe.

## Safety boundary

The default mode is `--dry-run`. It reports planned changes and does not modify the target. Run dry-run and
review its output before authorizing `--apply`.

The script does not:

- start motors or servos;
- claim GPIO, I2C, SPI, PWM, or actuator outputs;
- install or change a Robot HAT Device Tree overlay;
- infer Robot HAT v4 when v5 detection fails;
- download or install Vosk models, Ollama, or Ollama models;
- perform physical hardware acceptance.

Do not run `--apply` on a development host. Use it only on the intended Raspberry Pi after identifying the
Pi model, Robot HAT revision, runtime user, and exact device nodes.

## Supported target

The script accepts Debian, Raspberry Pi OS, and Ubuntu only when `/proc/device-tree/model` identifies a
Raspberry Pi. It locates either `/boot/firmware/config.txt` or `/boot/config.txt` and refuses to continue if
neither exists.

Run the source-tree command from the repository root:

```sh
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --help
```

After installation, the equivalent script is normally `/usr/lib/xwalk/setup-rpi.sh`.

## Hardware defaults and overrides

| Argument | Default | Meaning |
| --- | --- | --- |
| `--profile robot_hat_v4\|robot_hat_v5` | `robot_hat_v4` | Selects the Robot HAT profile |
| `--runtime-user USER` | `xwalk` | Selects an existing unprivileged account that will run xWalk |
| `--gpio-device /dev/gpiochipN` | `/dev/gpiochip4` | Selects one exact GPIO controller |
| `--i2c-device /dev/i2c-N` | `/dev/i2c-1` | Selects the exact I2C controller |
| `--spi-device /dev/spidevN.N` | `/dev/spidev0.0` | Selects the exact SPI controller and chip select |
| `--camera csi\|usb` | `csi` | Selects the camera connection and package family |

The authoritative values are in `rpi-defaults.conf` and are shared with CMake
and runtime-generation scripts. Explicit options override them. A v4 profile
conflicts with a detected v5 UUID. A v5 profile is rejected unless UUID
`9daeea78-0000-076e-0032-582369ac3e02` is present.

The script does not create the runtime user. Create and review the dedicated account separately before
running setup.

## Optional arguments and defaults

| Argument | Default | Meaning |
| --- | --- | --- |
| `--config FILE` | `/var/lib/xwalk/picar-x.conf` | Selects the writable runtime configuration |
| `--template-config FILE` | `/etc/xwalk/picar-x.conf` | Selects a read-only manifest template |
| `--template-fragments DIRECTORY` | `/etc/xwalk/picar-x.d` | Selects read-only fragment templates |
| `--with-vosk` | Disabled | Installs explicitly supplied repository-controlled Vosk assets |
| `--vosk-library-source FILE` | Required with Vosk | Selects the architecture-matched library source |
| `--vosk-model-source DIRECTORY` | Required with Vosk | Selects the model source directory |
| `--validate-ollama` | Disabled | Validates the selected user's installed executable and model manifest |
| `--help`, `-h` | Not applicable | Prints usage without changing the system |

The Vosk option copies only the explicitly supplied repository-controlled assets
to `/usr/lib/xwalk` and `/usr/share/xwalk/models/vosk`. Ollama installation is
owned by `setup-rpi-local.sh`; `--validate-ollama` performs no installation and
does not contact the service.

## Operating modes

| Mode | System changes | Result |
| --- | --- | --- |
| `--dry-run` | None | Prints the provisioning plan; this is the default |
| `--check`, `--validate` | None | Validates required packages, interfaces, devices, and configuration |
| `--apply` | Privileged | Performs the reviewed provisioning plan |

If more than one mode is supplied, the last mode on the command line takes effect. Use exactly one mode to
avoid ambiguity.

## Recommended workflow

### 1. Identify the hardware

Confirm the Raspberry Pi model and Robot HAT revision physically. A Raspberry Pi or a 40-pin connector is
not evidence of the HAT revision.

Inspect available device nodes without claiming them:

```sh
gpiodetect
ls -l /dev/gpiochip* /dev/i2c-* /dev/spidev*
```

Missing globs can cause `ls` to report an error; that output indicates the corresponding interface may not
be enabled or available. Do not guess a GPIO controller when more than one is listed.

### 2. Review the dry-run plan

When the target matches all reviewed defaults, preview the plan with:

```sh
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --dry-run
```

The dry run reports:

- detected operating system and Raspberry Pi model;
- selected Robot HAT profile and device nodes;
- missing required packages;
- missing I2C or SPI boot settings;
- planned groups and runtime-user membership;
- planned configuration and narrowly scoped udev rule;
- optional Vosk or Ollama availability when requested;
- confirmation that Robot HAT overlays will remain unchanged.

### 3. Apply the reviewed plan

Only an administrator should run apply mode. The script uses the current root account or invokes `sudo` for
individual privileged commands when available:

```sh
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --apply --profile robot_hat_v4 --runtime-user xwalk --gpio-device /dev/gpiochip4
```

Do not add a Robot HAT overlay merely to make profile validation succeed. Robot HAT v5 requires its already
verified supported UUID. Robot HAT v4 remains a manual hardware selection.

### 4. Reboot or refresh the login session

Reboot if setup enabled I2C or SPI. Log out and back in after group membership changes so the runtime user
receives the new supplementary groups.

### 5. Validate the resulting state

Run the same selection in check mode:

```sh
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --check
```

Then inspect identity and permissions:

```sh
id xwalk
groups xwalk
stat /dev/i2c-1 /dev/gpiochip4 /dev/spidev0.0
getfacl /dev/i2c-1 /dev/gpiochip4 /dev/spidev0.0
udevadm verify /etc/udev/rules.d/99-xwalk-picarx.rules
```

Finally run the passive xWalk diagnostic before calibration or movement:

```sh
xwalk-picarx-control --deployment-config /var/lib/xwalk/picar-x.conf doctor
```

`doctor` is non-moving, but successful host or passive device checks are not a substitute for the raised-wheel
hardware acceptance procedure in the [Deployment Guide](Deployment%20Guide.md).

## Changes made by apply mode

Apply mode can perform these operations:

1. Install missing required Debian packages with `apt-get`.
2. Append `dtparam=i2c_arm=on` or `dtparam=spi=on` when the setting is absent.
3. Create the system groups `xwalk`, `i2c`, `gpio`, and `spi` when absent.
4. Add the runtime user to those groups and to existing `audio`, `video`, and `render` groups.
5. Create the selected configuration directory with owner `root:xwalk` and mode `0770`.
6. Create the runtime manifest and `picar-x.d` fragments from the selected templates when absent.
7. Set the manifest to `0660` and included configuration files to `0640`, owned by `root:xwalk`.
8. Install `/etc/udev/rules.d/99-xwalk-picarx.rules` for only the selected device names.
9. Reload udev rules and trigger the I2C, GPIO, and SPI subsystems.
10. Run `provision-hardware.sh` when it is executable and the selected GPIO device exists.

Package installation is skipped when every required package is already installed. Existing enabled boot
settings and system groups are not duplicated. User-group addition is repeatable. The selected udev rule is
regenerated from the repository template.

If the boot configuration explicitly contains `dtparam=i2c_arm=off` or `dtparam=spi=off`, the script stops
and requires an administrator to resolve the conflict manually.

## Required packages

The common package set is:

```text
build-essential cmake ninja-build pkg-config python3 python3-nacl linux-libc-dev
libasound2-dev alsa-utils libatomic1
libcurl4-openssl-dev libsndfile1-dev libprotobuf-dev libgrpc++-dev libgtest-dev libjson-c-dev libtinyxml2-dev
libyaml-cpp-dev libopencv-dev libboost-dev i2c-tools libi2c-dev gpiod
espeak-ng libttspico-utils curl ca-certificates
```

CSI camera selection additionally requires `rpicam-apps`. USB camera selection instead requires `ffmpeg`.

## Configuration behavior

The default mutable configuration is `/var/lib/xwalk/picar-x.conf`. During apply
mode, the script copies a missing manifest and missing `picar-x.d` fragment tree
once from the installed administrator templates under `/etc/xwalk`.
Existing mutable configuration is retained rather than replaced from the template.

Hardware provisioning records the selected board profile, GPIO device, GPIO chip name and label, I2C device,
and SPI device. Provisioning is deferred when the selected GPIO device is not yet available. Run setup again
after reboot and confirm the resulting values with `doctor`.

## Exit status and troubleshooting

| Status | Meaning |
| --- | --- |
| `0` | Requested plan, validation, or provisioning completed |
| `1` | Check mode found one or more missing required items |
| `2` | Arguments, platform, profile, configuration, or a required tool were invalid |

Common failures:

- **Unsupported operating system:** run the script on a supported Debian-family Raspberry Pi target.
- **Not a Raspberry Pi:** do not apply target provisioning on the host workstation.
- **v5 UUID missing:** verify the physical board and existing target configuration; do not force an overlay.
- **GPIO path invalid:** select a path reported by `gpiodetect`.
- **Configuration missing:** install the package template before apply mode.
- **Device missing after apply:** reboot when an interface was enabled, then rerun `--check`.
- **Permission denied:** refresh the user login session and inspect exact device ownership and udev results.

## Host-side script verification

These commands check script syntax and provisioning behavior without accessing Raspberry Pi hardware:

```sh
bash -n xWalkTool/shell-agent/deploy-tool/setup-rpi.sh xWalkTool/shell-agent/deploy-tool/provision-hardware.sh
bash xWalkTool/shell-agent/deploy-tool/test/setup-rpi-test.sh
```

Do not run `--apply`, hardware-labelled CTest tests, or actuator commands during ordinary host verification.
