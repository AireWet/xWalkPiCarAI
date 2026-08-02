# xWalkLocalVoiceChatbot

`xWalkLocalVoiceChatbot` is a hardware-independent foreground Agent coordinator.
It repeatedly listens through a caller-owned `XWalkVoiceAssistant`,
prompts its language model, removes hidden-thinking sections, speaks one clean
response, handles silence, and performs a deterministic goodbye sequence.

The module owns no microphone, recognizer, language model, network transport,
speech synthesizer, ALSA stream, Robot HAT GPIO, thread, or signal handler.
Those resources remain selected by the process composition layer.
The Raspberry Pi composition reads the language-model provider, endpoint, model,
and optional API key from the shared deployment configuration. This coordinator
therefore works with native Ollama and supported OpenAI-compatible services
without importing a vendor SDK or reading credentials itself.

## Host verification

```bash
cmake -S xWalkAgent/xWalkLocalVoiceChatbot -B xWalkAgent/xWalkLocalVoiceChatbot/build-host -DXWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS=ON
cmake --build xWalkAgent/xWalkLocalVoiceChatbot/build-host --parallel
ctest --test-dir xWalkAgent/xWalkLocalVoiceChatbot/build-host --output-on-failure
```
