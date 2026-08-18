#!/usr/bin/env bash

set -eu

usage() {
    echo "Usage: $0 [--build-directory DIRECTORY] [--ollama-manifest FILE] [--generate-only]"
}

script_directory="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
workspace_root="$(CDPATH='' cd -- "$script_directory/../../.." && pwd)"
build_directory="$workspace_root/build-rpi"
ollama_manifest=""
generate_only="false"

while [ "$#" -gt 0 ]; do
    case "$1" in
        --build-directory) build_directory="${2-}"; shift 2 ;;
        --ollama-manifest) ollama_manifest="${2-}"; shift 2 ;;
        --generate-only) generate_only="true"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; exit 2 ;;
    esac
done

runtime_user="$(id -un)"
runtime_home="$(getent passwd "$runtime_user" | awk -F: 'NR == 1 { print $6 }')"
if [ -z "$runtime_home" ] || [ ! -d "$runtime_home" ]; then
    echo "Unable to resolve the runtime home for $runtime_user." >&2
    exit 2
fi
if [ -n "${HOME-}" ] && [ "$HOME" != "$runtime_home" ]; then
    echo "HOME does not match the account database for $runtime_user." >&2
    exit 2
fi

models_directory="$runtime_home/.local/share/ollama/models"
if [ -z "$ollama_manifest" ] && [ -d "$models_directory/manifests" ]; then
    ollama_manifest="$(find "$models_directory/manifests" -type f \
        -path '*/registry.ollama.ai/library/llama3.2/3b' -print -quit)"
fi
if [ -z "$ollama_manifest" ]; then
    ollama_manifest="$models_directory/manifests/registry.ollama.ai/library/llama3.2/3b"
fi
case "$ollama_manifest" in
    /*) ;;
    *) echo "--ollama-manifest must be an absolute path." >&2; exit 2 ;;
esac

if [ "$generate_only" != "true" ] && [ ! -r "$ollama_manifest" ]; then
    echo "The llama3.2:3b manifest is not readable: $ollama_manifest" >&2
    echo "Run setup-rpi-local.sh --apply before configuring the runtime." >&2
    exit 1
fi

"$script_directory/generate-rpi-runtime.sh" \
    --build-directory "$build_directory" \
    --ollama-manifest "$ollama_manifest"

if [ "$generate_only" = "true" ]; then
    exit 0
fi

ollama_executable="$runtime_home/.local/bin/ollama"
if [ ! -x "$ollama_executable" ]; then
    echo "The user-local Ollama executable is missing: $ollama_executable" >&2
    exit 1
fi

systemctl --user daemon-reload
systemctl --user enable --now ollama.service
systemctl --user --no-pager status ollama.service
"$ollama_executable" list

echo "Runtime configuration and the Ollama user service are ready."
echo "Run from xWalk-rpi5: ../build-rpi/xwalk --validate-config"
echo "Doctor performs the documented MCU-reset-only bounded preflight: ../build-rpi/xwalk doctor"
