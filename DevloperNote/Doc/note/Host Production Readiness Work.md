# Host production readiness work

This page tracks work that can improve release confidence without a Raspberry Pi, Robot HAT, motors, or
other physical peripherals. Host evidence cannot establish electrical motor shutdown, correct board identity,
device permissions on the target image, Raspberry Pi plug-and-run behavior, or ARM package quality.

## 1. Implemented host gates

- Strict GCC/Clang warnings apply only to project-owned targets.
- Debug and Release tests retain active assertions.
- AddressSanitizer and UndefinedBehaviorSanitizer use an isolated build.
- ThreadSanitizer uses a separate build and is never combined with AddressSanitizer.
- Clang-Tidy and focused Cppcheck checks use the compilation database.
- Coverage instrumentation has an isolated GCC/gcov build.
- Shell syntax, provisioning behavior, and staged installation have host-safe checks.
- CI repeats all host tests 20 times and stops at the first failure.
- CI rejects group-writable or world-writable installed files and records staged-file checksums.

## 2. Local verification commands

Run the ordinary host gates:

```sh
cmake --fresh --preset sanity
cmake --build --preset sanity --parallel
ctest --preset sanity
cmake --fresh --preset host-release
cmake --build --preset host-release --parallel
ctest --preset host-release
cmake --fresh --preset clang-tidy
cmake --build --preset clang-tidy --parallel
cmake --build build-host/clang-tidy --target cppcheck
cmake --fresh --preset sanitizers
cmake --build --preset sanitizers --parallel
ctest --preset sanitizers
```

Run concurrency and repeated-test gates separately:

```sh
cmake --fresh --preset thread-sanitizer
cmake --build --preset thread-sanitizer --parallel
ctest --preset thread-sanitizer
ctest --preset host-stress
```

ThreadSanitizer may be incompatible with a traced or restricted execution environment. A runtime failure
caused by the environment is not a pass; repeat the same test on an ordinary CI runner.

## 3. Coverage gate

Install `gcovr` explicitly through the operating system or an isolated host-tools virtual environment:

```sh
sudo apt-get install gcovr
python3 -m venv build-host/tools/gcovr-venv
build-host/tools/gcovr-venv/bin/pip install gcovr
```

Only one installation method is required. The repository never installs this tool automatically. Generate
terminal, detailed HTML, and Cobertura XML reports with:

```sh
xWalkTool/run-host-coverage.sh run
```

The foreground-only runner uses a system `gcovr` executable or
`build-host/tools/gcovr-venv/bin/gcovr`. It fails before configuration when neither is available and does not
create a detached process.

Review line, function, and branch coverage separately. Add tests for meaningful production behavior,
especially motor shutdown, configuration validation, resource validation, initialization rollback, worker
cancellation, and malformed backend responses. Do not enforce thresholds until the measured baseline is
reviewed, and do not improve percentages by excluding important production files or testing trivial getters.

## 4. Clean-environment checks

The GitHub host workflow supplies clean Ubuntu runners for compiler, sanitizer, analysis, coverage, script,
stress, and staged-install checks. A successful workflow run is required release evidence; adding the workflow
file alone is not evidence that it passed.

The staged-install job must confirm:

- the CLI and required configuration, sounds, music, and documentation are installed;
- the CLI runs from an unrelated working directory;
- installed configuration does not contain the checkout path;
- linked shared libraries are inspectable;
- installed files are not group-writable or world-writable;
- a checksum manifest is retained as a workflow artifact.

## 5. Host work requiring additional tools or external execution

- Run the GitHub workflow and preserve its result for the release record.
- Run LeakSanitizer on an ordinary untraced runner when local tracing blocks it.
- Use `gcovr` for exact line, function, and branch percentages.
- Use ShellCheck for every project-owned shell script.
- Validate systemd and generated udev files inside a complete compatible operating-system root.
- Optionally inspect an explicitly labelled host Debian package, without treating it as the ARM release.

These items remain incomplete until their commands actually pass. Missing local tools must be reported rather
than silently skipped.

## 6. Local evidence from 2026-08-02

- The isolated ThreadSanitizer configuration and all 264 build steps completed successfully.
- ThreadSanitizer execution is blocked in this traced sandbox by `unexpected memory mapping`; it did not pass.
- The strict host suite completed 20 repetitions without failure: 58 tests per cycle and 1,160 executions.
- The stress run completed in 83.16 seconds without physical device access.
- `gcovr` and ShellCheck are unavailable locally, so their CI jobs remain the required execution evidence.
- No Raspberry Pi, ARM package, system service, udev installation, or physical actuator test was performed.

## 7. Raspberry Pi boundary

The following cannot be completed from host fakes:

- native ARM composition and package dependency verification;
- clean Raspberry Pi installation and reboot testing;
- real GPIO, I2C, SPI, audio, camera, and permission checks;
- Robot HAT revision and GPIO chip identity confirmation;
- motor direction, balance, steering, first-run cap, and electrical stop verification;
- low-battery, sensor, service-stop, restart, and backend-fault physical acceptance.

Until those gates pass, describe the repository as host-tested and prepared for controlled Raspberry Pi
integration, not production-perfect, physically verified, plug-and-run, or ARM release-qualified.
