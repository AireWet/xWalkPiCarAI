#!/usr/bin/env bash

set -eu

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)"

usage() {
    echo "Usage: $0 run"
    echo "  run      Build, test, and generate coverage in the foreground."
}

gcovr_command() {
    if command -v gcovr >/dev/null 2>&1; then
        command -v gcovr
        return 0
    fi
    local virtual_environment_gcovr="$repository_root/build-host/tools/gcovr-venv/bin/gcovr"
    if [ -x "$virtual_environment_gcovr" ]; then
        printf '%s\n' "$virtual_environment_gcovr"
        return 0
    fi
    echo "gcovr is required but was not found." >&2
    echo "Install it with the OS package manager, or create the documented host virtual environment." >&2
    return 2
}

run_coverage() {
    local gcovr_executable
    gcovr_executable="$(gcovr_command)" || return $?
    cd "$repository_root" || return 2
    cmake --fresh --preset coverage || return $?
    cmake --build --preset coverage --parallel || return $?
    ctest --preset coverage || return $?
    "$gcovr_executable" --config xWalkTool/environment/gcovr.cfg
}

case "${1-}" in
    run) run_coverage ;;
    -h|--help|help) usage ;;
    *) usage; exit 2 ;;
esac
