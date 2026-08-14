# xWalkBoot

`xWalkBoot` owns the process-level boundary between platform hardware and Agent
coordinators. The host target supplies a device-free stub. The optional
`xWalkBootRpi` target composes the complete hardware graph used by the CLI.

The shared `XWalkBoot` base owns callback validation and the one-shot lifecycle
guard. Platform implementations reuse that lifecycle instead of placing boot
state in an application controller.

The Raspberry Pi object does not claim resources during construction. Its
one-shot `run()` call loads the layered deployment configuration and delegates
to one mode-specific source file. Actuator modes
detect the Robot HAT, reset the MCU, and construct the selected motor topology;
camera-only and passive modes bypass those steps. Every service remains valid
until the application callback returns.
The graph is then destroyed in reverse construction order. The automatic boot
object itself is destroyed when `main()` exits.

Boot modes are intentionally bounded:

| Mode | Services |
| --- | --- |
| `Base` | PiCar-X and its required HAL graph |
| `Doctor` | Passive report only; no actuator or service ownership |
| `ComputerVision` | OpenCV camera provider and `XWalkComputerVision` only |
| `FaceTracking` | Base PiCar-X graph, OpenCV provider, and `XWalkFaceTracking` |
| `BullFight` | Base PiCar-X graph, OpenCV red detection, and `XWalkBullFight` |
| `TreasureHunt` | Base PiCar-X, OpenCV color detection, Pico2Wave, and spoken color game |
| `VideoRecording` | OpenCV camera and continuous AVI recording only |
| `VideoCar` | Base PiCar-X graph, OpenCV camera, and interactive driving |
| `AppControl` | Base PiCar-X, explicit WebSocket transport, camera, and sound |
| `SoundBackgroundMusic` | Shared ALSA music graph and interactive example 13 Agent |
| `LineTracking` | Base plus `XWalkLineTracking` |
| `SelfDrive` | Base plus ALSA Music and `XWalkSelfDrive` |
| `Sound` | Base plus ALSA Music |
| `VoiceChat` | Base plus Vosk, Piper, local Ollama, and VoiceAssistant |
| `VoiceActiveCar` | Base voice graph plus Music, SelfDrive, status LED, and still camera |
| `VoiceActiveCarGpt` | VoiceActiveCar graph plus the English Buddy profile |
| `GptCar` | VoiceActiveCar graph plus the upstream JSON GPT-car profile |
| `VoiceControlledCar` | Base plus Vosk, ALSA capture, and speech recognition |
| `VoicePromptCar` | Base plus Espeak, ALSA playback, and speaker control |
| `StorytellingRobot` | Base plus Piper WAV synthesis, playback, and speaker control |
| `TextVisionTalk` | Camera-only still capture plus local Ollama vision model |
| `OnlineLlmTest` | OpenAI-compatible HTTPS language-model provider only |
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
ComputerVision mode also bypasses the base graph and claims only the configured
camera stream. It does not inspect or reset the Robot HAT or claim motors, GPIO,
I2C, SPI, audio, speech, model, or network services.
VideoRecording mode has the same camera-only boundary and additionally owns a
local AVI writer and capture worker for the duration of the command.

