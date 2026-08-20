# xWalkVoiceActiveCarGpt

`xWalkVoiceActiveCarGpt` adapts `example/21.voice_active_car_gpt.py` as the
Gemini-backed Jarvis profile over the shared `xWalkVoiceActiveCar`
sensor/action coordinator. It retains the ten-centimetre ultrasonic trigger,
image input, English recognition profile, and bounded hardware composition.
The configured wake phrase is `hey jarvis`, and the cinematic AI-style wake
answer is `Systems online. Ready when you are, Joxy.`

The Raspberry Pi composition uses Gemini `gemini-3.7-flash` through Google's
OpenAI-compatible endpoint, the independently trained British male Piper voice
`en_GB-alan-medium`, Vosk microphone recognition, still-image capture, the
Robot HAT status LED, and the shared SelfDrive actions. `GEMINI_API_KEY`
exclusively supplies the credential; the key is never accepted through CLI
arguments, committed configuration, or diagnostics.

Both the `hey jarvis` wake listen and the following request use incremental
Vosk recognition. Each listen returns after Vosk accepts an utterance endpoint
following trailing silence; the existing 30-second setting remains a hard
safety upper bound and finalizes speech that continues until that limit. No new
runtime configuration is required for this behavior.

Jarvis may return only the exact locally allowlisted actions for bounded
directions, horn and engine sounds, expression gestures, and background-music
start or stop. Unsupported action names are rejected by `XWalkSelfDrive`.
Emoji remain response text and never become executable actions.
Each completed Gemini response is split into spoken response text and filtered
actions. Piper synthesizes the response through the configured playback device
while the SelfDrive worker executes accepted actions. The profile does not
clone or impersonate an actor's voice.

Jarvis also answers ordinary safe questions through Gemini; a question does
not need to request vehicle movement. For a conversational answer, Gemini puts
the answer in the response-text section and emits `stop` as the fail-safe
action. Every spoken or keyboard-chat response addresses the user as Joxy. The
action metadata is never spoken and cannot expand the local allowlist.

## Source layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarVoiceActiveCarGpt.h` | Immutable example-21 defaults |
| `src/xAgent_Rpi5CarVoiceActiveCarGpt.cpp` | Full prompt and profile construction |
| `test/src/xAgent_Rpi5CarVoiceActiveCarGptTest.cpp` | Device-free profile verification |
