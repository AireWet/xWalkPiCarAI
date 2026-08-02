# Dependency Installer Guide

[`xWalkTool/python/xHal_Rpi5CarDependencyInstaller`](../../../xWalkTool/python/xHal_Rpi5CarDependencyInstaller)
is the executable Python tool for checking and installing the operating-system
packages catalogued in
[`xWalkTool/apt-packages.txt`](../../../xWalkTool/apt-packages.txt). It prints a final result table grouped by
xWalk module or tooling responsibility.

The script can also configure the Raspberry Pi boot files for one locally detected and physically verified
Robot HAT v5. Host mode never reads, copies, or modifies Raspberry Pi boot configuration or overlay files.

## Safety boundary

The script does not start motors, servos, speakers, cameras, microphones, or hardware tests. Package and boot
installation can still change the operating system and may require root privileges. Always run `--check` and
`--dry-run` before installation on a Raspberry Pi.

The script does not:

- infer a Robot HAT revision from a Raspberry Pi or 40-pin header;
- treat failed v5 detection as evidence of Robot HAT v4;
- use the Servo HAT+ overlay as a Robot HAT v4 overlay;
- download Vosk models or install Ollama through an unverified remote command;
- configure runtime users, groups, udev rules, or `/var/lib/xwalk/picar-x.conf`;
- reboot the operating system;
- prove physical hardware safety.

Python 3 is a bootstrap requirement: Python must already be available to run the installer. It is also
recorded as a required package so later status checks and provisioning retain it.

## Package manifest

The beginning of `apt-packages.txt` is a human-readable Debian/Ubuntu package list and xWalk HAL dependency
map. The bounded `XWALK MACHINE-READABLE PACKAGE CATALOG V1` section supplies package mappings for the Python
installer. Do not reorder its pipe-separated fields without updating the parser and tests.

The catalog supports these package families:

| Operating-system family | Package manager |
| --- | --- |
| Debian, Ubuntu, Raspberry Pi OS, Linux Mint, Pop!_OS | APT and `dpkg-query` |
| Fedora, RHEL, CentOS, Rocky Linux, AlmaLinux | DNF and RPM |
| Arch Linux, Manjaro, EndeavourOS | Pacman |
| macOS | Homebrew and Apple Command Line Tools |

An unknown Linux distribution is rejected. An explicit `--os` selects a mapping; it does not emulate that OS
or install a missing package manager. Repository hardware backends require Linux. macOS can install compatible
development tools but reports ALSA and Linux device dependencies as unsupported.

## Device selection

`--device` accepts `auto`, `host`, or `rpi`. The default is `auto`:

- A readable local Device Tree model containing `Raspberry Pi` selects `rpi`.
- Every other supported machine selects `host`.
- `--target` remains an alias for compatibility.
- `--device host` unconditionally skips all `config.txt` and overlay behavior.
- `--device rpi` requires a locally detected Raspberry Pi and a verified profile.

Use an explicit host selection when reviewing or installing workstation dependencies:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device host --check
```

## Actions and scopes

Installation is the default action. The action options are mutually exclusive:

| Option | Behavior |
| --- | --- |
| `--check` | Queries installed state without changing packages or boot files |
| `--dry-run` | Prints planned package and boot changes without applying them |
| `--install` | Explicit spelling of the default installation action |

By default, required, native-audio, quality, release, generator, and external scopes are selected. External
Vosk and Ollama entries are checked and reported but are not automatically installed. Use `--required-only`
for the minimum normal build/runtime selection. Optional scopes can then be added individually with repeated
`--include audio`, `--include quality`, `--include release`, `--include generator`, or `--include external`.

Camera selection accepts `auto`, `none`, `csi`, or `usb`. Raspberry Pi `auto` selects CSI and therefore
`rpicam-apps`; USB selects `ffmpeg`. Host mode selects no camera package automatically.

## Host workflow

Check the minimum packages without changing the host:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device host --required-only --check
```

Preview all supported package scopes:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device host --dry-run
```

Install the minimum packages on Ubuntu:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os ubuntu --device host --required-only
```

Equivalent explicit examples for other supported workstation families are:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os fedora --device host --required-only
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os arch --device host --required-only
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --os macos --device host --required-only
```

Linux installation uses the current root account or prefixes individual package commands with `sudo`.
Homebrew is never executed through `sudo`. Installed packages are skipped and absent mapped packages are sent
to the selected package manager in one installation request.

## Raspberry Pi Robot HAT v5 workflow

Raspberry Pi boot configuration is supported only on the APT-based Raspberry Pi OS and Ubuntu target covered
by the repository deployment architecture. The administrator must physically confirm Robot HAT v5 and supply
`--profile robot_hat_v5`. The script also requires local Device Tree UUID
`9daeea78-0000-076e-0032-582369ac3e02`; the option cannot override failed detection.

First inspect current state:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device rpi --profile robot_hat_v5 --camera csi --required-only --check
```

Then review every planned change:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device rpi --profile robot_hat_v5 --camera csi --required-only --dry-run
```

Apply only on the verified target:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device rpi --profile robot_hat_v5 --camera csi --required-only
```

Before applying boot changes, the script verifies:

