# xExample

Provides the central home for C++ examples ported from the upstream Robot HAT
project. Examples are added one by one from user-supplied source paths without
moving their behavior into the unit-test runner implementation.

`xExample` is an example launcher, not a test executable. It has no XML test
profile and no CTest registration. Select an example by its formal selector
name; the default YAML file supplies its validated board, AI, and bounded
runtime arguments.

Configure and build it for Raspberry Pi without enabling the test framework:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON
cmake --build build-rpi --target xExample --parallel
./build-rpi/xExample
```

Calling it without a selector prints every accepted formal argument list and
returns without opening hardware.

Run an example by name using the copied default YAML configuration:

```sh
./build-rpi/xExample led
```

Select a deployment-specific YAML file with either supported form:

```sh
./build-rpi/xExample --config /etc/xwalk/xHal_Rpi5CarExampleConfig.yml led
./build-rpi/xExample --config=/etc/xwalk/xHal_Rpi5CarExampleConfig.yml led
```

The YAML file stores ordinary board paths, endpoints, model paths, bounded
counts, and executable names. It stores only environment-variable names for
credentials; secret values remain in the process environment. Existing
selector-specific positional arguments remain available as an explicit
compatibility override.

Each entry below `examples` is the exact formal argument list passed to the
existing selector validation. The `board` and `ai` anchors centralize values
shared by those lists. Provider settings that are not exposed as formal runner
arguments retain their existing CMake or source defaults.

## LED example

The `led` selector ports `robot-hat/examples/led_test.py`. It preserves the
six progress messages and the following bounded flow:

1. Turn the Robot HAT LED on and wait two seconds.
2. Turn it off and wait two seconds.
3. Blink one cycle with a one-second transition delay, then wait five seconds.
4. Blink three cycles with a 0.1-second transition delay and 0.5-second pause,
   then wait five seconds.
5. Blink two cycles with a 0.2-second transition delay and one-second pause,
   then wait five seconds before closing the LED.

After confirming the correct Raspberry Pi, Robot HAT, GPIO device, and visible
LED state are safe, run the example by its selector and formal arguments:

```sh
./build-rpi/xExample led /dev/gpiochip0 "" ""
```

The adapter claims the logical `LED` pin, which maps to GPIO26. Run it only
after confirming the connected hardware is safe.

## Pin-input example

The `pin-input` selector ports `robot-hat/examples/pin_input.py`. It preserves
the `D3` input, pull-up bias, printed zero-or-one values, and 100-millisecond
sampling interval. The C++ runner adds a required sample count from one through
36,000 so the upstream infinite loop becomes bounded.

After confirming the correct Raspberry Pi, Robot HAT, GPIO device, and D3
wiring are safe, run ten samples directly:

```sh
./build-rpi/xExample pin-input 10 /dev/gpiochip0 "" ""
```

`D3` maps to GPIO22. The physical adapter configures it as an input with an
internal pull-up and does not drive the line.

## Ultrasonic example

The `ultrasonic` selector ports `robot-hat/examples/ultrasonic.py`. It preserves
the D2 trigger, D3 echo, terminal-line refresh format, centimeter unit, and
200-millisecond sampling interval. The C++ runner adds a required sample count
from one through 18,000 so the upstream infinite loop becomes bounded.

After confirming the correct Raspberry Pi, Robot HAT, GPIO device, sensor
wiring, and clear sensing area, run five samples directly:

```sh
./build-rpi/xExample ultrasonic 5 /dev/gpiochip0 "" ""
```

`D2` maps to GPIO27 and is pulsed as the trigger. `D3` maps to GPIO22 and is
configured as a pull-down echo input. Hardware execution remains explicitly
opt-in.

## Voice-assistant example

The `voice-assistant` selector ports `robot-hat/examples/voice_assistant.py`.
It preserves the active `Buddy` name, `gpt-4o-mini` language model,
`en_US-ryan-low` Piper voice, `en-us` speech language, image support, keyboard
input, `hey buddy` wake phrase, `Hi there` acknowledgement, welcome text, and
system instructions.

The upstream implementation can monitor keyboard and wake input concurrently.
The bounded C++ runner makes the source explicit for each run: `keyboard` reads
terminal prompts, while `wake` listens for the exact wake phrase and then one
spoken prompt. Both paths capture a 640-by-480 image before each OpenAI request.

An explicitly approved one-round Raspberry Pi wake-mode run is:

```sh
OPENAI_API_KEY="sk-..." ./build-rpi/xExample voice-assistant
```

Use `keyboard` in place of `wake` for terminal input. Piper must resolve the
`en_US-ryan-low` voice files. The command captures microphone audio and images,
sends prompt data to OpenAI and produces audible output, so invoke it only with
explicit approval for the connected hardware and remote service.

## Servo example

The `servo` selector ports `robot-hat/examples/servo.py`. It preserves Robot
HAT servo channel one and each source range exactly: `-90` through `89`, then
`90` through `-89`. Every angle command waits 10 milliseconds and is reported
with a carriage return. Each half-sweep ends with a one-second pause.

The C++ runner adds a required cycle count from one through 100 so the upstream
infinite loop becomes bounded.

After confirming the servo is mounted with enough clearance, the Robot HAT is
correctly powered, and a full-range sweep is safe, run one cycle directly:

```sh
./build-rpi/xExample servo 1 /dev/i2c-1
```

One cycle issues 360 angle commands and takes approximately 5.6 seconds. Normal
completion leaves the last requested angle at -89 degrees, matching the source.

## DeepSeek language-model example

The `llm-deepseek` selector ports `robot-hat/examples/llm_deepseek.py`. It keeps
the upstream `deepseek-chat` model, system instructions, welcome message, and
20-message history limit. The C++ runner adds a required prompt limit from one
through 100 so an example run is bounded; end of input stops it earlier.

For an explicitly approved live run, place the credential only in the
`DEEPSEEK_API_KEY` environment variable and select a maximum prompt count:

```sh
export DEEPSEEK_API_KEY='<DeepSeek API key>'
./build-rpi/xExample llm-deepseek 5
```

Run the live command only with explicit approval because entered prompts leave
the machine over HTTPS and may incur provider charges. The key is
never accepted as a command-line argument or embedded in the source.

The existing HAL HTTP provider requests a completed OpenAI-compatible chat
response. Consequently, the port writes the completed response as one flushed
fragment instead of reproducing the Python source's token-by-token streaming.

## Doubao camera language-model example

The `llm-doubao-with-image` selector ports
`robot-hat/examples/llm_doubao_with_image.py`. It keeps the upstream
`doubao-seed-1-6-250615` model, instructions, welcome message, and 20-message
history. Each prompt captures a fresh JPEG into the configured build-local
`xWalkHal/xWalkTest/xExample/config/llm-img.jpg` path and submits it as an
OpenAI-compatible image message.

For an explicitly approved live CSI-camera run:

```sh
export DOUBAO_API_KEY='<Doubao API key>'
./build-rpi/xExample llm-doubao-with-image 5 csi rpicam-still /dev/video0
```

For a USB camera, select `usb`, a compatible capture executable such as
`ffmpeg`, and the required V4L2 device. Run the live command only with explicit
approval because the example operates a camera, uploads captured images, and
may incur provider charges. The
credential is read only from `DOUBAO_API_KEY`. The C++ camera backend starts a
bounded capture process for each image, so it does not require the Python
source's persistent-camera two-second startup delay.

As with the DeepSeek port, the current HTTP provider returns a completed
response instead of token-by-token streaming.

## Doubao text language-model example

The `llm-doubao` selector ports `robot-hat/examples/llm_doubao.py`. It keeps
the upstream `doubao-seed-1-6-250615` model, instructions, welcome message, and
20-message history without operating a camera or attaching images.

For an explicitly approved live run:

```sh
export DOUBAO_API_KEY='<Doubao API key>'
./build-rpi/xExample llm-doubao 5
```

The credential is read only from `DOUBAO_API_KEY`. A required prompt limit from
one through 100 bounds the source's otherwise infinite loop. The current HTTP
provider writes each completed response as one flushed fragment rather than
token-by-token streaming.

## Gemini language-model example

The `llm-gemini` selector ports `robot-hat/examples/llm_gemini.py`. It keeps the
upstream `gemini-2.5-flash` model, instructions, welcome message, and 20-message
history. The commented optional reasoning and Google-specific `extra_body`
settings are not enabled by the Python source and are therefore not added to
the C++ request.

For an explicitly approved live run:

```sh
export GEMINI_API_KEY='<Gemini API key>'
./build-rpi/xExample llm-gemini 5
```

The credential is read only from `GEMINI_API_KEY`. The live adapter uses
Google's OpenAI-compatible chat-completions endpoint. A required prompt limit
from one through 100 bounds execution, and the current HTTP provider writes
each completed response instead of streaming tokens.

## Grok language-model example

The `llm-grok` selector ports `robot-hat/examples/llm_grok.py`. It keeps the
upstream `grok-4-latest` model, instructions, welcome message, and 20-message
history.

For an explicitly approved live run:

```sh
export GROK_API_KEY='<Grok API key>'
./build-rpi/xExample llm-grok 5
```

The credential is read only from `GROK_API_KEY`. The live adapter uses xAI's
OpenAI-compatible chat-completions endpoint. A required prompt limit from one
through 100 bounds execution, and the current HTTP provider writes each
completed response instead of streaming tokens.

## Ollama text language-model example

The `llm-ollama` selector ports `robot-hat/examples/llm_ollama.py`. It keeps
the upstream `deepseek-r1:1.5b` model, instructions, welcome message, and
20-message history. The default endpoint is the upstream localhost service.

Override the text-only endpoint at configuration time when Ollama runs on
another approved host:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DXWALK_EXAMPLE_OLLAMA_TEXT_ENDPOINT=http://localhost:11434/api/chat
```

