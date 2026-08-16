# Dependency Installer Script Flags

This reference documents every command-line flag accepted by
[dependency installer](../../../../xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller).
The separate [CMake Dependency Guide](Dependency%20Installer%20Guide.md) explains the workspace's CMake
configure-time and link-time dependency requirements.

## Command form

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller [OPTIONS]
```

Installation is the default action when `--check`, `--dry-run`, and `--install` are all omitted. Always use
`--check` and `--dry-run` before allowing installation or Raspberry Pi boot changes.

## Flag summary

| Flag | Value | Default | Purpose |
| --- | --- | --- | --- |
| `-h`, `--help` | None | Not applicable | Prints generated command help and exits |
| `--manifest` | File path | `xWalkTool/apt-packages.txt` | Selects the machine-readable package catalog |
| `--os` | OS name or `auto` | `auto` | Detects or selects the package-manager mapping |
| `--device` | `auto`, `host`, or `rpi` | `auto` | Selects workstation or Raspberry Pi behavior |
| `--target` | `auto`, `host`, or `rpi` | `auto` | Compatibility alias for `--device` |
| `--profile` | `robot_hat_v4` or `robot_hat_v5` | None | Supplies a physically verified Robot HAT profile |
| `--camera` | `auto`, `none`, `csi`, or `usb` | `auto` | Selects Raspberry Pi camera dependencies |
| `--include` | Optional scope | None | Adds one optional dependency scope; repeatable |
| `--required-only` | None | Disabled | Limits selection to required packages and explicit includes |
| `--check` | None | Disabled | Reports current state without installing or changing boot files |
| `--dry-run` | None | Disabled | Prints planned package and boot changes without applying them |
| `--install` | None | Implicit default | Explicitly requests installation and eligible boot changes |

## `-h` and `--help`

Print the parser-generated usage text and exit without reading the package catalog, querying packages, or
changing the system.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --help
```

## `--manifest PATH`

Select a package catalog instead of the default `xWalkTool/apt-packages.txt`. The path is resolved before it
is read. The file must contain a valid bounded `XWALK MACHINE-READABLE PACKAGE CATALOG V1` section with at
least one matching package record.

This option is primarily for controlled validation and testing. Supplying an unreviewed catalog can change
which packages are queried or installed.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --manifest xWalkTool/apt-packages.txt --check
```

## `--os NAME`

Select the package-family mapping. The default `auto` detects macOS through the platform name or reads Linux
distribution identifiers from `/etc/os-release`.

Common accepted names include:

| Family | Accepted examples | Package tools |
| --- | --- | --- |
| Debian | `debian`, `ubuntu`, `raspbian`, `linuxmint`, `pop`, `elementary` | APT and `dpkg-query` |
| Fedora | `fedora`, `rhel`, `centos`, `rocky`, `almalinux`, `ol` | DNF and RPM |
| Arch | `arch`, `manjaro`, `endeavouros` | Pacman |
| macOS | `macos`, `darwin`, `osx` | Homebrew and Apple Command Line Tools |

The special value `linux` reads the local `/etc/os-release` instead of selecting a family directly. An
explicit OS name changes package mapping only; it does not emulate that operating system or provide a missing
package manager. Unknown distributions are rejected.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --os ubuntu --device host --required-only --check
```

## `--device MODE` and `--target MODE`

Both spellings write the same device selection. `--target` is retained as a compatibility alias.

| Mode | Behavior |
| --- | --- |
| `auto` | Selects `rpi` only when the local Device Tree identifies a Raspberry Pi; otherwise selects `host` |
| `host` | Selects workstation packages and prevents Raspberry Pi boot configuration behavior |
| `rpi` | Selects Raspberry Pi packages and requires local Raspberry Pi and Robot HAT validation |

