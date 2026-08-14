#!/usr/bin/env bash

set -eu

SCRIPT_DIRECTORY=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
XWALK_RPI5_ROOT="${SCRIPT_DIRECTORY}"
REPOSITORY_ROOT=$(CDPATH='' cd -- "${XWALK_RPI5_ROOT}/.." && pwd)
BUILD_DIRECTORY="${XWALK_RPI5_ROOT}/xWalkController/build-eclipse-host"

if [ "${1:-}" = "clean" ]
then
    cmake -E remove_directory "${BUILD_DIRECTORY}"
else
    cmake --fresh -S "${REPOSITORY_ROOT}" -B "${BUILD_DIRECTORY}" \
        -DXWALK_CLI_BUILD_HOST=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    cmake --build "${BUILD_DIRECTORY}" --parallel
    ctest --test-dir "${BUILD_DIRECTORY}" --output-on-failure --no-tests=error
fi
