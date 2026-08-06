# xWalkTextVisionTalk

`xWalkTextVisionTalk` ports upstream `example/17.text_vision_talk.py` through
caller-owned camera-capture and language-model services. It applies the source
instructions, welcome text, and 20-message conversation limit, waits two
seconds for camera warm-up, and captures a new 1280-by-720 image for every typed
prompt until `exit` or `quit`.

The Raspberry Pi composition defaults to Ollama at
`http://127.0.0.1:11434/api/chat`, model `llava:7b`, and image path
`/tmp/llm-img.jpg`. The provider-neutral HAL returns a complete response, so the
CLI emits that final response as one line instead of Python word fragments.
