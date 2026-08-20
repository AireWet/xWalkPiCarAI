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
voice_active_car_gpt_model = gemini-3.6-flash
voice_active_car_gpt_maximum_output_tokens = 256
voice_active_car_gpt_piper_model = /usr/share/xwalk/models/piper/en_GB-alan-medium.onnx
voice_active_car_gpt_with_image = false
voice_active_car_gpt_continuous_conversation = true
voice_active_car_gpt_conversation_idle_timeout_ms = 30000
voice_active_car_gpt_conversation_maximum_rounds = 10
voice_active_car_gpt_conversation_maximum_misses = 3
voice_active_car_gpt_sleep_phrases = "goodbye jarvis,go to sleep,stop listening"
voice_active_car_gpt_sleep_acknowledgement = "Going to sleep. Say hey Jarvis when you need me, Joxy."
```

The tracked `xWalkConfig/picar-x.d/voice.conf` keeps portable audio defaults:

```ini
voice_capture_device = default
voice_playback_device = default
voice_mixer_device = default
voice_mixer_element = PCM
voice_piper_executable = piper
voice_piper_playback_executable = aplay
voice_vosk_endpoint_start_seconds = 0.5
voice_vosk_endpoint_end_seconds = 1.0
voice_vosk_endpoint_max_seconds = 15.0
voice_vosk_silence_peak_threshold = 500
voice_vosk_trace_transcript = false
```

The default Raspberry Pi generation profile writes these deployment overrides
to `build-rpi/runtime/picar-x.d/voice.conf`:

```ini
voice_capture_device = plughw:CARD=Device,DEV=0
voice_mixer_device = pulse
voice_mixer_element = Master
voice_piper_executable = /opt/xwalk/piper-tts/venv/bin/piper
```

The installed Vosk 0.3.45 C API does not expose endpoint-timing setters. xWalk
therefore prefers Vosk's native endpoint and uses these values only for its
partial-transcript-armed fallback. Quiet initial input does not arm the
fallback. The 30-second listen timeout remains the absolute safety bound.

Leave transcript tracing disabled for normal use. Enabling it records recognized
speech and is suitable only for deliberate local diagnosis.

Jarvis is permanently text-only after speech transcription. Its Raspberry Pi
composition does not read camera configuration, initialize a camera backend,
create a capture Agent, produce a temporary image, or attach an image to Gemini.
The tracked image setting must remain `false`; validation rejects `true`.

The tracked `voice.conf` keeps portable `default` ALSA devices, the `PCM` mixer
element, and the `piper` command. Raspberry Pi runtime generation overrides
those values with the selected USB microphone, PulseAudio `Master`, and the
provisioned Piper path. Override another deployment during CMake configuration:

```bash
cmake --fresh -S xWalk-rpi5 --preset rpi-release -DXWALK_RPI_VOICE_CAPTURE_DEVICE=default -DXWALK_RPI_VOICE_MIXER_DEVICE=default -DXWALK_RPI_VOICE_MIXER_ELEMENT=PCM -DXWALK_RPI_PIPER_EXECUTABLE=piper
```

Regenerate an existing build-local runtime after tracked defaults or deployment
overrides change:

```bash
xWalkTool/shell-agent/deploy-tool/configure-rpi-runtime.sh --build-directory "$PWD/build-rpi" --runtime-user xwalk --generate-only
```

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
ask a question or request an allowed robot action. Follow-up questions do not
need another wake phrase until the 30-second idle limit, ten-round limit, or
three-miss limit ends the session. Say `Goodbye Jarvis`, `Go to sleep`, or
`Stop listening` to return immediately to wake mode; those phrases are not sent
to Gemini or interpreted as actions. Every session exit stops vehicle movement.
Jarvis addresses Joxy in every spoken or keyboard-chat reply. Press `Ctrl+C` to
stop.
