# xWalkVoiceActiveCar

This Agent contains the sensor-aware PiCar-X voice behavior:

- `XWalkVoiceActiveCar` coordinates
  sensors, model responses, preset actions, optional images, and LED states.
The coordinators own no physical device or provider backend. Speech recognition
and speech synthesis are injected through the existing xWalk HAL services.
On Raspberry Pi, `xWalkBootRpi` supplies Vosk recognition, ALSA capture and
playback, Espeak synthesis, the configured language-model HTTP backend, Music,
SelfDrive, speaker power,
the Robot HAT status LED, and a deployment-selected CSI or USB camera according
to the selected command. `XWalkCameraCapture` supplies the image callback. The
Agent remains portable and device-free despite the complete target composition.

`XWalkVoiceActiveCar` is the canonical port of
`example/voice_active_car.py`. Its profile preserves the Rolly identity,
`hey rolly` wake phrase, `Hi there` wake response, 10-centimetre ultrasonic
trigger, image-enabled rounds, English recognition setting, OpenAI
`gpt-4o-mini` model, welcome message, complete instructions, LED lifecycle,
response parser, and preset-action dispatch. The OpenAI credential is read
only from `OPENAI_API_KEY` by the Raspberry Pi composition.

The shared coordinator also supports opt-in bounded continuous conversation.
A wake phrase opens the session, follow-up prompts are accepted without another
wake phrase, and the existing language-model object retains its configured
history. Idle time, successful-round count, and consecutive recognition misses
bound the session. A trimmed case-insensitive sleep phrase ends the session
before model, camera, or action processing. Every session exit, cancellation,
and terminal error stops vehicle output. Profiles that leave continuous mode
disabled retain the original wake-before-every-request behavior.

Image attachment is a profile setting. Disabled profiles do not call the image
callback; the Raspberry Pi composition also avoids constructing the still-camera
provider, eliminating camera startup and capture timeout from text-only rounds.

The module-local `config/voice-active-car.jpg` is the deterministic host image.
Its directory is selected by `XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY`; tests
load and validate that JPEG instead of referring to `/tmp`. Physical camera
captures still use the writable deployment `camera_output` setting.

The standalone [`xWalkVoicePromptCar`](../xWalkVoicePromptCar) module owns the
spoken movement demonstration port from upstream example 14.

The standalone [`xWalkVoiceControlledCar`](../xWalkVoiceControlledCar) module
owns the wake-word movement port from upstream example 16.
