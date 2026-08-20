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
    --runtime-user "$(id -un)" \
    --ollama-manifest "$manifest" \
    --generate-only >/dev/null

configuration="$build_directory/runtime/picar-x.conf"
vision="$build_directory/runtime/picar-x.d/vision.conf"
hardware="$build_directory/runtime/picar-x.d/hardware.conf"
voice="$build_directory/runtime/picar-x.d/voice.conf"
features="$build_directory/runtime/picar-x.d/ai/features.conf"
ollama="$build_directory/runtime/picar-x.d/ai/providers/ollama.conf"
launcher="$build_directory/xwalk"

test -f "$configuration"
grep -Fxq "camera_connection = csi" "$vision"
grep -Fxq "camera_csi_executable = $runtime_home/.local/bin/rpicam-still" "$vision"
grep -Fxq "camera_csi_device = /dev/media0" "$vision"
grep -Fxq "hardware_board = robot_hat_v4" "$hardware"
grep -Fxq "hardware_gpio_device = /dev/gpiochip4" "$hardware"
grep -Eq '^hardware_gpio_chip_name =[[:space:]]*$' "$hardware"
grep -Eq '^hardware_gpio_chip_label =[[:space:]]*$' "$hardware"
grep -Fq "/xWalk-rpi5/xWalkLibrary/aarch64/lib/libvosk.so" "$voice"
grep -Fxq "voice_capture_device = plughw:CARD=Device,DEV=0" "$voice"
grep -Fxq "voice_mixer_device = pulse" "$voice"
grep -Fxq "voice_mixer_element = Master" "$voice"
grep -Fxq "voice_piper_executable = /opt/xwalk/piper-tts/venv/bin/piper" "$voice"
grep -Fxq "voice_active_car_model = gpt-4o-mini" "$features"
grep -Fxq "gpt_car_model = gpt-4o" "$features"
grep -Fxq "voice_active_car_gpt_model = gemini-3.6-flash" "$features"
grep -Fxq "voice_active_car_gpt_maximum_output_tokens = 256" "$features"
grep -Fxq "voice_active_car_gpt_with_image = false" "$features"
grep -Fxq "voice_active_car_gpt_continuous_conversation = true" "$features"
grep -Fxq "voice_active_car_gpt_conversation_idle_timeout_ms = 30000" "$features"
grep -Fxq "voice_active_car_gpt_conversation_maximum_rounds = 10" "$features"
grep -Fxq 'voice_active_car_gpt_sleep_phrases = "goodbye jarvis,go to sleep,stop listening"' "$features"
grep -Fxq "voice_language_model_provider = ollama" "$ollama"
grep -Fxq "voice_language_model_endpoint = http://127.0.0.1:11434/api/chat" "$ollama"
grep -Fxq "voice_language_model_model = llama3.2:3b" "$ollama"
grep -Fxq "voice_ollama_model = llama3.2:3b" "$ollama"
grep -Fxq "voice_ollama_model_manifest = $manifest" "$ollama"
grep -Fq "export PATH=\"\$runtime_home/.local/bin:\${PATH-}\"" "$launcher"
grep -Fq "exec \"\$executable\" --deployment-config=\"\$configuration\" --resource-directory=\"\$resources\" \"\$@\"" \
    "$launcher"
bash -n "$launcher"

source_checksum="$(find "$script_directory/../../../../xWalk-rpi5/xWalkController/xWalkConfig" -type f \
    -exec sha256sum {} + | sort | sha256sum)"
"$script_directory/../generate-rpi-runtime.sh" \
    --build-directory "$build_directory" \
    --runtime-user "$(id -un)" \
    --profile robot_hat_v5 \
    --gpio-device /dev/gpiochip7 \
    --i2c-device /dev/i2c-3 \
    --spi-device /dev/spidev2.1 \
    --camera usb \
    --ollama-manifest "$manifest" >/dev/null
grep -Fxq "hardware_board = robot_hat_v5" "$hardware"
grep -Fxq "hardware_gpio_device = /dev/gpiochip7" "$hardware"
grep -Fxq "hardware_i2c_device = /dev/i2c-3" "$hardware"
grep -Fxq "hardware_spi_device = /dev/spidev2.1" "$hardware"
grep -Fxq "camera_connection = usb" "$vision"
test "$source_checksum" = "$(find "$script_directory/../../../../xWalk-rpi5/xWalkController/xWalkConfig" \
    -type f -exec sha256sum {} + | sort | sha256sum)"

echo "RPi user-local runtime generation test passed"
