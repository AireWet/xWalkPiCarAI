# xWalkGptCar

`xWalkGptCar` ports the upstream `gpt_examples` application onto the shared
`xWalkVoiceActiveCar` and `xWalkSelfDrive` coordinators. It preserves voice or
keyboard input, optional camera context, the JSON `actions` and `answer`
contract, the complete preset-action vocabulary, and the two sound effects.

The C++ composition uses the OpenAI-compatible chat endpoint with `gpt-4o`.
`OPENAI_API_KEY` exclusively supplies the credential. The upstream Assistants
identifier and generated speech files are not persisted by this adapter.

## Source layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarGptCar.h` | Profile, execution adapter, and provider defaults |
| `src/xAgent_Rpi5CarGptCar.cpp` | Prompt, JSON-mode configuration, and delegation |
| `test/src/xAgent_Rpi5CarGptCarTest.cpp` | Device-free profile and JSON parsing checks |
