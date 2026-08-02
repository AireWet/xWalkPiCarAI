# Hardware Provisioning Script Guide

[`xWalkTool/shell/provision-hardware.sh`](../../../xWalkTool/shell/provision-hardware.sh)
validates a selected Robot HAT profile and Linux device identity, then persists
that identity into one existing writable xWalk
configuration file. It is normally invoked by `setup-rpi.sh` near the end of target provisioning.

The script does not move actuators, claim device outputs, install packages, edit Raspberry Pi boot settings,
or install Device Tree overlays.

## Requirements

- Run on the intended Raspberry Pi after confirming the Robot HAT revision.
- Provide an existing writable configuration file.
- Install `gpiodetect` from the target distribution's GPIO tools.
- Select device paths that exist on the target and match the supported Linux naming forms.

## Arguments

| Argument | Required | Meaning |
| --- | --- | --- |
| `--profile robot_hat_v4` | One profile required | Selects a manually verified v4 board |
| `--profile robot_hat_v5` | One profile required | Selects v5 only when its supported UUID is detected |
| `--config FILE` | Yes | Selects an existing writable configuration file |
| `--gpio-device /dev/gpiochipN` | Conditional | Selects an exact GPIO chip |
| `--i2c-device /dev/i2c-N` | No | Records an exact I2C controller |
| `--spi-device /dev/spidevN.N` | No | Records an exact SPI controller and chip select |
| `--help`, `-h` | No | Prints usage and exits |

When `--gpio-device` is omitted, automatic selection succeeds only if exactly one `/dev/gpiochip*` exists.
Use an explicit value on systems with more than one GPIO controller.

## Inspect before provisioning

```sh
gpiodetect
ls -l /dev/gpiochip* /dev/i2c-* /dev/spidev*
```

Confirm the selected GPIO entry has a usable label and at least 28 lines. Confirm the Pi and HAT revision
physically; device paths alone do not identify the HAT.

## Provision the configuration

```sh
xWalkTool/shell/provision-hardware.sh --profile robot_hat_v4 --config /var/lib/xwalk/picar-x.conf --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0
```

Use the actual profile and devices reported on the target. Robot HAT v5 is rejected unless the supported
Device Tree UUID is present. Robot HAT v4 is rejected when that v5 UUID is detected.

## Configuration keys

The script writes or replaces these keys:

```text
hardware_board
hardware_gpio_device
hardware_gpio_chip_name
hardware_gpio_chip_label
hardware_i2c_device
hardware_spi_device
```

An omitted I2C or SPI option does not guess a device. Existing matching values are blanked, or no new key is
added when the key is absent. Other configuration lines are preserved.

The update is assembled in a temporary file beside the selected configuration, preserves the original file
mode, and replaces the selected file with `mv` after successful generation. A temporary file is removed on
an ordinary trapped exit.

## Validate the result

Run the passive CLI diagnostic before calibration or movement:

```sh
xwalk-picarx-control --deployment-config /var/lib/xwalk/picar-x.conf doctor
```

Check the recorded values directly when troubleshooting:

```sh
sed -n '/^hardware_/p' /var/lib/xwalk/picar-x.conf
```

The script's successful exit confirms configuration provisioning, not physical motor or servo safety.

## Exit behavior

- Status 0 indicates provisioning completed.
- Status 2 indicates invalid arguments, profile conflict, missing tools, invalid device identity, or an
  unusable configuration file.

## Host verification

The repository test uses a temporary simulated target and does not access physical hardware:

```sh
bash -n xWalkTool/shell/provision-hardware.sh
bash xWalkTool/deployment/test/setup-rpi-test.sh
```
