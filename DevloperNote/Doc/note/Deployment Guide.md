# Raspberry Pi Deployment Guide

xWalk supports a native Raspberry Pi build on current Debian-family Raspberry Pi OS and Ubuntu Server.
The exact Raspberry Pi model, Robot HAT revision, GPIO identity, audio devices, camera, and attached SPI
peripheral must still be verified on the target. A source build is not proof that the robot is plug-and-run.

## Dependencies

Required build and runtime components are a C++ compiler, CMake, Ninja, ALSA, libcurl, libsndfile, I2C tools,
the Linux I2C/SPI/GPIO interfaces, Espeak NG, and `gpiod`. Camera commands use either `rpicam-apps` for CSI or
`ffmpeg` for USB. The reviewed Vosk runtime and model are installed from
`xWalkLibrary`; Ollama remains optional and is never installed unless its setup
option is selected.

Inspect the target without changing it:

```sh
xWalkTool/shell/setup-rpi.sh --check --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
xWalkTool/shell/setup-rpi.sh --dry-run --profile robot_hat_v4 --runtime-user pi --gpio-device /dev/gpiochip0
```

Use the real profile and GPIO device. The script detects Raspberry Pi OS or Ubuntu, the Pi model, and the
applicable `/boot/firmware/config.txt` or `/boot/config.txt`. It reports all privileged changes before
`--apply`, adds no duplicate setting or group membership, and refuses an unverified Robot HAT v5 profile.
It never changes a Robot HAT overlay. A 40-pin header does not identify a HAT revision; v4 is always selected
manually, while v5 requires its supported Device Tree UUID.

`setup-rpi.sh` remains overlay-neutral. Separately, `xHal_Rpi5CarDependencyInstaller --device rpi` can install the
bundled Robot HAT v5 overlay only after an explicit `--profile robot_hat_v5` selection and local detection of
the supported UUID. No verified Robot HAT v4 overlay is bundled; the Servo HAT+ asset is not used as a v4
substitute. Review this operation first with `--dry-run`.

After reviewing the plan, an administrator may run the same command with `--apply`. Do not prefix inspection
or dry-run commands with `sudo`; use privilege only for the reviewed apply operation.

## Clean builds and verification

Run CMake from the repository root. The checked-in `CMakePresets.json` requires
CMake 3.25 or newer and Ninja. List the available configure presets with:

```sh
cmake --list-presets
```

The root presets keep every generated file under `build-host/` or
`build-rpi/`. `--fresh` discards the selected build directory's old CMake cache
before configuring it; it does not delete source files.

Use the strict host configuration for ordinary development verification:

```sh
cmake --fresh --preset sanity
cmake --build --preset sanity --parallel
ctest --preset sanity
```

Use the host Debug preset when the additional sanity warnings are not required:

```sh
cmake --fresh --preset host-debug
cmake --build --preset host-debug --parallel
ctest --preset host-debug
```

Use the host Release preset to verify optimized code while retaining test
assertions:

```sh
cmake --fresh --preset host-release
cmake --build --preset host-release --parallel
ctest --preset host-release
```

The specialised host presets use independent caches and must not share a build
directory:

```sh
cmake --fresh --preset clang-tidy
cmake --build --preset clang-tidy --parallel
cmake --build build-host/clang-tidy --target cppcheck
cmake --fresh --preset sanitizers
cmake --build --preset sanitizers --parallel
ctest --preset sanitizers
cmake --fresh --preset coverage
cmake --build --preset coverage --parallel
ctest --preset coverage
```

Use `xWalkTool/shell/run-host-coverage.sh run` after the coverage test when `gcovr` is
available. ThreadSanitizer has its own `thread-sanitizer` configure, build, and
test presets and must not be combined with AddressSanitizer.

Run the native Raspberry Pi Release configuration on the target Raspberry Pi
or in an approved matching ARM build environment:

