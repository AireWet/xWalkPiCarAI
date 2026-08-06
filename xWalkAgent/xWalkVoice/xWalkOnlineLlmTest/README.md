# xWalkOnlineLlmTest

`xWalkOnlineLlmTest` ports upstream `example/18.online_llm_test.py` through a
caller-owned language-model service. It applies the source instructions,
welcome text, and 20-message history limit, then submits typed text-only prompts
until process cancellation.

The Raspberry Pi composition defaults to OpenAI's chat-completions endpoint and
model `gpt-4o`. It reads the credential only from `OPENAI_API_KEY`; the key is
never accepted in command arguments, committed configuration, or diagnostics.
The provider-neutral HAL returns a complete response, so the CLI emits one final
response line instead of Python stream fragments.