1. The OS uses the supported APT path.
2. The local model is a Raspberry Pi.
3. The v5 profile was explicitly selected.
4. The supported Robot HAT v5 UUID is locally exposed.
5. The bundled overlay is readable and has its recorded SHA-256 checksum.
6. A supported boot configuration and overlay directory pair exists.
7. I2C and SPI are not explicitly disabled.
8. The overlapping `sunfounder-servohat+` overlay is not active.

The supported boot layouts are:

| Configuration | Overlay directory |
| --- | --- |
| `/boot/firmware/config.txt` | `/boot/firmware/overlays` |
| `/boot/config.txt` | `/boot/overlays` |

Before its first modification, the installer preserves `config.txt.xwalk-backup`. If a different destination
`sunfounder-robothat5.dtbo` already exists, it preserves
`sunfounder-robothat5.dtbo.xwalk-backup`. Existing backups are not overwritten.

The source blob is installed as `sunfounder-robothat5.dtbo`. Missing settings are appended in an explicit
global section so a preceding model-specific section cannot accidentally contain them:

```text
[all]
# xWalk Robot HAT v5 configuration
dtparam=i2c_arm=on
dtparam=spi=on
dtoverlay=sunfounder-robothat5
```

Already valid settings and a matching destination overlay are retained. The operation does not reboot.

## Robot HAT v4 limitation

The repository contains Robot HAT v5 and Servo HAT+ assets. It does not contain a verified Robot HAT v4
overlay. `--device rpi --profile robot_hat_v4` therefore exits before package or boot modification. This is an
intentional safety failure, not an instruction to install `sunfounder-servohat+.dtbo`.

Use [`setup-rpi.sh`](../../../xWalkTool/shell/setup-rpi.sh) for the separately documented v4 deployment profile and
leave overlay selection under verified target administration until a genuine v4 asset and validation evidence
are added.

## Result table and exit status

The final table contains these columns:

| Column | Meaning |
| --- | --- |
| Symbol | `✓` means installed/valid; `✗` means missing, failed, external, planned, or unsupported |
| Module/submodule | xWalk consumer or tooling responsibility |
| Dependency | Logical manifest dependency or Raspberry Pi boot requirement |
| Package | Selected package name, external component, or exact boot path |
| State / issue | Existing, newly installed, planned, unsupported, or failure detail |

Exit statuses are:

| Status | Meaning |
| --- | --- |
| `0` | Selected check/install completed with every selected row valid, or a valid dry-run was produced |
| `1` | One or more selected packages or boot requirements remain unavailable or failed |
| `2` | Arguments, OS, device, HAT identity, or required selection is invalid |

The default all-scope installation can return `1` after package installation when optional external Vosk or
Ollama components remain unavailable. Use `--required-only` when validating the minimum deployment packages.

## Complete Raspberry Pi deployment sequence

Package and overlay installation is only one part of deployment:

1. Physically verify the Raspberry Pi and Robot HAT revision.
2. Run the dependency installer with `--check` and `--dry-run`.
3. Apply the reviewed Robot HAT v5 dependency and boot changes.
4. Reboot the Raspberry Pi.
5. Run `setup-rpi.sh --dry-run` and then its reviewed `--apply` operation.
6. Refresh the runtime-user login session if group membership changed.
7. Run the non-moving CLI `doctor` command.
8. Complete the documented raised-wheel and hardware-in-the-loop acceptance procedure.

The dependency installer does not replace `setup-rpi.sh`: setup owns runtime groups, udev permissions,
device-node selection, and mutable deployment configuration.

## Host verification

These checks do not install packages or access hardware:

```sh
python3 -m py_compile xWalkTool/python/xHal_Rpi5CarDependencyInstaller xWalkTool/deployment/test/dependency-installer-test.py
python3 xWalkTool/deployment/test/dependency-installer-test.py
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --device host --required-only --check
ctest --test-dir build-host/sanity --output-on-failure -R xHal_Rpi5CarDependencyInstallerHostTest
```

The host tests validate package-catalog coverage, comment handling, boot-section scoping, expected overlay
checksum/configuration recognition, and refusal to substitute Servo HAT+ for Robot HAT v4. They do not prove
that the Raspberry Pi boots or that the physical Robot HAT is safe.

## Troubleshooting

- **Package query executable unavailable:** install or select the correct package manager for the actual OS.
- **Unsupported Linux distribution:** add reviewed catalog mappings and tests before enabling installation.
- **Raspberry Pi not detected:** do not force boot changes from a workstation or container.
- **Robot HAT v5 UUID missing:** confirm the physical board and EEPROM/Device Tree state; do not bypass it.
- **v4 overlay error:** no verified v4 blob exists; never substitute the Servo HAT+ blob.
- **I2C or SPI disabled:** resolve the existing `dtparam=...=off` entry manually before rerunning.
- **Servo HAT+ conflict:** remove only after verifying the installed physical board and resource ownership.
- **Checksum mismatch:** do not install the blob until its source, license, and expected checksum are reviewed.
- **Installation failure:** read the final package/module row and the package manager's last diagnostic.
- **Reboot required:** reboot, inspect device nodes, run `doctor`, and perform hardware acceptance.
