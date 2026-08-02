# Eclipse Build Script Guide

[`xWalkTool/eclipse-build.sh`](../../../xWalkTool/eclipse-build.sh) configures and builds the host CLI tree used by
the repository's Eclipse workflow. It does not configure Raspberry Pi backends or access physical hardware.

## Fixed configuration

The script always uses:

| Setting | Value |
| --- | --- |
| Source tree | `xWalkCLI` |
| Build tree | `xWalkCLI/build-eclipse-host` |
| Host option | `XWALK_CLI_BUILD_HOST=ON` |
| Build type | `Debug` |
| Compilation database | Enabled |

The source and build paths are resolved from the script location. Running it from another working directory
does not redirect its output.

## Configure and build

```sh
xWalkTool/eclipse-build.sh
```

This command configures the existing or new build tree and then runs a parallel build. The generated
`compile_commands.json` can be used by Eclipse, Clang-Tidy, and compatible editor tooling.

## Clean the configured build

```sh
xWalkTool/eclipse-build.sh clean
```

The script still runs CMake configuration first, then invokes the configured `clean` target. It retains the
build directory and CMake cache. Use the clean-build tool when the entire generated directory must be removed.

```sh
xWalkTool/clean-build.sh --dry-run
```

## Command limitations

The script currently has no `--help` option and parses only the first argument. The only supported forms are
the no-argument build and the `clean` action. Any other first argument currently falls through to a normal
build, so do not rely on unknown-argument rejection.

Configuration or compilation failures stop the script with the failing CMake status. Existing output in
`xWalkCLI/build-eclipse-host` is reused unless it is cleaned or removed explicitly.

## Verification

Syntax validation does not configure or build:

```sh
bash -n xWalkTool/eclipse-build.sh
```

Running the script itself writes generated host output and performs compilation. It does not run CTest.