ServoZeroing mode resets the Robot HAT MCU and owns PWM and Servo objects for
channels 0 through 11. It publishes only the dedicated servo-zeroing Agent and
does not construct the PiCar-X motor, sensor, or camera graph.

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
| `hardware_gpio_minimum_line_count` | `28` |
| `hardware_mcu_reset_pin` | `MCURST` |
| `hardware_mcu_reset_settle_ms` | `200` |
| `hardware_battery_adc_channel` | `A4` |
| `hardware_pan_pwm_channel` | `P0` |
| `hardware_tilt_pwm_channel` | `P1` |
| `hardware_direction_pwm_channel` | `P2` |
| `hardware_ultrasonic_trigger_pin` | `D2` |
| `hardware_ultrasonic_echo_pin` | `D3` |
| `hardware_status_led_pin` | `LED` |
| `hardware_spi_device` | `/dev/spidev0.0` |
| `hardware_spi_speed_hz` | `500000` |
| `hardware_spi_mode` | `0` |
| `hardware_spi_bits_per_word` | `8` |
| `voice_vosk_library` | `/usr/lib/xwalk/libvosk.so` |
| `voice_vosk_model` | `/usr/share/xwalk/models/vosk/vosk-model-small-en-us-0.15` |
| `voice_capture_device` | `default` |
| `voice_playback_device` | `default` |
| `voice_mixer_device` | `default` |
| `voice_mixer_element` | `PCM` |
| `voice_espeak_executable` | `espeak-ng` |
| `voice_espeak_voice` | `en` |
| `local_voice_chatbot_piper_model` | `en_US-amy-low` |
| `local_voice_chatbot_ollama_endpoint` | `http://127.0.0.1:11434/api/chat` |
| `local_voice_chatbot_ollama_model` | `llama3.2:3b` |
| `treasure_hunt_pico2wave_executable` | `pico2wave` |
| `treasure_hunt_playback_executable` | `aplay` |
| `treasure_hunt_language` | `en-US` |
| `voice_language_model_provider` | `ollama` |
| `voice_language_model_endpoint` | `http://127.0.0.1:11434/api/chat` |
| `voice_language_model_model_environment` | `OLLAMA_MODEL` |
| `voice_language_model_api_key_environment` | empty for Ollama |
| `voice_language_model_maximum_output_tokens` | `1024` |
| `voice_language_model_timeout_ms` | `120000` |
| `voice_active_car_gpt_endpoint` | `https://api.openai.com/v1/chat/completions` |
| `voice_active_car_gpt_model` | `gpt-4o-mini` |
| `voice_active_car_gpt_maximum_output_tokens` | `1024` |
| `voice_active_car_gpt_piper_model` | `en_US-ryan-low` |
| `voice_ollama_model_manifest` | empty; provision the local manifest path |
| `camera_connection` | `csi` |
| `camera_csi_executable` | `rpicam-still` |
| `camera_csi_device` | `/dev/media0` |
| `camera_usb_executable` | `ffmpeg` |
| `camera_usb_device` | `/dev/video0` |
| `camera_output` | `/tmp/xwalk-voice-image.jpg` |
| `camera_width` | `640` |
| `camera_height` | `480` |
| `camera_timeout_ms` | `5000` |
| `picarx_motor_watchdog_timeout_ms` | `500`; valid range 1 through 60000 |
| `computer_vision_camera_backend` | `v4l2` |
| `computer_vision_camera_device` | `/dev/video0` |
| `computer_vision_photo_directory` | `/tmp/xwalk-pictures` |
| `computer_vision_face_cascade` | OpenCV frontal-face cascade under `/usr/share/opencv4` |
| `computer_vision_width` | `640` |
| `computer_vision_height` | `480` |
| `video_recording_camera_backend` | `v4l2` |
| `video_recording_camera_device` | `/dev/video0` |
| `video_recording_directory` | `/tmp/xwalk-videos` |
| `video_recording_fps` | `20` |
| `text_vision_width` | `1280` |
| `text_vision_height` | `720` |
| `local_voice_chatbot_maximum_messages` | `20` |

OpenCV source values are `v4l2`, `gstreamer`, `video_file`,
`image_sequence`, and `automatic`. The source string is mandatory: path-based
backends require an absolute path, while GStreamer receives a validated
pipeline string directly through OpenCV without invoking a shell.

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

The dedicated `voice-chat` composition preserves example 19 with local Ollama,
Piper, and a 20-message history. The `voice-active-car` mode preserves the
Rolly profile from `example/voice_active_car.py`, uses OpenAI `gpt-4o-mini`,
and reads its credential from `OPENAI_API_KEY`. Example 21's
`voice-active-car-gpt` mode also uses OpenAI `gpt-4o-mini` and uses Piper
`en_US-ryan-low`. Supported generic provider names are `ollama`, `openai`,
`chatgpt`, `gemini`, `grok`, `xai`, `claude`, `anthropic`, and
`openai_compatible`. Cloud
providers require HTTPS, an endpoint ending in `/chat/completions`, a model,
and a non-empty credential in the provider-configured environment variable.
Ollama requires an endpoint ending in `/api/chat` and no key.
The tracked Grok profile uses xAI's OpenAI-compatible
`https://api.x.ai/v1/chat/completions` endpoint and reads `XAI_API_KEY` and
`XAI_MODEL` only from the deployment environment.
Legacy `voice_ollama_endpoint` and `voice_ollama_model` values remain readable
when their generic replacements are absent.

