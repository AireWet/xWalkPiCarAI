# Gemini Jarvis configuration

## 1. Store the Gemini API key

Create `$HOME/.netrc` for runtime user `xwalk`:

```text
machine generativelanguage.googleapis.com
    login apikey
    password REPLACE_WITH_THE_GEMINI_API_KEY
```

```bash
chmod 600 "$HOME/.netrc"
source xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
test -n "${GEMINI_API_KEY:-}" && echo "GEMINI_API_KEY loaded"
```

Never commit or print the API key.

## 2. Install the Jarvis voice

```bash
sudo -u xwalk python3 -m pip install --user piper-tts
sudo install -d -o xwalk -g xwalk /usr/share/xwalk/models/piper
sudo -u xwalk python3 -m piper.download_voices en_GB-alan-medium --data-dir /usr/share/xwalk/models/piper
```

Verify Piper, ALSA playback, and both model files:

```bash
sudo -u xwalk bash -lc 'command -v piper && command -v aplay && test -r /usr/share/xwalk/models/piper/en_GB-alan-medium.onnx && test -r /usr/share/xwalk/models/piper/en_GB-alan-medium.onnx.json'
```

## 3. Configure Jarvis

Set these values in `xWalkConfig/picar-x.d/ai/features.conf`:

```ini
voice_active_car_gpt_api_key_environment = GEMINI_API_KEY
voice_active_car_gpt_endpoint = https://generativelanguage.googleapis.com/v1beta/openai/chat/completions
voice_active_car_gpt_model = gemini-3.7-flash
voice_active_car_gpt_maximum_output_tokens = 1024
voice_active_car_gpt_piper_model = /usr/share/xwalk/models/piper/en_GB-alan-medium.onnx
```

Confirm these values in `xWalkConfig/picar-x.d/voice.conf`:

```ini
voice_playback_device = default
voice_mixer_device = default
voice_mixer_element = PCM
voice_piper_executable = piper
voice_piper_playback_executable = aplay
```

If `build-rpi/runtime` already exists, set the same Jarvis values in
`build-rpi/runtime/picar-x.d/ai/features.conf`. CMake preserves existing runtime
configuration.

## 4. Build and check

```bash
cmake --fresh -S xWalk-rpi5 --preset rpi-release
cmake --build build-rpi/cmake --parallel
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --deployment-config="$PWD/build-rpi/runtime/picar-x.conf" --validate-config
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control doctor
```

## 5. Start Jarvis

Place the vehicle safely with its wheels clear before enabling voice actions.

```bash
source xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control voice-active-car-gpt start
```

Say `Hey Jarvis`, wait for `Systems online. Ready when you are, Joxy.`, and then
ask a question or request an allowed robot action. Say `Hey Jarvis` again before
each new request. Jarvis addresses Joxy in every spoken or keyboard-chat reply.
Press `Ctrl+C` to stop.
