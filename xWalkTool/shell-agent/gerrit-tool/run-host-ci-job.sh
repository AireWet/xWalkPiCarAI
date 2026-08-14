#!/usr/bin/env bash

set -Eeuo pipefail

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../../.." && pwd)"
product_root="$repository_root/xWalk-rpi5"
job="${1-}"
shift || true

usage() {
    printf 'Usage: %s JOB [ARGUMENTS]\n' "$0" >&2
    exit 2
}

require_no_arguments() {
    if [ "$#" -ne 0 ]; then
        usage
    fi
}

run_build_and_test() {
    if [ "$#" -ne 2 ]; then
        usage
    fi

    local compiler="$1"
    local build_type="$2"
    case "$compiler" in
        gcc)
            export CC=gcc
            export CXX=g++
            ;;
        clang)
            export CC=clang
            export CXX=clang++
            ;;
        *) usage ;;
    esac
    case "$build_type" in
        Debug|Release) ;;
        *) usage ;;
    esac

    local build_directory="build-host/ci-${compiler}-${build_type}"
    cmake --fresh -S "$product_root" -B "$build_directory" -G Ninja \
        -DCMAKE_BUILD_TYPE="$build_type" -DXWALK_ENABLE_STRICT_WARNINGS=ON
    cmake --build "$build_directory" --parallel
    ctest --test-dir "$build_directory" --output-on-failure --no-tests=error
    "$build_directory/xWalkController/xWalkApp/xwalk-picarx-control" \
        --deployment-config="$repository_root/xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf" \
        --diagnose --no-hardware
    ctest --test-dir "$build_directory" --output-on-failure --no-tests=error -L agent-aggregate
    ctest --test-dir "$build_directory" --output-on-failure --no-tests=error -L agent-group
    ctest --test-dir "$build_directory" --output-on-failure --no-tests=error -L simulation
    ctest --test-dir "$build_directory" --output-on-failure --no-tests=error -L recorded-media
    ctest --test-dir "$build_directory" --output-on-failure --no-tests=error -L streaming
    "$build_directory/xGoogleTest" TEST_SUITE_XWALK_SEQUENCE:1 --gtest_brief=1
    "$build_directory/xCliGoogleTest" --gtest_brief=1
    "$build_directory/xCliSequenceTest" --gtest_brief=1
}

run_fuzz_smoke() {
    cmake --fresh -S "$product_root" --preset fuzz
    cmake --build build-host/fuzz --target xWalkFuzzers --parallel
    local fuzz_name
    for fuzz_name in configuration protobuf grpc http_requests camera_sources model_metadata scenarios \
        i2c_payloads image_decode; do
        "build-host/fuzz/xWalkTool/cpp-tool/fuzz/xWalkFuzz_${fuzz_name}" -runs=1000 \
            -artifact_prefix=/tmp/xwalk-fuzz- "xWalkTool/cpp-tool/fuzz/corpus/${fuzz_name}"
    done
}

run_staged_install() {
    cmake --fresh -S "$product_root" -B build-host/install -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build-host/install --parallel
    DESTDIR="$repository_root/build-host/deploy" cmake --install build-host/install
    test -x build-host/deploy/usr/bin/xwalk-picarx-control
    test -x build-host/deploy/usr/lib/xwalk/xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
    test -x build-host/deploy/usr/lib/xwalk/xWalkTool/py-agent/dev-tool/xWalkLicenseTool
    test -r build-host/deploy/usr/lib/xwalk/xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg
    test ! -e build-host/deploy/usr/lib/xwalk/xWalkLibrary/X_WALK_LICENSE.KEY
    test -r build-host/deploy/etc/xwalk/picar-x.conf
    test -r build-host/deploy/etc/xwalk/picar-x.d/hardware.conf
    test -r build-host/deploy/etc/xwalk/picar-x.d/ai/providers/gemini.conf
    test -r build-host/deploy/usr/lib/xwalk/libvosk.so
    test -r build-host/deploy/usr/share/xwalk/models/vosk/vosk-model-small-en-us-0.15/am/final.mdl
    test -r build-host/deploy/usr/share/xwalk/sounds/car-double-horn.wav
    test -r build-host/deploy/usr/share/xwalk/sounds/car-start-engine.wav
    test -r build-host/deploy/usr/share/xwalk/music/slow-trail-Ahjay_Stelino.mp3
    (
        cd /tmp
        "$repository_root/build-host/deploy/usr/bin/xwalk-picarx-control" --help
        "$repository_root/build-host/deploy/usr/bin/xwalk-picarx-control" \
            --deployment-config="$repository_root/build-host/deploy/etc/xwalk/picar-x.conf" \
            --diagnose --no-hardware
    )
    if grep -R --fixed-strings "$repository_root" \
        build-host/deploy/etc build-host/deploy/usr/share/xwalk/config; then
        exit 1
    fi
    ldd build-host/deploy/usr/bin/xwalk-picarx-control
    test -z "$(find build-host/deploy -type f -perm /022 -print -quit)"
    find build-host/deploy -type f -print0 | sort -z | xargs -0 sha256sum >build-host/deploy.sha256
}

