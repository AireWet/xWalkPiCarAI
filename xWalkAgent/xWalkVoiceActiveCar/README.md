# xWalkVoiceActiveCar

This Agent contains three voice-driven PiCar-X behaviors:

- `XWalkVoiceActiveCar` coordinates
  sensors, model responses, preset actions, optional images, and LED states.
- `XWalkVoiceControlledCar` waits
  for “hey robot”, accepts forward, backward, left, and right commands, and
  returns to wake-word mode after “sleep”.
- `XWalkVoicePromptCar` speaks and runs the forward, backward,
  left, and right demonstration sequence.

The coordinators own no physical device or provider backend. Speech recognition
and speech synthesis are injected through the existing xWalk HAL services.
On Raspberry Pi, `xWalkBootRpi` supplies Vosk recognition, ALSA capture and
playback, Espeak synthesis, the configured language-model HTTP backend, Music,
SelfDrive, speaker power,
the Robot HAT status LED, and a deployment-selected CSI or USB camera according
to the selected command. `XWalkCameraCapture` supplies the image callback. The
Agent remains portable and device-free despite the complete target composition.
