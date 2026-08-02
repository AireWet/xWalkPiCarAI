# Device Tree Overlay Assets Guide

`xWalkTool/dtoverlays` contains compiled Raspberry Pi Device Tree overlay blobs associated with SunFounder
boards. They are binary boot-configuration assets, not shell scripts, runtime libraries, or Git metadata.

## Files

| File | Intended board role |
| --- | --- |
| `sunfounder-robothat5.dtbo` | SunFounder Robot HAT 5 boot-time configuration |
| `sunfounder-servohat+.dtbo` | SunFounder Servo HAT+ boot-time configuration |

The files are kept as source-development assets. Current CMake installation rules, host CI,
`setup-rpi.sh`, and `provision-hardware.sh` do not install or activate them.

## Safety restrictions

- Physically identify the attached board before selecting an overlay.
- Do not infer Robot HAT v4 because v5 detection failed.
- Do not enable the Robot HAT 5 and Servo HAT+ overlays together.
- Inspect existing boot configuration for resource conflicts before any target change.
- Preserve the target boot configuration and keep a recoverable backup.
- Reboot only after reviewing the complete configuration change.
- Perform hardware acceptance after any overlay or kernel change.

The overlays can configure overlapping I2C, SPI, I2S, and audio resources. An incorrect selection can prevent
devices from appearing correctly and can invalidate GPIO, audio, or peripheral assumptions.

## Safe host inspection

Identify the files without installing them:

```sh
file xWalkTool/dtoverlays/*.dtbo
sha256sum xWalkTool/dtoverlays/*.dtbo
```

If the Device Tree compiler is installed, render a blob for review without modifying the target:

```sh
dtc -I dtb -O dts xWalkTool/dtoverlays/sunfounder-robothat5.dtbo
```

Warnings from decompilation require review and are not evidence that an overlay is safe for the attached
board. Host inspection cannot verify electrical compatibility or runtime behavior.

## Target use boundary

There is intentionally no automatic installation command in this guide. Overlay installation changes the
Raspberry Pi boot configuration and requires all of the following first:

1. Physical confirmation of the exact HAT revision.
2. Review of the target Raspberry Pi model and operating-system overlay mechanism.
3. Review of existing I2C, SPI, I2S, audio, GPIO, and other overlays.
4. A recovery method for an unbootable or incorrectly configured target.
5. Explicit authorization to modify the target boot partition.

After an authorized target administrator installs and selects the verified overlay, reboot the Raspberry Pi,
run the non-moving `doctor` command, inspect the expected device nodes, and complete the documented physical
hardware-acceptance procedure before any ordinary movement test.

```sh
xwalk-picarx-control --deployment-config /var/lib/xwalk/picar-x.conf doctor
```

Until Raspberry Pi and hardware-in-the-loop checks pass, treat these blobs as unverified target assets.
