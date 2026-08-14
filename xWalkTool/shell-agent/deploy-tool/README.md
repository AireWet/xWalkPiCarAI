# Deployment tools

This directory contains reviewed Raspberry Pi provisioning, cross-build audit,
package-installation, service, udev, tmpfiles, and host-test assets for xWalk.
Run commands from the `MyPiCarX` repository root.

Hardware operations are opt-in. Do not run `--apply` or
`provision-hardware.sh` until the exact Raspberry Pi, Robot HAT revision,
wiring, power state, GPIO controller, I2C bus, SPI device, and safe actuator
state have been confirmed.

## Directory contents

| Path | Purpose |
|---|---|
| `setup-rpi.sh` | Assess, validate, preview, or apply Raspberry Pi host provisioning. |
| `provision-hardware.sh` | Record a verified Robot HAT and exact device identities in a writable configuration. |
| `aarch64-dependency-audit.sh` | Validate an ARM64 sysroot and dependency architecture. |
| `debian/` | Debian package metadata. |
| `systemd/` | Installed xWalk service unit and defaults. |
| `udev/` | Device-access rule template. |
| `tmpfiles/` | Runtime-directory configuration. |
| `test/` | Host-safe tests and fixtures; no physical hardware is accessed. |

## Inspect the Raspberry Pi plan

Show the supported options:

```bash
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --help
```

The script defaults to dry-run, but it still requires a real supported
Raspberry Pi identity and matching Robot HAT profile. Preview the exact plan:

```bash
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --profile robot_hat_v5 --runtime-user "$USER" --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0 --dry-run
```

Use `--check` to validate the target without changing it. `--apply` may install
packages, update boot configuration, install service and access-control assets,
and change the target configuration. Review the complete dry-run output before
authorization:

```bash
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --profile robot_hat_v5 --runtime-user "$USER" --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0 --check
```

## Record verified hardware

The hardware provisioner validates the detected HAT identity and GPIO metadata,
then atomically updates an existing writable configuration while preserving its
permissions:

```bash
xWalkTool/shell-agent/deploy-tool/provision-hardware.sh --profile robot_hat_v5 --config /var/lib/xwalk/picar-x.conf --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0
```

After provisioning, run the passive `doctor` or `--diagnose --no-hardware`
flow before any separately authorized calibration or actuator test.

## Audit an ARM64 sysroot

Point the audit at an explicit absolute sysroot. It validates headers,
pkg-config metadata, library search paths, and AArch64 library identities
without operating hardware:

```bash
XWALK_AARCH64_SYSROOT=/absolute/aarch64/sysroot xWalkTool/shell-agent/deploy-tool/aarch64-dependency-audit.sh
```

See [ARM64_CROSS_BUILD.md](ARM64_CROSS_BUILD.md) and
[HARDWARE_INDEPENDENT_READINESS.md](HARDWARE_INDEPENDENT_READINESS.md) for the
complete cross-build and readiness requirements.

## Host-safe verification

These checks use fixtures or temporary directories and do not actuate hardware:

```bash
bash xWalkTool/shell-agent/deploy-tool/test/setup-rpi-test.sh
bash xWalkTool/shell-agent/deploy-tool/test/aarch64-dependency-audit-test.sh
bash xWalkTool/shell-agent/deploy-tool/test/environment-loader-test.sh
bash xWalkTool/shell-agent/deploy-tool/test/language-model-config-test.sh
python3 xWalkTool/shell-agent/deploy-tool/test/dependency-installer-test.py
```

Run ShellCheck over the deployment scripts before review:

```bash
xWalkTool/shell-agent/quality-tool/run-host-shellcheck.sh
```
