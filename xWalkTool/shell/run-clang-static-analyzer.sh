#!/usr/bin/env bash

set -u

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)"
report_directory="$repository_root/build-host/clang-analyzer/reports"

for tool_name in clang clang++ scan-build; do
    if ! command -v "$tool_name" >/dev/null 2>&1; then
        echo "CLANG_STATIC_ANALYZER: SKIPPED_MISSING_TOOL - $tool_name was not found"
        exit 2
    fi
done

cd "$repository_root" || exit 1
cmake --fresh --preset clang-analyzer || exit 1
cmake --build build-host/clang-analyzer --target clean || exit 1
mkdir -p "$report_directory"
scan-build --status-bugs --keep-empty -plist-html -o "$report_directory" \
    cmake --build --preset clang-analyzer --parallel
status=$?
if [ "$status" -ne 0 ]; then
    echo "CLANG_STATIC_ANALYZER: FAILED - reports: $report_directory"
    exit "$status"
fi
echo "CLANG_STATIC_ANALYZER: PASSED - reports: $report_directory"
