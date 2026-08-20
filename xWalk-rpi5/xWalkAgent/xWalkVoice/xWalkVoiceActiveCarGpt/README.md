# xWalkVoiceActiveCarGpt

`xWalkVoiceActiveCarGpt` adapts `example/21.voice_active_car_gpt.py` as the
Gemini-backed Jarvis profile over the shared `xWalkVoiceActiveCar`
sensor/action coordinator. It retains the ten-centimetre ultrasonic trigger,
English recognition profile, and bounded hardware composition. Jarvis is
permanently text-only after speech transcription and has no camera input.
The configured wake phrase is `hey jarvis`, and the cinematic AI-style wake
answer is `Systems online. Ready when you are, Joxy.`

The Raspberry Pi composition uses Gemini `gemini-3.6-flash` through Google's
OpenAI-compatible endpoint, the independently trained British male Piper voice
`en_GB-alan-medium`, Vosk microphone recognition, the Robot HAT status LED, and
the shared SelfDrive actions. `GEMINI_API_KEY`
exclusively supplies the credential; the key is never accepted through CLI
arguments, committed configuration, or diagnostics.

The first `hey jarvis` opens a bounded continuous session. Follow-up requests
do not repeat the wake phrase and use the same language-model history. The
session returns to wake mode after 30 seconds idle, ten successful rounds,
three consecutive recognition misses, cancellation, a terminal error, or one
of `goodbye jarvis`, `go to sleep`, and `stop listening`. Sleep phrases never
reach Gemini or action parsing. Session shutdown stops vehicle output.

Both wake and follow-up listens use incremental Vosk recognition. The installed
Vosk C API has no endpoint-timing setter, so xWalk uses native endpoints first
and a partial-transcript-armed trailing-silence fallback second. The existing
30-second listen timeout remains the hard safety upper bound. Normal traces
report timing decisions and transcript length without speech content; explicit
transcript tracing is privacy-sensitive and disabled by default.

Boot returns the Jarvis service graph before reading camera configuration or
constructing a camera backend or capture Agent. Gemini always receives an empty
image path. The tracked `voice_active_car_gpt_with_image = false` setting is a
validated policy lock; `true` is rejected rather than enabling capture. Jarvis
requests concise speech-friendly answers and defaults to at most 256 output
tokens without changing action syntax.

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
