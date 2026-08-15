#!/usr/bin/env bash

set -u

case "${1-}" in
    "") repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../../.." && pwd)" ;;
    --tool-root) repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)" ;;
    *) printf 'Usage: %s [--tool-root]\n' "$0" >&2; exit 2 ;;
esac
if ! command -v shellcheck >/dev/null 2>&1; then
    echo "SHELLCHECK: SKIPPED_MISSING_TOOL - shellcheck was not found"
    exit 2
fi

cd "$repository_root" || exit 1
find . \
    -path './.git' -prune -o \
    -path './build-host' -prune -o \
    -path './build-rpi' -prune -o \
    -path './build-aarch64' -prune -o \
    -path './xWalk-rpi5/xWalkLibrary/x86_64' -prune -o \
    -path './xWalk-rpi5/xWalkLibrary/aarch64' -prune -o \
    -path '*/third_party/*' -prune -o \
    -path '*/auto-gen/*' -prune -o \
    -type f -name '*.sh' -print0 |
    xargs -0 -r shellcheck -x -P SCRIPTDIR
status=$?
if [ "$status" -ne 0 ]; then
    echo "SHELLCHECK: FAILED"
    exit "$status"
fi
echo "SHELLCHECK: PASSED"