```sh
cmake --fresh --preset rpi-release
cmake --build --preset rpi-release --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

The last command only lists physical tests. Never run them until the correct target is attached and secured.
The RPi configuration requires the target development packages, including ALSA,
libcurl, and libsndfile. It also enables Debian packaging but does not install a
package or access hardware during compilation.

The preset-to-directory mapping is:

| Preset | Generated directory |
| --- | --- |
| `host-debug` | `build-host/cmake/` |
| `host-release` | `build-host/release/` |
| `sanity` | `build-host/sanity/` |
| `clang-tidy` | `build-host/clang-tidy/` |
| `sanitizers` | `build-host/sanitizers/` |
| `thread-sanitizer` | `build-host/thread-sanitizer/` |
| `coverage` | `build-host/coverage/` |
| `rpi-release` | `build-rpi/cmake/` |

If presets cannot be used, configure the same host sanity build explicitly:

```sh
cmake --fresh -S . -B build-host/sanity -G Ninja -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DXWALK_BUILD_RPI=OFF -DXWALK_ENABLE_STRICT_WARNINGS=ON
cmake --build build-host/sanity --parallel
ctest --test-dir build-host/sanity --output-on-failure --no-tests=error
```

Configure the same native Raspberry Pi Release build explicitly with:

```sh
cmake --fresh -S . -B build-rpi/cmake -G Ninja -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DXWALK_BUILD_RPI=ON -DXWALK_ENABLE_PACKAGING=ON
cmake --build build-rpi/cmake --parallel
ctest --test-dir build-rpi/cmake -N -L hardware
```

Host Release verification keeps assertions active even though the compiler defines `NDEBUG`; this prevents
existing state-changing test assertions from disappearing. New tests should still evaluate an operation
before asserting its result.

The `Host quality` workflow runs GCC and Clang Debug and Release verification, sanitizers, static analysis,
coverage generation, shell validation, provisioning tests, and staged-install checks. Hardware-labelled
tests are never run by this workflow.

Each compiler and build-type matrix entry also runs the CLI centralized controller suite and the complete
CLI controller-to-HAL sequence suite as explicit GitHub Actions steps after aggregate CTest verification.

The workflow also runs ThreadSanitizer in its own build, repeats the complete host suite under load, rejects
unsafe installed file permissions, and retains a checksum manifest for the staged installation. See
[Host Production Readiness Work](Host%20Production%20Readiness%20Work.md) for local commands and the evidence
boundary.

Use `xWalkTool/shell/run-host-coverage.sh run` for foreground-only coverage. The script does not create a detached
process, install packages, or request privileges.

## Installed layout and staging

Project libraries are linked statically into the CLI, so the runtime package installs the executable and its
resources rather than separate project shared objects.

| Path | Purpose |
| --- | --- |
| `/usr/bin/xwalk-picarx-control` | CLI executable |
| `/usr/lib/xwalk/xWalkTool/shell/xWalkEnv.sh` | Authenticated licence environment loader |
| `/usr/lib/xwalk/xWalkTool/python/xWalkLicenseTool` | Licence encryption and decryption tool |
| `/usr/lib/xwalk/xWalkTool/environment/xWalkLicense.cfg` | Empty model-selection input template |
| `/etc/xwalk/picar-x.conf` | Administrator-controlled configuration manifest |
| `/etc/xwalk/picar-x.d/` | Functional defaults and separate AI-provider profiles |
| `/var/lib/xwalk/picar-x.conf` | Writable active manifest, created once by setup |
| `/var/lib/xwalk/picar-x.d/` | Active functional and AI configuration fragments |
| `/var/cache/xwalk/` | Runtime cache |
| `/run/xwalk/` | Volatile runtime state |
| `/usr/share/xwalk/config/` | Immutable v4 and v5 profile templates |
| `/usr/share/xwalk/sounds/` | Packaged sounds |
| `/usr/share/xwalk/music/` | Packaged music |
| `/usr/share/doc/xwalk/audio-resources/README.md` | Combined audio provenance and integrity hashes |
| `/usr/share/doc/xwalk/` | Deployment documentation |

Test the exact package layout without modifying the development host:

```sh
cmake --fresh -S . -B build-host/cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build-host/cmake --parallel
DESTDIR="$PWD/build-host/deploy" cmake --install build-host/cmake
find build-host/deploy -type f -o -type l
```

Do not configure the prefix as `build-host/usr`; `DESTDIR` preserves `/usr`, `/etc`, and `/var` correctly.
Packaged sound paths are resolved below the CMake-defined data directory. Relative CLI sound paths use the
resource root; `--resource-directory /absolute/path` provides a development/test override. Required files
must be readable regular files. A missing safety sound rejects its dependent action before motor movement.

## Debian package

Configure the RPi preset, build it, then create and inspect the package without installing it:

```sh
cd build-rpi/cmake
cpack -G DEB
dpkg-deb --contents xwalk-picarx_*.deb
lintian xwalk-picarx_*.deb
```

The package installs under normal system paths. The manifest and every file below
`/etc/xwalk/picar-x.d` are Debian conffiles and are not silently replaced on
upgrade. Runtime state remains under `/var`, never `/usr/share`.

## Runtime user and device permissions

Use a dedicated `xwalk` service account. Debian-family images commonly provide `i2c`, `gpio`, `spi`, `audio`,
`video`, and sometimes `render`; setup detects whether a group exists before using it. `render` is needed only
for camera/media acceleration. Distribution group names can differ, so inspect the target rather than assume.

The optional udev template grants `0660` only to explicitly provisioned I2C, GPIO, and SPI kernel device
names. It contains no `0666`, USB wildcard, all-GPIO rule, or all-video rule. Audio and camera devices keep
their distribution rules because ordinary group membership is sufficient.

Validate the actual account and exact nodes after login or reboot:

```sh
id xwalk
groups xwalk
stat /dev/i2c-1 /dev/gpiochip0 /dev/spidev0.0
getfacl /dev/i2c-1 /dev/gpiochip0 /dev/spidev0.0
udevadm verify /etc/udev/rules.d/99-xwalk-picarx.rules
udevadm test /sys/class/gpio/gpiochip0
```

Adjust paths to the provisioned devices. A template containing unresolved substitutions is not an installed
udev rule and will intentionally fail validation.

## Optional systemd service

The package supplies `xwalk.service` and an environment file. Its default non-moving command is `doctor`;
select a reviewed foreground command in `/etc/xwalk/xwalk-service.conf` before enabling a persistent service.
The environment file contains only the reviewed service command. AI models and
credentials are never duplicated there. The package installs `xWalkEnv.sh`,
`xWalkLicenseTool`, and the empty model configuration under the matching
`/usr/lib/xwalk/xWalkTool` subdirectories. An explicitly provisioned package may
also install `X_WALK_LICENSE.KEY` under `/usr/lib/xwalk/xWalkLibrary`; normal
packages omit the deployment-specific ciphertext. See the
[licence-key workflow](License%20Key%20Workflow.md).

An operator may source the installed loader for an interactive process. The
loader requests the separate decryption key, rejects incomplete or unexpected
fields, never prints values, and removes temporary plaintext. The service does
not embed or automatically retrieve that key; unattended integration requires
a separately reviewed operating-system credential mechanism.
The unit uses the unprivileged `xwalk` user, explicit executable/configuration/working-directory paths,
bounded restart, a 10-second stop timeout, SIGTERM, and device-compatible hardening.

```sh
systemd-analyze verify /usr/lib/systemd/system/xwalk.service
systemd-analyze security xwalk.service
systemctl enable --now xwalk.service
journalctl -u xwalk.service -f
```

Manual execution uses the same explicit configuration contract:

```sh
/usr/bin/xwalk-picarx-control --deployment-config /var/lib/xwalk/picar-x.conf doctor
```

`SIGINT` and `SIGTERM` only request shutdown in the signal handler. The normal controlling thread performs
motor cleanup. Scope-bound non-throwing guards stop both motors on normal return, initialization failure after
activation, and an escaping application failure. Host fakes exercise those paths without starting motors.

## Optional voice/model setup

`--with-vosk` validates the architecture-selected runtime and model installed
from `xWalkLibrary`. `--with-ollama` adds validation and provisioning reminders,
not a remote installer pipeline. Deploy Ollama model data separately, then set
its absolute manifest path in the active configuration. `doctor` reports
availability without recording audio or contacting a model service.

## Uninstall

Confirm the installed package name and state:

```sh
dpkg -l | grep xwalk-picarx
```

Stop and disable the optional service before removing its executable and unit:

```sh
sudo systemctl disable --now xwalk.service
```

Remove the package while retaining administrator-controlled configuration:

```sh
sudo apt remove xwalk-picarx
```

Use purge only when `/etc/xwalk` package configuration should also be removed:

```sh
sudo apt purge xwalk-picarx
```

After reviewing the proposed dependency changes, remove packages that APT says
are no longer required:

```sh
sudo apt autoremove
```

`remove` keeps package configuration under `/etc/xwalk`; `purge` removes the
package-managed configuration. Runtime-created `/var/lib/xwalk` calibration and
`/var/cache/xwalk` data may remain because package removal must not silently
destroy operator data. Inspect and archive those directories before deciding
whether to delete them manually. Do not remove calibration data when it may be
needed by a later reinstall.

## Hardware acceptance

Physical acceptance remains mandatory: secure the chassis with wheels raised, verify the HAT and GPIO chip,
run `doctor`, calibrate left/right direction, balance and steering at the first-run cap, then send SIGINT and
SIGTERM during controlled motion and confirm both motors electrically stop. Also verify restart, power loss,
camera, audio, sensors, and the selected SPI peripheral. Until those target checks pass, call this repository
host-tested and deployment-ready in source, not physically verified or plug-and-run.
