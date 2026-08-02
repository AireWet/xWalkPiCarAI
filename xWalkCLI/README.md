# xWalk CLI

`xWalkCLI` is the standalone command-line application aggregate. It composes the sibling `xWalkAgent`
coordinators with the `xWalkController` parser, command dispatcher, guarded boot, and Raspberry Pi executable.

The directory contains only the controller module. Agent coordinators remain owned by the sibling
`xWalkAgent` aggregate and are imported through CMake when required.

## Layout

| Path | Responsibility |
| --- | --- |
| `CMakeLists.txt` | CLI aggregate options and Agent dependency composition |
| `xWalkController/` | CLI parsing, commands, tests, and Raspberry Pi application |

## Host verification

```bash
cmake --fresh --preset sanity
cmake --build --preset sanity --parallel
ctest --preset sanity
```

## Raspberry Pi compilation and test discovery

The repository provides a dry-run-first setup tool for required packages,
interfaces, device groups, and exact-node permissions. After installing the
RPi build, preview setup with an explicit board profile and GPIO controller:

```bash
/usr/lib/xwalk/setup-rpi.sh --dry-run --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
```

Review the plan and repeat it with `--apply`. Use `--camera usb` for a USB V4L2
webcam. The wake-word command also requires an approved `libvosk.so` and unpacked
Vosk model deployment. Configure model paths and ALSA device names in the active
`/var/lib/xwalk/picar-x.conf` before running the command.
The SPI command additionally requires an enabled `/dev/spidev*` node configured
with the target peripheral's mode, clock speed, and bits per word.

```bash
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

Install the complete deployment layout after a successful build:

```bash
DESTDIR="$PWD/build-host/deploy" cmake --install build-host/cmake
```

See the [deployment guide](../DevloperNote/Doc/note/Deployment%20Guide.md) for the installed
paths, package list, Robot HAT revision safeguards, and permission policy.

The last command lists hardware tests without executing them. Do not execute those tests until the correct
Raspberry Pi and Robot HAT are connected and the robot has been placed in a safe test configuration.
