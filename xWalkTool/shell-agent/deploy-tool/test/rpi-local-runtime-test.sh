#!/usr/bin/env bash

set -eu

script_directory="$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)"
configuration_script="$script_directory/../configure-rpi-runtime.sh"
test_directory="$(mktemp -d)"
cleanup() {
    rm -rf -- "$test_directory"
}
trap cleanup EXIT HUP INT TERM

build_directory="$test_directory/build-rpi"
runtime_home="$(getent passwd "$(id -un)" | awk -F: 'NR == 1 { print $6 }')"
manifest="$runtime_home/.local/share/ollama/models/manifests/registry.ollama.ai/library/llama3.2/3b"
"$configuration_script" \
    --build-directory "$build_directory" \
    --ollama-manifest "$manifest" \
    --generate-only >/dev/null

configuration="$build_directory/runtime/picar-x.conf"
vision="$build_directory/runtime/picar-x.d/vision.conf"
hardware="$build_directory/runtime/picar-x.d/hardware.conf"
voice="$build_directory/runtime/picar-x.d/voice.conf"
ollama="$build_directory/runtime/picar-x.d/ai/providers/ollama.conf"
launcher="$build_directory/xwalk"

test -f "$configuration"
grep -Fxq "camera_connection = csi" "$vision"
grep -Fxq "camera_csi_executable = $runtime_home/.local/bin/rpicam-still" "$vision"
grep -Fxq "camera_csi_device = /dev/media0" "$vision"
grep -Fxq "hardware_board = robot_hat_v4" "$hardware"
grep -Fxq "hardware_gpio_device = /dev/gpiochip4" "$hardware"
grep -Fxq "hardware_gpio_chip_name = gpiochip4" "$hardware"
grep -Fxq "hardware_gpio_chip_label = pinctrl-rp1" "$hardware"
grep -Fq "/xWalk-rpi5/xWalkLibrary/aarch64/lib/libvosk.so" "$voice"
grep -Fxq "voice_language_model_provider = ollama" "$ollama"
grep -Fxq "voice_ollama_model = llama3.2:3b" "$ollama"
grep -Fxq "voice_ollama_model_manifest = $manifest" "$ollama"
grep -Fq "export PATH=\"\$runtime_home/.local/bin:\${PATH-}\"" "$launcher"
grep -Fq "exec \"\$executable\" --deployment-config=\"\$configuration\" --resource-directory=\"\$resources\" \"\$@\"" \
    "$launcher"
bash -n "$launcher"

echo "RPi user-local runtime generation test passed"
