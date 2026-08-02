# xWalkLanguageModel

C++17 embedded-oriented provider-neutral language-model coordination.

The public interface consolidates general, Deepseek, Grok, Doubao, Gemini, Qwen,
OpenAI, and Ollama conversation behavior behind
one injected backend instead of embedding provider SDKs in the firmware layer.

## Directory layout

```text
xWalkLanguageModel/
├── include/
│   ├── xHal_Rpi5CarLanguageModel.h
│   └── xHal_Rpi5CarLanguageModelTypes.h
├── src/
│   ├── xHal_Rpi5CarLanguageModel.cpp
│   └── xHal_Rpi5CarLanguageModelLifecycle.cpp
├── hardware/
│   ├── include/
│   │   ├── xHal_Rpi5CarLanguageModelOllama.h
│   │   └── xHal_Rpi5CarLanguageModelOllamaTypes.h
│   ├── src/
│   │   ├── xHal_Rpi5CarLanguageModelOllamaCallbacks.cpp
│   │   ├── xHal_Rpi5CarLanguageModelOllamaJson.cpp
│   │   ├── xHal_Rpi5CarLanguageModelOllamaLifecycle.cpp
│   │   └── xHal_Rpi5CarLanguageModelOllamaSystem.cpp
│   └── test/src/xHal_Rpi5CarLanguageModelOllamaTest.cpp
├── test/
│   ├── hardware/src/xHal_Rpi5CarLanguageModelHardwareTest.cpp
│   └── src/xHal_Rpi5CarLanguageModelTest.cpp
├── CMakeLists.txt
└── README.md
```

## Composition and ownership

`XWalkLanguageModelHttp` is the compatibility name for the concrete bounded HTTP
backend. The legacy `XWalkLanguageModelOllama` class name remains source and ABI
compatible. Create the backend before the coordinator and pass it as callback
context:

```cpp
XWalkLanguageModelHttp backend("http://127.0.0.1:11434/api/chat", "gemma3");
XWalkLanguageModel languageModel(&backend, backend.callbacks());
languageModel.setInstructions("Answer briefly.");
const string response = languageModel.prompt("Hello");
```

The provider owns its dialect, endpoint, model name, optional API key,
instructions, welcome text, encoded images, and bounded conversation history.
Real requests use libcurl with either the non-streaming Ollama `/api/chat`
contract or an authenticated OpenAI-compatible `/chat/completions` contract.
Calls require external serialization. An injected HTTP operation supports
deterministic tests and alternate transports.

The OpenAI-compatible dialect is used for ChatGPT, Gemini's compatibility API,
Claude's compatibility API, and explicitly compatible private services. The
deployment supplies the complete endpoint and model name; the firmware does not
guess either value.

## Ported behavior

- `setInstructions()` replaces system instructions.
- `setWelcome()` replaces conversation welcome text.
- `setMaximumMessages()` configures a non-zero retained-message limit.
- The default retained-message limit is 20 messages.
- `addMessage()` supports system, user, and assistant roles.
- `addMessage()` and `prompt()` accept an optional image path.
- `prompt()` returns one owned final response and preserves empty responses.
- Empty instructions, welcome text, message content, and prompt text are preserved.

The implementation enforces these provider limits:

- 200 retained messages, with 20 by default.
- 256 KiB for each model, endpoint, instruction, welcome, message, or response content value.
- 4 MiB for one raw image, encoded immediately as base64.
- 8 MiB for one serialized JSON request and 1 MiB for one HTTP response.
- 1 through 300,000 milliseconds per request, with 120,000 by default.

The native Ollama dialect sends no authorization header and is intended for an
appropriately protected local or deployment-controlled endpoint. The compatible
dialect requires HTTPS and sends the configured key only as an `Authorization:
Bearer` header. The key is bounded, rejected when it contains control-line
characters, and never copied into request JSON or conversation history. TLS,
credential rotation, and network access policy remain deployment
responsibilities. Request, response, image, and credential content must never be
written to normal diagnostics.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkHal/xWalkLanguageModel -B build-lm-host -DXWALK_LANGUAGE_MODEL_BUILD_HOST_TESTS=ON
cmake --build build-lm-host --parallel
ctest --test-dir build-lm-host --output-on-failure
```

The provider host test uses fake HTTP transport and a test-owned three-byte image.
It performs no model, credential, process, or network operation.

## Target compile and test discovery

```bash
cmake -S xWalkHal/xWalkLanguageModel -B build-lm-rpi -DXWALK_LANGUAGE_MODEL_BUILD_HARDWARE_TESTS=ON
cmake --build build-lm-rpi --parallel
ctest --test-dir build-lm-rpi -N -L hardware
```

The hardware-labelled executable requires an explicit endpoint, model, and
prompt, then performs exactly one bounded request without printing its content.
CTest registers no arguments, so normal verification only lists it. Run it
manually only after approving the endpoint, model, prompt, and network policy:

```bash
./build-lm-rpi/xWalkLanguageModelHardwareTest <endpoint/api/chat> <model> <prompt>
```

The provider follows Ollama's documented non-streaming chat request and final
`message.content` response shape.
