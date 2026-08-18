#!/usr/bin/env bash

set -eu

usage() {
    echo "Usage: $0 [--build-directory DIRECTORY] [--ollama-manifest FILE]"
}

script_directory="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
workspace_root="$(CDPATH='' cd -- "$script_directory/../../.." && pwd)"
build_directory="$workspace_root/build-rpi"
ollama_manifest=""

while [ "$#" -gt 0 ]; do
    case "$1" in
        --build-directory) build_directory="${2-}"; shift 2 ;;
        --ollama-manifest) ollama_manifest="${2-}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) usage >&2; exit 2 ;;
    esac
done

case "$build_directory" in
    /*) ;;
    *) build_directory="$(CDPATH='' cd -- "$(dirname -- "$build_directory")" && pwd)/$(basename -- "$build_directory")" ;;
esac

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

if [ -z "$ollama_manifest" ]; then
    ollama_manifest="$runtime_home/.local/share/ollama/models/manifests/registry.ollama.ai/library/llama3.2/3b"
fi
case "$ollama_manifest" in
    /*) ;;
    *) echo "--ollama-manifest must be an absolute path." >&2; exit 2 ;;
esac

source_configuration="$workspace_root/xWalk-rpi5/xWalkController/xWalkConfig"
runtime_directory="$build_directory/runtime"
mkdir -p "$build_directory"
temporary_runtime="$(mktemp -d "$build_directory/.runtime.XXXXXX")"
cleanup() {
    rm -rf -- "$temporary_runtime"
}
trap cleanup EXIT HUP INT TERM
cp -a "$source_configuration/." "$temporary_runtime/"

set_configuration_value() {
    local file_path="$1"
    local key="$2"
    local value="$3"
    local temporary_file
    temporary_file="$(mktemp "${file_path}.XXXXXX")"
    awk -v requested_key="$key" -v replacement="$value" '
        BEGIN { replaced = 0 }
        {
            separator = index($0, "=")
            name = separator == 0 ? "" : substr($0, 1, separator - 1)
            gsub(/^[[:space:]]+|[[:space:]]+$/, "", name)
            if (name == requested_key) {
                print requested_key " = " replacement
                replaced = 1
            } else {
                print
            }
        }
        END {
            if (replaced == 0) {
                print requested_key " = " replacement
            }
        }
    ' "$file_path" > "$temporary_file"
    mv -- "$temporary_file" "$file_path"
}

set_configuration_value "$temporary_runtime/picar-x.d/vision.conf" camera_connection csi
set_configuration_value "$temporary_runtime/picar-x.d/vision.conf" camera_csi_executable \
    "$runtime_home/.local/bin/rpicam-still"
set_configuration_value "$temporary_runtime/picar-x.d/vision.conf" camera_csi_device /dev/media0

set_configuration_value "$temporary_runtime/picar-x.d/hardware.conf" hardware_board robot_hat_v4
set_configuration_value "$temporary_runtime/picar-x.d/hardware.conf" hardware_i2c_device /dev/i2c-1
set_configuration_value "$temporary_runtime/picar-x.d/hardware.conf" hardware_gpio_device /dev/gpiochip4
set_configuration_value "$temporary_runtime/picar-x.d/hardware.conf" hardware_gpio_chip_name gpiochip4
set_configuration_value "$temporary_runtime/picar-x.d/hardware.conf" hardware_gpio_chip_label pinctrl-rp1
set_configuration_value "$temporary_runtime/picar-x.d/hardware.conf" hardware_spi_device /dev/spidev0.0

set_configuration_value "$temporary_runtime/picar-x.d/voice.conf" voice_vosk_library \
    "$workspace_root/xWalk-rpi5/xWalkLibrary/aarch64/lib/libvosk.so"
set_configuration_value "$temporary_runtime/picar-x.d/voice.conf" voice_vosk_model \
    "$workspace_root/xWalk-rpi5/xWalkLibrary/common/models/vosk/vosk-model-small-en-us-0.15"

ollama_configuration="$temporary_runtime/picar-x.d/ai/providers/ollama.conf"
set_configuration_value "$ollama_configuration" voice_language_model_provider ollama
set_configuration_value "$ollama_configuration" voice_language_model_endpoint \
    http://127.0.0.1:11434/api/chat
set_configuration_value "$ollama_configuration" voice_language_model_model_environment ""
set_configuration_value "$ollama_configuration" voice_ollama_model llama3.2:3b
set_configuration_value "$ollama_configuration" voice_ollama_model_manifest "$ollama_manifest"

rm -rf -- "$runtime_directory"
mv -- "$temporary_runtime" "$runtime_directory"
trap - EXIT HUP INT TERM

launcher="$build_directory/xwalk"
temporary_launcher="$(mktemp "$build_directory/.xwalk.XXXXXX")"
cat > "$temporary_launcher" <<'EOF'
#!/usr/bin/env bash

set -eu

launcher_directory="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
workspace_root="$(CDPATH='' cd -- "$launcher_directory/.." && pwd)"
runtime_user="$(id -un)"
runtime_home="$(getent passwd "$runtime_user" | awk -F: 'NR == 1 { print $6 }')"
if [ -z "$runtime_home" ] || [ ! -d "$runtime_home" ]; then
    echo "Unable to resolve the runtime home for $runtime_user." >&2
    exit 2
fi

export PATH="$runtime_home/.local/bin:${PATH-}"
configuration="$launcher_directory/runtime/picar-x.conf"
resources="$workspace_root/xWalk-rpi5/xWalkAudioResources"
executable="$launcher_directory/cmake/xWalkController/xWalkApp/xwalk-picarx-control"

exec "$executable" --deployment-config="$configuration" --resource-directory="$resources" "$@"
EOF
chmod 0755 "$temporary_launcher"
mv -- "$temporary_launcher" "$launcher"

echo "Generated $runtime_directory/picar-x.conf and $runtime_directory/picar-x.d"
echo "Generated $launcher"