After installing Ollama, pulling `deepseek-r1:1.5b`, starting the service, and
confirming that the endpoint is safe, run:

```sh
./build-rpi/xExample llm-ollama 5
```

The required prompt limit from one through 100 bounds execution. The native
Ollama provider returns each completed response instead of streaming tokens.

## Ollama camera language-model example

The `llm-ollama-with-image` selector ports
`robot-hat/examples/llm_ollama_with_image.py`. It keeps the upstream `llava:7b`
model, instructions, welcome message, 20-message history, and 1280×720 still
capture. Each prompt captures a fresh JPEG into the configured build-local
`xWalkHal/xWalkTest/xExample/config/llm-img.jpg` path and submits it using Ollama's
native image-message format.

The upstream endpoint is retained as the default CMake setting. Override it at
configuration time when Ollama runs elsewhere:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DXWALK_EXAMPLE_OLLAMA_ENDPOINT=http://192.168.100.145:11434/api/chat
```

After confirming the camera and local-network endpoint are safe, run:

```sh
./build-rpi/xExample llm-ollama-with-image 5 csi rpicam-still /dev/video0
```

The source notes that `llava:7b` requires an Ollama installation and at least
8 GB of memory. The Linux camera backend starts one bounded capture process per
prompt instead of owning the persistent Picamera2 session used by Python. The
current HTTP provider returns completed responses rather than streamed tokens.

## OpenAI text language-model example

The `llm-openai` selector ports `robot-hat/examples/llm_openai.py`. It keeps
the upstream `gpt-4o` model, instructions, welcome message, and 20-message
history.

For an explicitly approved live run:

```sh
export OPENAI_API_KEY='<OpenAI API key>'
./build-rpi/xExample llm-openai 5
```

The credential is read only from `OPENAI_API_KEY`. A required prompt limit from
one through 100 bounds API requests. The Linux adapter uses OpenAI's
chat-completions endpoint and returns completed responses rather than streamed
tokens.

## Generic compatible-provider language-model example

The `llm-others` selector ports `robot-hat/examples/llm_others.py`. The Python
source intentionally leaves its provider base URL and model empty. The C++
port therefore requires a complete OpenAI-compatible chat-completions endpoint
and model selection at runtime while preserving the instructions, welcome
message, and 20-message history.

For an explicitly approved live run:

```sh
export LLM_API_KEY='<provider API key>'
./build-rpi/xExample llm-others 5 https://provider.example/v1/chat/completions deployment-model
```

Configure the endpoint and model used by the direct example command when
building the Raspberry Pi profile:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DXWALK_EXAMPLE_OTHERS_ENDPOINT=https://provider.example/v1/chat/completions -DXWALK_EXAMPLE_OTHERS_MODEL=deployment-model
```

