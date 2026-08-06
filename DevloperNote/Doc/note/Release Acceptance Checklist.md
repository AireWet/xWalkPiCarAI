# Release acceptance checklist

This checklist separates evidence that can be produced on a development host from evidence that requires
the final Raspberry Pi, Robot HAT, peripherals, and vehicle. A release claim is valid only when every item
in its section has recorded evidence. Building successfully is not a substitute for physical verification.

## 1. Host release gate

- [ ] GCC Debug build and all host tests pass.
- [ ] GCC Release build and all host tests pass with assertions enabled.
- [ ] Clang Debug and Release builds and all host tests pass.
- [ ] Strict warnings build without project-owned warnings.
- [ ] Clang-Tidy completes without unsuppressed findings.
- [ ] The focused Cppcheck gate completes without unsuppressed findings.
- [ ] AddressSanitizer and UndefinedBehaviorSanitizer tests pass.
- [ ] LeakSanitizer passes in an environment that does not trace test processes.
- [ ] ThreadSanitizer passes in its separate, untraced host build.
- [ ] The bounded 20-cycle host stress preset completes without a failure.
- [ ] Coverage HTML and Cobertura reports are retained with line, function, and branch summaries.
- [ ] ShellCheck and provisioning host tests pass.
- [ ] A staged installation under `build-host/deploy` contains the documented system layout.
- [ ] The staged executable runs from an unrelated working directory.
- [ ] Installed configuration contains no development-workspace absolute path.
- [ ] Installed files are not group-writable or world-writable, and staged checksums are retained.

## 2. ARM package gate

Complete this section later on the supported ARM target or with a verified ARM toolchain.

- [ ] The package architecture is the target architecture, normally `arm64`.
- [ ] `dpkg-deb --contents` shows only the documented `/usr`, `/etc`, `/var`, and `/run` layout.
- [ ] `/etc/xwalk/picar-x.conf`, its `picar-x.d` fragments, and `xwalk-service.conf` are Debian conffiles.
- [ ] `lintian` has no unexplained error or warning.
- [ ] Installation and removal succeed on a clean disposable Raspberry Pi image.
- [ ] Upgrade preserves administrator configuration and mutable calibration state.

## 3. Raspberry Pi plug-and-run gate

- [ ] The supported Raspberry Pi model and operating system are recorded.
- [ ] The Robot HAT revision is physically identified; v4 is never inferred from failed v5 discovery.
- [ ] `setup-rpi.sh --check` and `doctor` pass without claiming actuator outputs.
- [ ] GPIO chip name, label, line count, and configured identity agree.
- [ ] I2C, SPI, GPIO, audio, video, and optional render permissions work for the `xwalk` user.
- [ ] The installed systemd and generated udev files validate on the target.
- [ ] Packaged sounds and music resolve without depending on the working directory.
- [ ] Camera, microphone, speaker, mixer, and required external executables are available.

## 4. Physical safety gate

Secure the car and raise the drive wheels before the first motor command.

- [ ] Left and right motor directions are checked independently at the first-run speed cap.
- [ ] Paired-motor balance and persistent calibration are confirmed.
- [ ] Steering center and camera-servo limits do not mechanically bind.
- [ ] Normal exit, `SIGINT`, `SIGTERM`, service stop, failure, and restart stop both motors.
- [ ] Grayscale and cliff references are calibrated on the intended surface and lighting.
- [ ] Ultrasonic behavior and timeout are checked against known distances.
- [ ] Battery voltage is compared with an appropriate external measurement.
- [ ] Audio and camera operations are checked separately before voice-controlled movement.
- [ ] Every hardware-labelled test is reviewed and then run individually when safe.

## 5. Sign-off record

Record the commit identifier, package checksum, Raspberry Pi model, operating system, Robot HAT revision,
test operator, test date, and links to CI, coverage, static-analysis, package, and hardware evidence. Do not
mark the firmware production-perfect, physically verified, plug-and-run, or ARM release-quality without this
record.