`--device rpi` is accepted only on a locally detected Raspberry Pi using the supported APT path. It also
requires `--profile`. It cannot be used to prepare boot files from a workstation or container.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device host --check
```

## `--profile PROFILE`

Declare the Robot HAT revision that was physically verified by the operator. This flag is valid only with
`--device rpi` and is required in that mode.

| Profile | Current behavior |
| --- | --- |
| `robot_hat_v5` | Requires the supported local HAT UUID before package or boot processing continues |
| `robot_hat_v4` | Rejected because the repository has no verified Robot HAT v4 overlay |

The flag never overrides failed board detection. The Servo HAT+ overlay is not accepted as a Robot HAT v4
substitute.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device rpi --profile robot_hat_v5 --check
```

## `--camera MODE`

Select optional camera packages for a Raspberry Pi target.

| Mode | Raspberry Pi behavior |
| --- | --- |
| `auto` | Selects CSI camera dependencies |
| `none` | Adds no camera dependency scope |
| `csi` | Selects the `camera-csi` scope and its `rpicam-apps` mapping |
| `usb` | Selects the `camera-usb` scope and its `ffmpeg` mapping |

Host mode adds no camera scope, even when `csi` or `usb` is supplied. Camera selection installs supporting
software only; it does not prove that a camera is connected or safe to access.

## `--include SCOPE`

Add an optional package scope. The flag may be repeated, and each value must be one of:

- `audio`
- `quality`
- `release`
- `generator`
- `external`

Without `--required-only`, every optional scope is already selected, so repeating `--include` does not widen
the default selection. Combine the flag with `--required-only` to add specific capabilities to the minimum
required set.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device host --required-only --include quality --include generator --check
```

The `external` scope reports dependencies such as Vosk and Ollama but does not automatically install them.

## `--required-only`

Select only the mandatory `required` scope, plus scopes explicitly added with `--include` and any applicable
Raspberry Pi camera scope. Use this flag for the minimum normal build and runtime dependency set.

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device host --required-only --dry-run
```

## Action flags

`--check`, `--dry-run`, and `--install` are mutually exclusive. Supplying more than one is an argument error.

### `--check`

Query the selected package manager and, for a validated Raspberry Pi target, inspect the boot configuration
and installed overlay. It does not install packages or change boot files.

### `--dry-run`

Print the package-manager commands that would run and mark eligible boot changes as planned. It performs the
same identity, platform, schema, checksum, and conflict validation required by installation but does not
apply the changes.

### `--install`

Install missing package-managed dependencies and apply eligible Robot HAT v5 boot changes. This is also the
behavior when no action flag is supplied. Linux installation uses the current root account or `sudo`;
Homebrew is never run through `sudo`.

Installed packages are skipped. External Vosk and Ollama dependencies are reported but not automatically
installed. Raspberry Pi boot configuration is changed only after all required local checks pass.

## Flag combinations

| Combination | Result |
| --- | --- |
| `--device host --profile robot_hat_v5` | Rejected because profiles apply only to Raspberry Pi mode |
| `--device rpi` without `--profile` | Rejected before package or boot changes |
| `--device rpi --os macos` | Rejected because Raspberry Pi mode requires supported Linux and APT |
| `--required-only --include quality` | Selects required and quality scopes |
| `--include quality` without `--required-only` | Valid, but quality was already included by default |
| `--check --dry-run` | Rejected because action flags are mutually exclusive |
| No action flag | Performs the same installation action as `--install` |

## Exit status

| Status | Meaning |
| --- | --- |
| `0` | The selected operation succeeded, or a supported dry-run plan was produced |
| `1` | A dependency or boot requirement failed, or OS/catalog validation raised an error |
| `2` | Argument parsing, a device/profile combination, or the selected package set is invalid |

The default all-scope action can return status `1` when an external dependency is unavailable. Use
`--required-only` when checking the minimum package set.

## Safe starting commands

Inspect a development host without changing it:

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device host --required-only --check
```

Preview the complete host package selection:

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device host --dry-run
```

On a physically verified Raspberry Pi with Robot HAT v5, inspect before planning any changes:

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device rpi --profile robot_hat_v5 --camera csi --required-only --check
```

No command in this reference runs a hardware test or moves an actuator.
