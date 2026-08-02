#!/usr/bin/env bash

set -eu

SCRIPT_DIRECTORY=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
REPOSITORY_ROOT=$(CDPATH='' cd -- "${SCRIPT_DIRECTORY}/.." && pwd)
BUILD_DIRECTORY="${REPOSITORY_ROOT}/xWalkCLI/build-eclipse-host"

cmake -S "${REPOSITORY_ROOT}/xWalkCLI" -B "${BUILD_DIRECTORY}" \
    -DXWALK_CLI_BUILD_HOST=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ "${1:-}" = "clean" ]
then
    cmake --build "${BUILD_DIRECTORY}" --target clean
else
    cmake --build "${BUILD_DIRECTORY}" --parallel
fi
