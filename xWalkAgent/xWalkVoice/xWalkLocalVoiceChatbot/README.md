# xWalkLocalVoiceChatbot

`xWalkLocalVoiceChatbot` is a hardware-independent foreground Agent coordinator.
It repeatedly listens through a caller-owned `XWalkVoiceAssistant`,
prompts its language model, removes hidden-thinking sections, speaks one clean
response, handles silence, and performs a deterministic goodbye sequence. It
preserves the visible messages from `example/19.local_voice_chatbot.py`.

The module owns no microphone, recognizer, language model, network transport,
speech synthesizer, ALSA stream, Robot HAT GPIO, thread, or signal handler.
Those resources remain selected by the process composition layer.
The Raspberry Pi composition uses Vosk with the configured English model,
Piper with `en_US-amy-low`, and local Ollama with `llama3.2:3b`. It retains 20
messages, matching the source example. Provider paths and model names remain
deployment configurable without being owned by this coordinator.

## Host verification

```bash
cmake -S xWalkAgent/xWalkVoice/xWalkLocalVoiceChatbot -B xWalkAgent/xWalkVoice/xWalkLocalVoiceChatbot/build-host -DXWALK_LOCAL_VOICE_CHATBOT_BUILD_HOST_TESTS=ON
cmake --build xWalkAgent/xWalkVoice/xWalkLocalVoiceChatbot/build-host --parallel
ctest --test-dir xWalkAgent/xWalkVoice/xWalkLocalVoiceChatbot/build-host --output-on-failure
```
