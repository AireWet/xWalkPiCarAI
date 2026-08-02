# xWalkBoot

`xWalkBoot` owns the process-level boundary between platform hardware and Agent
coordinators. The host target supplies a device-free stub. The optional
`xWalkBootRpi` target composes the complete hardware graph used by the CLI.

The shared `XWalkBoot` base owns callback validation and the one-shot lifecycle
guard. Platform implementations reuse that lifecycle instead of placing boot
state in an application controller.

The Raspberry Pi object does not claim resources during construction. Its
one-shot `run()` call detects the Robot HAT, resets the MCU, constructs the
selected motor topology and command-specific services, and invokes the
application callback. Every service remains valid until that callback returns.
The graph is then destroyed in reverse construction order. The automatic boot
object itself is destroyed when `main()` exits.

Boot modes are intentionally bounded:

| Mode | Services |
| --- | --- |
| `Base` | PiCar-X and its required HAL graph |
| `Doctor` | Passive report only; no actuator or service ownership |
| `LineTracking` | Base plus `XWalkLineTracking` |
| `SelfDrive` | Base plus ALSA Music and `XWalkSelfDrive` |
| `Sound` | Base plus ALSA Music |
| `VoiceChat` | Base plus Vosk, Espeak, ALSA, configured HTTP model, and VoiceAssistant |
| `VoiceActiveCar` | VoiceChat graph plus Music, SelfDrive, status LED, and still camera |
| `VoiceActiveCarGpt` | VoiceActiveCar graph plus the English Buddy profile |
| `VoiceControlledCar` | Base plus Vosk, ALSA capture, and speech recognition |
| `VoicePromptCar` | Base plus Espeak, ALSA playback, and speaker control |
| `SpiTransfer` | Configured Linux SPI backend and SPI transfer Agent only |

The RPi graph includes device discovery, Linux I2C and GPIO, MCU reset, speaker
power, PWM, motors, servos, ADC, grayscale, ultrasonic sensing, persistent
configuration, and command-specific audio decoding or playback. Voice-active
modes additionally claim the status LED and selected still-camera backend.
Camera capture uses CSI through `rpicam-still` or a USB V4L2 webcam through
`ffmpeg`. Buzzer and user-button modules are not claimed by CLI commands.
SPI mode bypasses the base graph: it does not detect or reset the HAT and does
not claim I2C, GPIO, motors, servos, audio, camera, speech, or model resources.
Doctor mode also bypasses the base graph. It may read firmware and ADC battery
data, but it never requests GPIO lines, performs SPI transfers, enables audio,
captures media, resets the MCU, constructs actuators, or contacts model services.

Deployment configuration is loaded before any Linux device is opened. Boot
uses only the configured I2C, GPIO, and Device Tree paths and never selects the
first `/dev/gpiochip*`. Optional exact kernel chip name and label values are
verified together with a minimum 28-line controller size before any GPIO line
is claimed. Automatic board selection fails before MCU reset when no supported
Robot HAT UUID is detected.

All voice modes are fully composed. Deployment values are read from the shared
PiCar-X configuration file:

| Key | Default |
| --- | --- |
| `hardware_board` | `auto` |
| `hardware_i2c_device` | `/dev/i2c-1` |
| `hardware_gpio_device` | `/dev/gpiochip0` |
| `hardware_device_tree_root` | `/proc/device-tree` |
| `hardware_gpio_chip_name` | empty; no name verification |
| `hardware_gpio_chip_label` | empty; no label verification |
| `hardware_spi_device` | `/dev/spidev0.0` |
| `hardware_spi_speed_hz` | `500000` |
| `hardware_spi_mode` | `0` |
| `hardware_spi_bits_per_word` | `8` |
| `voice_vosk_library` | `libvosk.so` |
| `voice_vosk_model` | `/usr/share/vosk-model-small-en-us-0.15` |
| `voice_capture_device` | `default` |
| `voice_playback_device` | `default` |
| `voice_mixer_device` | `default` |
| `voice_mixer_element` | `PCM` |
| `voice_espeak_executable` | `espeak-ng` |
| `voice_espeak_voice` | `en` |
| `voice_language_model_provider` | `ollama` |
| `voice_language_model_endpoint` | `http://127.0.0.1:11434/api/chat` |
| `voice_language_model_model` | `qwen2.5:0.5b` |
| `voice_language_model_api_key` | empty |
| `voice_language_model_maximum_output_tokens` | `1024` |
| `voice_ollama_model_manifest` | empty; provision the local manifest path |
| `camera_connection` | `csi` |
| `camera_csi_executable` | `rpicam-still` |
| `camera_csi_device` | `/dev/media0` |
| `camera_usb_executable` | `ffmpeg` |
| `camera_usb_device` | `/dev/video0` |
| `camera_output` | `/tmp/xwalk-voice-image.jpg` |

`hardware_board` accepts `auto`, `robot_hat_v4`, or `robot_hat_v5`. The v4
value is the explicit legacy selection because the current Device Tree detector
recognizes only the v5 UUID. The v5 value requires successful v5 detection.
`auto` also requires v5 detection and never falls back to v4.

`XWalkBoardControl`, its reset GPIO, and the speaker-enable GPIO now remain
alive for the complete boot callback. This guarantees that Robot HAT speaker
power remains claimed while Espeak PCM is played through ALSA. The Vosk library,
model, microphone capture adapter, Espeak provider, selected model backend, ALSA
owner, and speech coordinators are destroyed in reverse order after the command
returns.

The selected provider applies to `voice-chat`, `voice-active-car`, and
`voice-active-car-gpt`. Supported provider names are `ollama`, `openai`,
`chatgpt`, `gemini`, `claude`, `anthropic`, and `openai_compatible`. Cloud
providers require HTTPS, an endpoint ending in `/chat/completions`, a model,
and an API key. Ollama requires an endpoint ending in `/api/chat` and no key.
Legacy `voice_ollama_endpoint` and `voice_ollama_model` values remain readable
when their generic replacements are absent.

Kiro is rejected intentionally. Its documented command-line interface offers
non-interactive chat but no documented, model-selectable HTTP inference contract
that this embedded backend can validate. Do not map Kiro to another vendor's
wire protocol without an explicit supported API.

## Source layout

```text
core/include/     Shared boot lifecycle and service types
core/src/         Shared boot lifecycle implementation
hardware/include/ Raspberry Pi boot contract
hardware/src/     Raspberry Pi composition and passive Doctor inspection
stub/include/     Device-free host-stub contract
stub/src/         Device-free host-stub implementation
stub/test/include/ Host-only test fixtures and callback declarations
stub/test/src/    Deterministic host lifecycle coverage
```

## Host verification

```bash
cmake -S xWalkAgent/xWalkBoot -B xWalkAgent/xWalkBoot/build-host -DXWALK_BOOT_BUILD_HOST_TESTS=ON
cmake --build xWalkAgent/xWalkBoot/build-host --parallel
ctest --test-dir xWalkAgent/xWalkBoot/build-host --output-on-failure
```

## Raspberry Pi compilation

Use the `xWalkAgent` or `xWalkCLI` aggregate RPi build. Hardware tests remain
opt-in and must only be listed during ordinary verification.