validate_ci_metadata() {
    local ansible_home="$repository_root/build-host/ansible"
    python3 xWalkTool/py-agent/dev-tool/xWalkZuulValidator .zuul.yaml
    ANSIBLE_HOME="$ansible_home" ANSIBLE_LOCAL_TEMP="$ansible_home/local" \
        ansible-playbook --syntax-check \
        xWalkTool/shell-agent/env-tool/playbooks/zuul/run-host-quality-job.yaml
    ANSIBLE_HOME="$ansible_home" ANSIBLE_LOCAL_TEMP="$ansible_home/local" \
        ansible-playbook --syntax-check \
        xWalkTool/shell-agent/env-tool/playbooks/zuul/collect-host-quality-artifacts.yaml
}

cd "$repository_root"
case "$job" in
    build-and-test) run_build_and_test "$@" ;;
    asan-ubsan) require_no_arguments "$@"; xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh asan ;;
    leak-sanitizer) require_no_arguments "$@"; xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh lsan ;;
    thread-sanitizer) require_no_arguments "$@"; xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh tsan ;;
    stress-tests)
        require_no_arguments "$@"
        cmake --fresh -S "$product_root" --preset sanity
        cmake --build build-host/sanity --parallel
        ctest --test-dir build-host/sanity --output-on-failure --no-tests=error \
            --repeat until-fail:20
        ;;
    recorded-scenarios)
        require_no_arguments "$@"
        cmake --fresh -S "$product_root" --preset host-debug
        cmake --build build-host/cmake \
            --target xWalkComputerVisionOpenCvHostTest xWalkVideoRecordingOpenCvHostTest \
            xWalkRoadUserSafetyTest --parallel
        ctest --test-dir build-host/cmake --output-on-failure --no-tests=error -L recorded-media
        ;;
    streaming)
        require_no_arguments "$@"
        cmake --fresh -S "$product_root" --preset host-debug
        cmake --build build-host/cmake --target xWalkVideoStreamingTest --parallel
        ctest --test-dir build-host/cmake --output-on-failure --no-tests=error -L streaming
        ;;
    fuzz-smoke) require_no_arguments "$@"; run_fuzz_smoke ;;
    soak-smoke)
        require_no_arguments "$@"
        cmake --fresh -S "$product_root" --preset host-debug
        cmake --build build-host/cmake --target xWalkRobotHatSoakTest --parallel
        ctest --test-dir build-host/cmake --output-on-failure --no-tests=error -L soak
        python3 -m json.tool \
            build-host/cmake/xWalkHal/simulation/xWalkRobotHat/xwalk-robot-hat-soak-report.json
        ;;
    static-analysis)
        require_no_arguments "$@"
        cmake --fresh -S "$product_root" --preset clang-tidy
        cmake --build build-host/clang-tidy --parallel
        cmake --build build-host/clang-tidy --target cppcheck
        ;;
    coverage)
        require_no_arguments "$@"
        xWalkTool/shell-agent/quality-tool/run-host-coverage.sh run gcc
        xWalkTool/shell-agent/quality-tool/run-host-coverage.sh run clang
        ;;
    valgrind)
        require_no_arguments "$@"
        xWalkTool/shell-agent/quality-tool/test/validate-valgrind-descriptors-test.sh
        xWalkTool/shell-agent/quality-tool/run-host-valgrind.sh
        ;;
    clang-static-analyzer)
        require_no_arguments "$@"
        xWalkTool/shell-agent/quality-tool/run-clang-static-analyzer.sh
        ;;
    shellcheck)
        require_no_arguments "$@"
        xWalkTool/shell-agent/quality-tool/run-host-shellcheck.sh
        validate_ci_metadata
        ;;
    deployment-scripts)
        require_no_arguments "$@"
        xWalkTool/shell-agent/quality-tool/run-host-shellcheck.sh
        bash xWalkTool/shell-agent/deploy-tool/test/setup-rpi-test.sh
        ;;
    staged-install) require_no_arguments "$@"; run_staged_install ;;
    host-quality-gate)
        require_no_arguments "$@"
        test -r .github/workflows/host-quality.yml
        test -r .zuul.yaml
        test -r xWalkTool/shell-agent/env-tool/playbooks/zuul/run-host-quality-job.yaml
        printf 'xWalk Host Quality dependency graph completed successfully.\n'
        ;;
    *) usage ;;
esac
