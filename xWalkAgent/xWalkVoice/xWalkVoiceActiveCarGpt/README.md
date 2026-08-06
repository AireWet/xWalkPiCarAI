# xWalkVoiceActiveCarGpt

`xWalkVoiceActiveCarGpt` ports `example/21.voice_active_car_gpt.py` as an
immutable profile over the shared `xWalkVoiceActiveCar` sensor/action
coordinator. It preserves the Buddy identity, ten-centimetre ultrasonic
trigger, image input, English recognition profile, `hey buddy` wake phrase,
`Hi there` wake answer, complete hardware and response instructions, and
source welcome text.

The Raspberry Pi composition uses OpenAI `gpt-4o-mini`, Piper
`en_US-ryan-low`, Vosk microphone recognition, still-image capture, the Robot
HAT status LED, and the shared SelfDrive actions. `OPENAI_API_KEY` exclusively
supplies the credential; the key is never accepted through CLI arguments,
committed configuration, or diagnostics.

## Source layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarVoiceActiveCarGpt.h` | Immutable example-21 defaults |
| `src/xAgent_Rpi5CarVoiceActiveCarGpt.cpp` | Full prompt and profile construction |
| `test/src/xAgent_Rpi5CarVoiceActiveCarGptTest.cpp` | Device-free profile verification |
