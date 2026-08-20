# xWalkVoiceActiveCarGpt

`xWalkVoiceActiveCarGpt` adapts `example/21.voice_active_car_gpt.py` as the
provider-neutral Jarvis profile over the shared `xWalkVoiceActiveCar`
sensor/action coordinator. It retains the ten-centimetre ultrasonic trigger,
English recognition profile, and bounded hardware composition. Jarvis is
permanently text-only after speech transcription and has no camera input.
The configured wake phrase is `hey jarvis`, and the cinematic AI-style wake
answer is `Systems online. Ready when you are, Joxy.`

The Raspberry Pi composition defaults to local Ollama `llama3.2:3b` through
`http://127.0.0.1:11434/api/chat`, the independently trained British male Piper voice
`en_GB-alan-medium`, Vosk microphone recognition, the Robot HAT status LED, and
the shared SelfDrive actions. Local Ollama requires no API key. Gemini remains
an optional HTTPS deployment and reads `GEMINI_API_KEY` only when selected.

The first `hey jarvis` opens a bounded continuous session. Follow-up requests
do not repeat the wake phrase and use the same language-model history. The
session returns to wake mode after 30 seconds idle, ten successful rounds,
three consecutive recognition misses, cancellation, a terminal error, or one
of `goodbye jarvis`, `go to sleep`, and `stop listening`. Sleep phrases never
reach the selected model or action parsing. Session shutdown stops vehicle output.

Both wake and follow-up listens use incremental Vosk recognition. The installed
Vosk C API has no endpoint-timing setter, so xWalk uses native endpoints first
and a partial-transcript-armed trailing-silence fallback second. The existing
30-second listen timeout remains the hard safety upper bound. Normal traces
report timing decisions and transcript length without speech content; explicit
transcript tracing is privacy-sensitive and disabled by default.

Boot returns the Jarvis service graph before reading camera configuration or
constructing a camera backend or capture Agent. The model always receives an empty
image path. The tracked `voice_active_car_gpt_with_image = false` setting is a
validated policy lock; `true` is rejected rather than enabling capture. Jarvis
requests concise speech-friendly answers and defaults to at most 256 output
tokens without changing action syntax.

Jarvis may return only the exact locally allowlisted actions for bounded
directions, horn and engine sounds, expression gestures, and background-music
start or stop. Unsupported action names are rejected by `XWalkSelfDrive`.
Emoji remain response text and never become executable actions.
Each completed model response is split into spoken response text and filtered
actions. Piper synthesizes the response through the configured playback device
while the SelfDrive worker executes accepted actions. The profile does not
clone or impersonate an actor's voice.

Jarvis also answers ordinary safe questions from local model knowledge; a question does
not need to request vehicle movement. For a conversational answer, the model puts
the answer in the response-text section and emits `stop` as the fail-safe
action. Every spoken or keyboard-chat response addresses the user as Joxy. The
action metadata is never spoken and cannot expand the local allowlist. Local
answers do not imply internet access, and Jarvis admits when current facts
cannot be verified.

Optional current-information retrieval uses only a deployment-controlled,
loopback SearXNG `/search` JSON endpoint. The HAL client bounds time, bytes, and
result count, strips markup, rejects unsafe result URLs, never follows result
links, and marks reference text as untrusted. Retrieval-assisted rounds force
the local `stop` action so web content cannot move the vehicle. Search failure
does not terminate Jarvis and does not fabricate current facts or citations.

## Source layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarVoiceActiveCarGpt.h` | Immutable example-21 defaults |
| `src/xAgent_Rpi5CarVoiceActiveCarGpt.cpp` | Full prompt and profile construction |
| `test/src/xAgent_Rpi5CarVoiceActiveCarGptTest.cpp` | Device-free profile verification |