The credential is read only from `LLM_API_KEY`. A required prompt limit from
one through 100 bounds requests. The generic adapter expects the provider to
implement OpenAI-compatible chat-completions request and response JSON.

## Qwen language-model example

The `llm-qwen` selector ports `robot-hat/examples/llm_qwen.py`. It keeps the
upstream `qwen-plus` model, instructions, welcome message, and 20-message
history. The default adapter uses the international DashScope
OpenAI-compatible Chat Completions endpoint.

For an explicitly approved live run:

```sh
export QWEN_API_KEY='<Qwen API key>'
./build-rpi/xExample llm-qwen 5
```

Override the endpoint at configuration time when the key belongs to another
supported deployment region:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON -DXWALK_EXAMPLE_QWEN_ENDPOINT=https://dashscope-intl.aliyuncs.com/compatible-mode/v1/chat/completions
```

The credential is read only from `QWEN_API_KEY`. A required prompt limit from
one through 100 bounds requests. The adapter returns completed responses rather
than streamed tokens.

## OpenAI camera language-model example

The `llm-openai-with-image` selector ports
`robot-hat/examples/llm_openai_with_image.py`. It keeps the upstream `gpt-4o`
model, instructions, welcome message, 20-message history, and 640×480 still
capture. Each prompt captures a fresh JPEG into the configured build-local
image path and submits it using OpenAI's image-message format.

For an explicitly approved live run:

```sh
export OPENAI_API_KEY='<OpenAI API key>'
./build-rpi/xExample llm-openai-with-image 5 csi rpicam-still /dev/video0
```

The credential is read only from `OPENAI_API_KEY`. A required prompt limit from
one through 100 bounds camera captures and API requests. The Linux adapter uses
OpenAI's chat-completions endpoint and returns completed responses rather than
streamed tokens.

## Streaming Vosk speech-to-text example

The `stt-vosk-stream` selector ports `robot-hat/examples/stt_vosk_stream.py`.
It retains the `Say something` prompt and partial/final event contract while
replacing the unbounded outer loop with one through 100 explicitly requested
sessions. Each capture is additionally bounded from one through 300,000
milliseconds.

For an explicitly approved Raspberry Pi microphone run:

```sh
./build-rpi/xExample stt-vosk-stream
```

The default YAML uses the architecture-selected Vosk 0.3.45 runtime and shared
small US English 0.15 model under the root-level `xWalkLibrary/common/models`. Set
`XWALK_VOSK_ARCHITECTURE`, `XWALK_VOSK_LIBRARY_PATH`, or
`XWALK_VOSK_MODEL_PATH` when configuring a deployment to override the detected
target or either asset. The existing HAL Vosk provider returns final results
after each bounded ALSA capture; its current API does not expose live partial
Vosk hypotheses to the Linux adapter.

## Non-streaming Vosk speech-to-text example

The `stt-vosk-without-stream` selector ports
`robot-hat/examples/stt_vosk_without_stream.py`. It retains the exact
`Say something` prompt, synchronous `stream=False` behavior, and unlabeled final
transcript output. The infinite source loop becomes one through 100 explicitly
requested sessions, each with a bounded capture timeout.

For an explicitly approved Raspberry Pi microphone run:

```sh
./build-rpi/xExample stt-vosk-without-stream
```

The arguments select sessions, capture milliseconds per session, ALSA capture
device, Vosk library, and model directory. Invoke the command only after
explicit microphone approval.

## Microsoft Edge text-to-speech example

The `tts-edge` selector ports `robot-hat/examples/tts_edge.py`. It preserves the
exact `en-US-AriaNeural` voice and the complete fixed Edge TTS introduction.
The Linux adapter executes `edge-playback` directly without a shell, passing
the voice and text as distinct process arguments.

For an explicitly approved live cloud and speaker run:

```sh
./build-rpi/xExample tts-edge edge-playback
```

The deployment requires the `edge-playback` executable from the `edge-tts`
package and its non-Windows playback dependency, `mpv`. Invoke the command only
with approval because it uses a remote service and produces audible output.

## Configured Espeak text-to-speech example

The `tts-espeak` selector ports `robot-hat/examples/tts_espeak.py`. It preserves
the exact amplitude `100`, speed `150`, word gap `1`, pitch `80`, and
`Hello world!` message. The Linux adapter invokes the selected Espeak executable
directly without a shell and passes each value as a distinct argument.

For an explicitly approved speaker run:

```sh
./build-rpi/xExample tts-espeak espeak-ng
```

The executable receives `-a 100 -s 150 -g 1 -p 80` followed by the fixed
message. Invoke the command only after explicit approval for audible output.

## OpenAI text-to-speech example

The `tts-openai` selector ports `robot-hat/examples/tts_openai.py`. It preserves
the exact `gpt-4o-mini-tts` model, `alloy` voice, three messages, and the two
optional speech instructions. The Linux adapter sends bounded authenticated
requests to `/v1/audio/speech`, captures MP3 responses, and invokes the selected
player without a shell. The API key is accepted only through
`OPENAI_API_KEY`; it is never placed in process arguments or diagnostics.

For an explicitly approved billable cloud and speaker run:

```sh
OPENAI_API_KEY="sk-..." ./build-rpi/xExample tts-openai mpv
```

Invoke the command only with approval because it contacts a billable remote
service and produces audible output.

## Pico2Wave text-to-speech example

The `tts-pico2wave` selector ports `robot-hat/examples/tts_pico2wave.py`. It
preserves the exact `en-US` language and `Hello world!` message. The Linux
adapter invokes Pico2Wave and the selected WAV player directly without a shell.
It uses a private temporary WAV file and removes that file after synthesis or
playback.

For an explicitly approved speaker run:

```sh
./build-rpi/xExample tts-pico2wave pico2wave aplay
```

Invoke the command only after explicit approval for audible output.

## Piper text-to-speech example

The `tts-piper` selector ports `robot-hat/examples/tts_piper.py`. It preserves
the exact `en_US-amy-low` voice model and the complete Piper introduction. The
Linux adapter invokes the Piper CLI and the selected WAV player directly
without a shell. It creates a private temporary WAV and removes it after
synthesis or playback.

For an explicitly approved speaker run:

```sh
./build-rpi/xExample tts-piper piper aplay
```

Piper must be able to resolve both `en_US-amy-low.onnx` and its matching
`.onnx.json` configuration from its configured voice-data directory. Invoke the
command only after explicit approval for audible output.

## Threaded Vosk wake-word example

The `stt-vosk-wake-word-thread` selector ports
`robot-hat/examples/stt_vosk_wake_word_thread.py`. It retains the exact
`hey robot` phrase, background recognition, three-second wake-state polling,
and original status messages. The C++ port replaces both unbounded loops with
explicit detection and poll limits and uses bounded recognition slices.

For an explicitly approved Raspberry Pi microphone run:

```sh
./build-rpi/xExample stt-vosk-wake-word-thread
```

The arguments select required detections, maximum three-second polls per
detection, recognition-slice milliseconds, ALSA device, Vosk library, and model
directory. The Linux adapter owns and joins its worker and cancels active ALSA
capture before shutdown. Invoke it only after explicit microphone approval.

## Synchronous Vosk wake-word example

The `stt-vosk-wake-word` selector ports
`robot-hat/examples/stt_vosk_wake_word.py`. It retains the exact `hey robot`
phrase, the `Wake me with :"Hey robot"` prompt, synchronous waiting, and the
successful detection message. A required attempt count and per-attempt timeout
bound the upstream blocking wait.

For an explicitly approved Raspberry Pi microphone run:

```sh
./build-rpi/xExample stt-vosk-wake-word
```

The arguments select maximum recognition attempts, capture milliseconds per
attempt, ALSA capture device, Vosk library, and model directory. Phrase matching
is case-insensitive. Invoke it only after explicit microphone approval.

## Structure

```text
xExample/
├── config/
│   ├── xHal_Rpi5CarExampleConfig.yml
│   └── xHal_Rpi5CarExampleConfig.h.in
├── core/
│   ├── include/
│   └── src/
├── hardware/
│   ├── include/
│   └── src/
├── CMakeLists.txt
├── main.cpp
└── README.md
```

Use `core/include` and `core/src` for reusable, host-testable example behavior.
Use `hardware/include` and `hardware/src` only for Linux and Raspberry Pi
composition. Source files remain in the layer that owns them.

`main.cpp` is the module's only process entry point and delegates to
`XWalkExampleRunner`. Each hardware example receives one explicit selector and
validated arguments in that runner; individual example sources must not define
another `main()`.

Examples are selected only through the `xExample` command line. The launcher
reads YAML arguments but does not read or write an XML selection file.

| Path | Responsibility |
| --- | --- |
| `core/include/xHal_Rpi5CarLedExample.h` | Injected LED example contract |
| `core/src/xHal_Rpi5CarLedExample.cpp` | Source-compatible LED actions and waits |
| `core/src/xHal_Rpi5CarLedExampleTest.cpp` | In-memory flow, validation, and cleanup verification |
| `core/include/xHal_Rpi5CarPinInputExample.h` | Injected bounded pin-input contract |
| `core/src/xHal_Rpi5CarPinInputExample.cpp` | D3 sampling, reporting, and 100-millisecond cadence |
| `core/src/xHal_Rpi5CarPinInputExampleTest.cpp` | In-memory pin and timing verification |
| `core/include/xHal_Rpi5CarUltrasonicExample.h` | Injected bounded ranging contract |
| `core/src/xHal_Rpi5CarUltrasonicExample.cpp` | Distance reporting and 200-millisecond cadence |
| `core/src/xHal_Rpi5CarUltrasonicExampleTest.cpp` | In-memory distance and timing verification |
| `core/include/xHal_Rpi5CarVoiceAssistantExample.h` | Exact bounded multimodal assistant contract |
| `core/src/xHal_Rpi5CarVoiceAssistantExample.cpp` | Keyboard, wake, image, model, and speech flow |
| `core/src/xHal_Rpi5CarVoiceAssistantExampleTest.cpp` | In-memory configuration and round verification |
| `core/include/xHal_Rpi5CarSttVoskStreamExample.h` | Injected bounded streaming-recognition contract |
| `core/src/xHal_Rpi5CarSttVoskStreamExample.cpp` | Prompt and partial/final result coordination |
| `core/src/xHal_Rpi5CarSttVoskStreamExampleTest.cpp` | In-memory streaming-result verification |
| `core/include/xHal_Rpi5CarSttVoskWithoutStreamExample.h` | Injected final-result contract |
| `core/src/xHal_Rpi5CarSttVoskWithoutStreamExample.cpp` | Bounded prompt and final-result flow |
| `core/src/xHal_Rpi5CarSttVoskWithoutStreamExampleTest.cpp` | In-memory final-result verification |
| `core/include/xHal_Rpi5CarTtsEdgeExample.h` | Exact Edge voice and speech request contract |
| `core/src/xHal_Rpi5CarTtsEdgeExample.cpp` | Single fixed Edge TTS request flow |
| `core/src/xHal_Rpi5CarTtsEdgeExampleTest.cpp` | In-memory voice and message verification |
| `core/include/xHal_Rpi5CarTtsEspeakExample.h` | Exact Espeak settings and request contract |
| `core/src/xHal_Rpi5CarTtsEspeakExample.cpp` | Single configured Espeak request flow |
| `core/src/xHal_Rpi5CarTtsEspeakExampleTest.cpp` | In-memory settings and message verification |
| `core/include/xHal_Rpi5CarTtsOpenAiExample.h` | Exact OpenAI TTS request contract |
| `core/src/xHal_Rpi5CarTtsOpenAiExample.cpp` | Three fixed OpenAI speech requests |
| `core/src/xHal_Rpi5CarTtsOpenAiExampleTest.cpp` | In-memory request and report verification |
| `core/include/xHal_Rpi5CarTtsPico2WaveExample.h` | Exact Pico2Wave language and request contract |
| `core/src/xHal_Rpi5CarTtsPico2WaveExample.cpp` | Single fixed Pico2Wave request flow |
| `core/src/xHal_Rpi5CarTtsPico2WaveExampleTest.cpp` | In-memory language and message verification |
| `core/include/xHal_Rpi5CarTtsPiperExample.h` | Exact Piper voice-model and request contract |
| `core/src/xHal_Rpi5CarTtsPiperExample.cpp` | Single fixed Piper request flow |
| `core/src/xHal_Rpi5CarTtsPiperExampleTest.cpp` | In-memory model and message verification |
| `core/include/xHal_Rpi5CarSttVoskWakeWordThreadExample.h` | Injected wake-worker contract |
| `core/src/xHal_Rpi5CarSttVoskWakeWordThreadExample.cpp` | Bounded wake polling and reporting |
| `core/src/xHal_Rpi5CarSttVoskWakeWordThreadExampleTest.cpp` | Host-safe lifecycle verification |
| `core/include/xHal_Rpi5CarSttVoskWakeWordExample.h` | Injected synchronous wake contract |
| `core/src/xHal_Rpi5CarSttVoskWakeWordExample.cpp` | Bounded recognition and phrase matching |
| `core/src/xHal_Rpi5CarSttVoskWakeWordExampleTest.cpp` | In-memory synchronous wake verification |
| `core/include/xHal_Rpi5CarServoExample.h` | Injected bounded servo-sweep contract |
| `core/src/xHal_Rpi5CarServoExample.cpp` | Exact angle ranges, delays, pauses, and reports |
| `core/src/xHal_Rpi5CarServoExampleTest.cpp` | In-memory angle and timing verification |
| `core/include/xHal_Rpi5CarDeepseekExample.h` | Injected interactive language-model contract |
| `core/src/xHal_Rpi5CarDeepseekExample.cpp` | Bounded instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarDeepseekExampleTest.cpp` | In-memory model and console verification |
| `core/include/xHal_Rpi5CarDoubaoImageExample.h` | Injected camera-chat contract |
| `core/src/xHal_Rpi5CarDoubaoImageExample.cpp` | Bounded capture and image-prompt flow |
| `core/src/xHal_Rpi5CarDoubaoImageExampleTest.cpp` | In-memory camera, model, and console verification |
| `core/include/xHal_Rpi5CarDoubaoExample.h` | Injected text-chat contract |
| `core/src/xHal_Rpi5CarDoubaoExample.cpp` | Bounded Doubao instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarDoubaoExampleTest.cpp` | In-memory text model and console verification |
| `core/include/xHal_Rpi5CarGeminiExample.h` | Injected Gemini chat contract |
| `core/src/xHal_Rpi5CarGeminiExample.cpp` | Bounded Gemini instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarGeminiExampleTest.cpp` | In-memory Gemini model and console verification |
| `core/include/xHal_Rpi5CarGrokExample.h` | Injected Grok chat contract |
| `core/src/xHal_Rpi5CarGrokExample.cpp` | Bounded Grok instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarGrokExampleTest.cpp` | In-memory Grok model and console verification |
| `core/include/xHal_Rpi5CarOllamaExample.h` | Injected Ollama text-chat contract |
| `core/src/xHal_Rpi5CarOllamaExample.cpp` | Bounded Ollama instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarOllamaExampleTest.cpp` | In-memory Ollama model and console verification |
| `core/include/xHal_Rpi5CarOllamaImageExample.h` | Injected Ollama camera-chat contract |
| `core/src/xHal_Rpi5CarOllamaImageExample.cpp` | Bounded capture and native image-prompt flow |
| `core/src/xHal_Rpi5CarOllamaImageExampleTest.cpp` | In-memory camera, model, and console verification |
| `core/include/xHal_Rpi5CarOpenAiImageExample.h` | Injected OpenAI camera-chat contract |
| `core/src/xHal_Rpi5CarOpenAiImageExample.cpp` | Bounded capture and OpenAI image-prompt flow |
| `core/src/xHal_Rpi5CarOpenAiImageExampleTest.cpp` | In-memory camera, model, and console verification |
| `core/include/xHal_Rpi5CarOpenAiExample.h` | Injected OpenAI text-chat contract |
| `core/src/xHal_Rpi5CarOpenAiExample.cpp` | Bounded OpenAI instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarOpenAiExampleTest.cpp` | In-memory OpenAI model and console verification |
| `core/include/xHal_Rpi5CarOthersExample.h` | Injected generic-provider chat contract |
| `core/src/xHal_Rpi5CarOthersExample.cpp` | Bounded generic instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarOthersExampleTest.cpp` | In-memory generic model and console verification |
| `core/include/xHal_Rpi5CarQwenExample.h` | Injected Qwen chat contract |
| `core/src/xHal_Rpi5CarQwenExample.cpp` | Bounded Qwen instructions, welcome, and prompt flow |
| `core/src/xHal_Rpi5CarQwenExampleTest.cpp` | In-memory Qwen model and console verification |
| `hardware/include/xHal_Rpi5CarLedExampleLinux.h` | Linux composition API |
| `hardware/src/xHal_Rpi5CarLedExampleLinux.cpp` | Physical GPIO, LED, timing, and output adapters |
| `hardware/include/xHal_Rpi5CarPinInputExampleLinux.h` | Physical D3 input-sampling API |
| `hardware/src/xHal_Rpi5CarPinInputExampleLinux.cpp` | GPIO22 pull-up, timing, and console adapters |
| `hardware/include/xHal_Rpi5CarUltrasonicExampleLinux.h` | Physical D2/D3 ranging API |
| `hardware/src/xHal_Rpi5CarUltrasonicExampleLinux.cpp` | GPIO27/GPIO22 sensor composition |
| `hardware/include/xHal_Rpi5CarVoiceAssistantExampleLinux.h` | Live multimodal provider API |
| `hardware/src/xHal_Rpi5CarVoiceAssistantExampleLinux.cpp` | Vosk, camera, OpenAI, and Piper composition |
| `hardware/include/xHal_Rpi5CarServoExampleLinux.h` | Physical channel-one servo-sweep API |
| `hardware/src/xHal_Rpi5CarServoExampleLinux.cpp` | I2C, PWM channel one, servo, and console adapters |
| `hardware/include/xHal_Rpi5CarSttVoskStreamExampleLinux.h` | ALSA and Vosk composition API |
| `hardware/src/xHal_Rpi5CarSttVoskStreamExampleLinux.cpp` | Microphone, final-result, and console adapters |
| `hardware/include/xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.h` | Non-streaming Vosk API |
| `hardware/src/xHal_Rpi5CarSttVoskWithoutStreamExampleLinux.cpp` | ALSA, Vosk, and console composition |
| `hardware/include/xHal_Rpi5CarTtsEdgeExampleLinux.h` | Edge playback process API |
| `hardware/src/xHal_Rpi5CarTtsEdgeExampleLinux.cpp` | Shell-free Edge playback execution |
| `hardware/include/xHal_Rpi5CarTtsEspeakExampleLinux.h` | Configured Espeak process API |
| `hardware/src/xHal_Rpi5CarTtsEspeakExampleLinux.cpp` | Shell-free configured playback execution |
| `hardware/include/xHal_Rpi5CarTtsOpenAiExampleLinux.h` | OpenAI speech and playback API |
| `hardware/src/xHal_Rpi5CarTtsOpenAiExampleLinux.cpp` | Bounded HTTPS and MP3 playback adapter |
| `hardware/include/xHal_Rpi5CarTtsPico2WaveExampleLinux.h` | Pico2Wave synthesis and playback API |
| `hardware/src/xHal_Rpi5CarTtsPico2WaveExampleLinux.cpp` | Shell-free temporary-WAV execution |
| `hardware/include/xHal_Rpi5CarTtsPiperExampleLinux.h` | Piper synthesis and playback API |
| `hardware/src/xHal_Rpi5CarTtsPiperExampleLinux.cpp` | Shell-free Piper WAV execution |
| `hardware/include/xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.h` | Threaded Vosk composition API |
| `hardware/src/xHal_Rpi5CarSttVoskWakeWordThreadExampleLinux.cpp` | Worker and wake detection adapter |
| `hardware/include/xHal_Rpi5CarSttVoskWakeWordExampleLinux.h` | Synchronous Vosk composition API |
| `hardware/src/xHal_Rpi5CarSttVoskWakeWordExampleLinux.cpp` | ALSA, recognition, and console adapter |
| `hardware/include/xHal_Rpi5CarDeepseekExampleLinux.h` | Live HTTP and console composition API |
| `hardware/src/xHal_Rpi5CarDeepseekExampleLinux.cpp` | DeepSeek HTTPS provider and console adapter |
| `hardware/include/xHal_Rpi5CarDoubaoImageExampleLinux.h` | Live camera-chat composition API |
| `hardware/src/xHal_Rpi5CarDoubaoImageExampleLinux.cpp` | Camera, Doubao HTTPS, and console adapters |
| `hardware/include/xHal_Rpi5CarDoubaoExampleLinux.h` | Live Doubao text-chat composition API |
| `hardware/src/xHal_Rpi5CarDoubaoExampleLinux.cpp` | Doubao HTTPS and console adapters |
| `hardware/include/xHal_Rpi5CarGeminiExampleLinux.h` | Live Gemini chat composition API |
| `hardware/src/xHal_Rpi5CarGeminiExampleLinux.cpp` | Gemini HTTPS and console adapters |
| `hardware/include/xHal_Rpi5CarGrokExampleLinux.h` | Live Grok chat composition API |
| `hardware/src/xHal_Rpi5CarGrokExampleLinux.cpp` | Grok HTTPS and console adapters |
| `hardware/include/xHal_Rpi5CarOllamaExampleLinux.h` | Live Ollama text-chat API |
| `hardware/src/xHal_Rpi5CarOllamaExampleLinux.cpp` | Native Ollama and console adapters |
| `hardware/include/xHal_Rpi5CarOllamaImageExampleLinux.h` | Live Ollama camera-chat API |
| `hardware/src/xHal_Rpi5CarOllamaImageExampleLinux.cpp` | Camera, Ollama, and console adapters |
| `hardware/include/xHal_Rpi5CarOpenAiImageExampleLinux.h` | Live OpenAI camera-chat API |
| `hardware/src/xHal_Rpi5CarOpenAiImageExampleLinux.cpp` | Camera, OpenAI HTTPS, and console adapters |
| `hardware/include/xHal_Rpi5CarOpenAiExampleLinux.h` | Live OpenAI text-chat API |
| `hardware/src/xHal_Rpi5CarOpenAiExampleLinux.cpp` | OpenAI HTTPS and console adapters |
| `hardware/include/xHal_Rpi5CarOthersExampleLinux.h` | Live generic-provider chat API |
| `hardware/src/xHal_Rpi5CarOthersExampleLinux.cpp` | Selected compatible HTTPS and console adapters |
| `hardware/include/xHal_Rpi5CarQwenExampleLinux.h` | Live Qwen chat API |
| `hardware/src/xHal_Rpi5CarQwenExampleLinux.cpp` | DashScope-compatible HTTPS and console adapters |
| `hardware/include/xHal_Rpi5CarExampleRunner.h` | Central example-selector contract |
| `hardware/src/xHal_Rpi5CarExampleRunner.cpp` | Argument, environment, and selector validation |
| `main.cpp` | The module's only process entry point |
| `config/xHal_Rpi5CarExampleConfig.yml` | Board, AI asset paths, and formal selector arguments |
| `config/xHal_Rpi5CarExampleConfig.h.in` | Build-local image path and Ollama endpoint template |

## Build

The executable is created only in the Raspberry Pi build profile and is
written directly to `${CMAKE_BINARY_DIR}`:

```sh
cmake -S . -B build-rpi -DXWALK_BUILD_RPI=ON
cmake --build build-rpi --target xExample --parallel
./build-rpi/xExample <example-name>
./build-rpi/xExample --config <yaml-file> <example-name>
```

An unknown selector or invalid argument count prints usage and returns status
2. Do not run physical examples before confirming the correct Raspberry Pi,
Robot HAT, wiring, power, and hardware safety.

The camera-chat image is generated at runtime under the configured build tree,
for example `build-rpi/xWalkHal/xWalkTest/xExample/config/llm-img.jpg`. CMake creates
the directory and generated path header; a camera capture creates or replaces
the JPEG. No source image is required, and the source `config` directory is not
modified by execution.
