#!/usr/bin/env bash

set -u

SCRIPT_DIRECTORY=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
REPOSITORY_ROOT=$(CDPATH='' cd -- "${SCRIPT_DIRECTORY}/.." && pwd)
DRY_RUN=false
ASSUME_YES=false

show_usage()
{
    printf '%s\n' \
        "Usage: xWalkTool/clean-build.sh [--dry-run] [--yes]" \
        "" \
        "  --dry-run  List generated build directories and files without deleting them." \
        "  --yes      Delete without an interactive confirmation." \
        "  --help     Show this help text."
}

for argument in "$@"
do
    case "${argument}" in
        --dry-run)
            DRY_RUN=true
            ;;
        --yes)
            ASSUME_YES=true
            ;;
        --help|-h)
            show_usage
            exit 0
            ;;
        *)
            printf 'ERROR: Unknown argument: %s\n' "${argument}" >&2
            show_usage >&2
            exit 2
            ;;
    esac
done

if [ ! -f "${REPOSITORY_ROOT}/xWalkHal/CMakeLists.txt" ] ||
    [ ! -f "${REPOSITORY_ROOT}/AGENTS.md" ]
then
    printf 'ERROR: Unable to verify the MyPiCarX workspace root: %s\n' \
        "${REPOSITORY_ROOT}" >&2
    exit 1
fi

if ! command -v cmake >/dev/null 2>&1
then
    printf 'ERROR: cmake is required to remove generated build output safely.\n' >&2
    exit 1
fi

mapfile -d '' -t BUILD_DIRECTORIES < <(
    find "${REPOSITORY_ROOT}" -mindepth 1 -type d \
        \( -name build -o -name 'build-*' \) \
        -prune -print0 | sort -z
)

mapfile -d '' -t CACHE_FILES < <(
    find "${REPOSITORY_ROOT}" -mindepth 1 -type f -name CMakeCache.txt \
        -print0 | sort -z
)

declare -a IN_SOURCE_BUILD_ROOTS=()

for cache_file in "${CACHE_FILES[@]}"
do
    cache_directory=${cache_file%/*}
    relative_cache_directory=${cache_directory#"${REPOSITORY_ROOT}"/}
    is_named_build_directory=false

    IFS='/' read -r -a path_components <<< "${relative_cache_directory}"
    for path_component in "${path_components[@]}"
    do
        case "${path_component}" in
            build|build-*)
                is_named_build_directory=true
                break
                ;;
        esac
    done

    if [ "${is_named_build_directory}" = false ]
    then
        IN_SOURCE_BUILD_ROOTS+=("${cache_directory}")
    fi
done

printf 'MyPiCarX build cleanup targets:\n'

for build_directory in "${BUILD_DIRECTORIES[@]}"
do
    printf '  directory: %s\n' "${build_directory#"${REPOSITORY_ROOT}"/}"
done

for build_root in "${IN_SOURCE_BUILD_ROOTS[@]}"
do
    printf '  in-source CMake output: %s\n' \
        "${build_root#"${REPOSITORY_ROOT}"/}"
done

if [ "${#BUILD_DIRECTORIES[@]}" -eq 0 ] &&
    [ "${#IN_SOURCE_BUILD_ROOTS[@]}" -eq 0 ]
then
    printf 'No generated CMake build output was found.\n'
    exit 0
fi

if [ "${DRY_RUN}" = true ]
then
    printf 'Dry run complete. No files were removed.\n'
    exit 0
fi

if [ "${ASSUME_YES}" = false ]
then
    if [ ! -t 0 ]
    then
        printf 'ERROR: Interactive confirmation is unavailable; use --yes to clean.\n' >&2
        exit 2
    fi

    printf 'Remove all listed generated build output? [y/N] '
    read -r confirmation
    case "${confirmation}" in
        y|Y|yes|YES)
            ;;
        *)
            printf 'Cleanup cancelled.\n'
            exit 0
            ;;
    esac
fi

for build_directory in "${BUILD_DIRECTORIES[@]}"
do
    if [ -f "${build_directory}/CMakeCache.txt" ]
    then
        cmake --build "${build_directory}" --target clean >/dev/null 2>&1 || true
    fi
    cmake -E remove_directory "${build_directory}"
done

for build_root in "${IN_SOURCE_BUILD_ROOTS[@]}"
do
    cmake --build "${build_root}" --target clean >/dev/null 2>&1 || true

    for generated_directory_name in CMakeFiles Testing _deps
    do
        generated_directory="${build_root}/${generated_directory_name}"
        if [ -d "${generated_directory}" ]
        then
            cmake -E remove_directory "${generated_directory}"
        fi
    done

    for generated_file_name in \
        .ninja_deps \
        .ninja_log \
        build.ninja \
        cmake_install.cmake \
        CMakeCache.txt \
        CTestTestfile.cmake \
        compile_commands.json \
        install_manifest.txt \
        Makefile \
        rules.ninja
    do
        generated_file="${build_root}/${generated_file_name}"
        if [ -f "${generated_file}" ]
        then
            cmake -E remove "${generated_file}"
        fi
    done
done

printf 'Build cleanup completed. Generated output cannot be recovered, but it can be rebuilt.\n'
