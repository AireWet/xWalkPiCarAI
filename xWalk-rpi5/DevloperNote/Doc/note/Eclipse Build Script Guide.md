# Eclipse Build Script Guide

[`xWalkTool/shell/eclipse-build.sh`](../../../../xWalkTool/shell/eclipse-build.sh)
configures, builds, and tests the host CLI tree used by the repository's Eclipse
workflow. It does not configure Raspberry Pi backends or access physical hardware.

## Fixed configuration

The script always uses:

| Setting | Value |
| --- | --- |
| Source tree | `xWalkController` |
| Build tree | `xWalk-rpi5/xWalkController/build-eclipse-host` |
| Host option | `XWALK_CLI_BUILD_HOST=ON` |
| Build type | `Debug` |
| Compilation database | Enabled |
| Host CTest inventory | Runs after a successful build |

The source and build paths are resolved from the script location. Running it from another working directory
does not redirect its output.

## Eclipse project metadata

Import the repository root as the Eclipse CDT project. The checked-in metadata
provides the following workspace behavior:

| File | Responsibility |
| --- | --- |
| `.project` | Project identity and the external host-build command |
| `.cproject` | C++17 language, source exclusions, and workspace include paths |
| `.settings/org.eclipse.cdt.core.prefs` | Fast Indexer and complete source/header indexing |
| `.settings/org.eclipse.core.resources.prefs` | UTF-8 project text encoding |

The CDT indexer covers source files outside the active build and unused C++
headers. Generated `build*` and `CMakeFiles` directories remain excluded by
`.cproject`, preventing generated build trees from being indexed as project
source. CMake compilation and CTest results remain authoritative when an
editor diagnostic differs from the configured build.

## Configure and build

```sh
xWalkTool/shell/eclipse-build.sh
```

This command configures the existing or new build tree, runs a parallel build, and executes every registered
host test. This includes the root `xWalkAgentGoogleTest`, all seven Agent group suites, centralized CLI tests,
and the module tests pulled into the CLI host composition. The generated `compile_commands.json` can be used
by Eclipse, Clang-Tidy, and compatible editor tooling.

## Clean the configured build

```sh
xWalkTool/shell/eclipse-build.sh clean
```

The script removes the complete generated Eclipse build directory. This also removes stale compiler dependency
metadata after source or header renames. Use the clean-build tool to preview or remove all recognized workspace
build output instead.

```sh
xWalkTool/shell/clean-build.sh --dry-run
```

## Command limitations

The script currently has no `--help` option and parses only the first argument. The only supported forms are
the no-argument build and the `clean` action. Any other first argument currently falls through to a normal
build, so do not rely on unknown-argument rejection.

Configuration, compilation, or test failures stop the script with the failing status. Existing output in
`xWalk-rpi5/xWalkController/build-eclipse-host` is reused until the `clean` action removes it explicitly.

## Verification

Syntax validation does not configure or build:

```sh
bash -n xWalkTool/shell/eclipse-build.sh
```

Running the script itself writes generated host output, performs compilation, and runs CTest.