The primary controller configuration is a writable manifest. It loads
function-specific fragments and exactly one generic provider profile from
`picar-x.d/ai/providers`; the tracked default selects Ollama. Primary-file
values appearing after the includes override fragment defaults.

Kiro is rejected intentionally. Its documented command-line interface offers
non-interactive chat but no documented, model-selectable HTTP inference contract
that this embedded backend can validate. Do not map Kiro to another vendor's
wire protocol without an explicit supported API.

## Source layout

```text
core/include/     Shared boot lifecycle and service types
core/src/         Shared boot lifecycle implementation
hardware/include/ Raspberry Pi boot contract
hardware/src/     One Raspberry Pi composition source per boot mode plus shared vehicle wiring
stub/include/     Device-free host-stub contract
stub/src/         Device-free host-stub implementation
stub/test/include/ Host-only test fixtures and callback declarations
stub/test/src/    Deterministic host lifecycle coverage
```

The Raspberry Pi implementation files have one composition responsibility:

| Source suffix | Responsibility |
| --- | --- |
| `BootRpi.cpp` | Lifecycle validation and top-level mode dispatch |
| `BootRpiBoard.cpp` | Robot HAT deployment selection |
| `BootRpiVehicle.cpp` | Shared I2C, GPIO, sensor, servo, and motor graph |
| `BootRpiVehicleMode.cpp` | Dispatch after the common PiCar-X graph exists |
| `BootRpiBase.cpp` | Base PiCar-X service |
| `BootRpiDoctor.cpp` | Passive deployment inspection |
| `BootRpiSpiTransfer.cpp` | Isolated SPI service |
| `BootRpiServoZeroing.cpp` | Twelve-channel servo-zeroing graph |
| `BootRpiComputerVision.cpp` | Camera-only interactive computer vision |
| `BootRpiVideoRecording.cpp` | Camera-only recording graph |
| `BootRpiFaceTracking.cpp` | Face detection and camera-servo tracking |
| `BootRpiBullFight.cpp` | Red-target pursuit |
| `BootRpiTreasureHunt.cpp` | Color pursuit and Pico2Wave speech |
| `BootRpiVideoCar.cpp` | Camera-assisted driving |
| `BootRpiAppControl.cpp` | WebSocket, camera, audio, and PiCar-X control |
| `BootRpiLineTracking.cpp` | Foreground line tracking |
| `BootRpiSelfDrive.cpp` | Named actions with configured sound resources |
| `BootRpiSound.cpp` | Standalone Music service |
| `BootRpiSoundBackgroundMusic.cpp` | Interactive effects and background music |
| `BootRpiVoiceControlledCar.cpp` | Vosk wake-word movement input |
| `BootRpiVoicePromptCar.cpp` | Espeak movement prompts |
| `BootRpiStorytellingRobot.cpp` | Piper narration service |
| `BootRpiVoiceChat.cpp` | Local Vosk, Piper, and Ollama chatbot |
| `BootRpiTextVisionTalk.cpp` | Still camera and local vision model |
| `BootRpiOnlineLlmTest.cpp` | Environment-authenticated online model |
| `BootRpiVoiceActiveMode.cpp` | Shared speech, model, camera, LED, and action graph |
| `BootRpiVoiceActiveCar.cpp` | Rolly profile selection |
| `BootRpiVoiceActiveCarGpt.cpp` | Buddy profile selection |
| `BootRpiGptCar.cpp` | GPT-car profile selection |

## Host verification

```bash
cmake -S xWalk-rpi5/xWalkAgent/xWalkPlatform/xWalkBoot -B xWalk-rpi5/xWalkAgent/xWalkPlatform/xWalkBoot/build-host -DXWALK_BOOT_BUILD_HOST_TESTS=ON
cmake --build xWalk-rpi5/xWalkAgent/xWalkPlatform/xWalkBoot/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkAgent/xWalkPlatform/xWalkBoot/build-host --output-on-failure
```

## Raspberry Pi compilation

Use the `xWalkAgent` or `xWalkController` aggregate RPi build. Hardware tests remain
opt-in and must only be listed during ordinary verification.
