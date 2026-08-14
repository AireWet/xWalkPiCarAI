#!/usr/bin/env bash

set -u

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../../.." && pwd)"
overall_status=0

run_check() {
    local name="$1"
    shift
    printf '\n== %s ==\n' "$name"
    "$@"
    local status=$?
    if [ "$status" -eq 1 ]; then
        overall_status=1
    elif [ "$status" -ne 0 ] && [ "$overall_status" -eq 0 ]; then
        overall_status="$status"
    fi
}

cd "$repository_root" || exit 1
run_check dependencies xWalkTool/shell-agent/quality-tool/check-host-quality-dependencies.sh
run_check asan-ubsan xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh asan
run_check leak-sanitizer xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh lsan
run_check thread-sanitizer xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh tsan
run_check gcc-coverage xWalkTool/shell-agent/quality-tool/run-host-coverage.sh run gcc
run_check valgrind xWalkTool/shell-agent/quality-tool/run-host-valgrind.sh
run_check clang-static-analyzer xWalkTool/shell-agent/quality-tool/run-clang-static-analyzer.sh
run_check shellcheck xWalkTool/shell-agent/quality-tool/run-host-shellcheck.sh
exit "$overall_status"
