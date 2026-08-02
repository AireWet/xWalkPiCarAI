# Clean Build Script Guide

[`xWalkTool/shell/clean-build.sh`](../../../xWalkTool/shell/clean-build.sh)
discovers generated CMake and Python output throughout the MyPiCarX workspace
and optionally removes it. Use it when a clean configure is needed, stale CMake
caches must be discarded, or Python caches must be regenerated.

## Requirements

- Run the checked-in script from this workspace.
- Keep the root `AGENTS.md` and `xWalkHal/CMakeLists.txt` present so the script can verify the workspace.
- Provide `cmake` on `PATH`; deletion uses `cmake -E`.

The script resolves the repository root from its own location, so the current working directory does not
select the deletion scope.

## Options

| Option | Behavior |
| --- | --- |
| `--dry-run` | Lists every detected target and exits without deleting anything |
| `--yes` | Deletes every listed target without interactive confirmation |
| `--help`, `-h` | Prints usage and exits |
| No option | Lists targets and asks for interactive confirmation |

Unknown arguments exit with status 2. When standard input is not a terminal, deletion without `--yes` is
rejected rather than assumed.

## Preview cleanup

Always inspect the target list first:

```sh
xWalkTool/shell/clean-build.sh --dry-run
```

The preview prints directories named `build` or `build-*` anywhere below the
verified repository root. It also identifies in-source CMake output by locating
`CMakeCache.txt` outside those named build directories.

Python cleanup includes `__pycache__`, common Python tool caches, virtual test
environments such as `.tox` and `.nox`, `*.egg-info`, `*.pyc`, `*.pyo`, and
Python coverage data. A `dist` directory is included only when its parent is
identified as a Python package by `pyproject.toml`, `setup.py`, or `setup.cfg`.

## Perform cleanup

Interactive cleanup:

```sh
xWalkTool/shell/clean-build.sh
```

Non-interactive cleanup after reviewing the dry run:

```sh
xWalkTool/shell/clean-build.sh --yes
```

The script first asks CMake to clean configured build directories when possible.
It then deletes named build directories and detected Python-generated output.
For detected in-source builds, it removes known CMake directories and generated
files rather than deleting the source root.

## Recovery and safety

Deleted build and Python-generated output is not recoverable through this script,
although it can be regenerated from source. The script does not delete arbitrary
user-selected paths and does not accept a path argument.

Before `--yes`, confirm that no user-owned files were placed inside a generated build directory. Do not stop
the script partway through deletion; an interrupted cleanup may leave a partially cleaned build tree.

## Verification

```sh
bash -n xWalkTool/shell/clean-build.sh
xWalkTool/shell/clean-build.sh --help
xWalkTool/shell/clean-build.sh --dry-run
```

The verification commands above do not remove files.
