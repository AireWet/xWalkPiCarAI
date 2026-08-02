# xWalkVoiceAssistant

`xWalkVoiceAssistant` provides synchronous, backend-neutral speech and model orchestration.

The application creates `XWalkSpeechToText`, `XWalkLanguageModel`, and
`XWalkTextToSpeech` in `main()` and passes references to the coordinator. The
assistant stores non-owning pointers and owns no microphone, model, network,
camera, speaker, trigger, thread, or operating-system resource.

## Directory layout

```text
xWalkVoiceAssistant/
├── hardware/test/src/xHal_Rpi5CarVoiceAssistantBackendsTest.cpp
├── include/
│   ├── xHal_Rpi5CarVoiceAssistant.h
│   └── xHal_Rpi5CarVoiceAssistantTypes.h
├── src/
│   ├── xHal_Rpi5CarVoiceAssistant.cpp
│   └── xHal_Rpi5CarVoiceAssistantLifecycle.cpp
├── test/
│   ├── hardware/src/xHal_Rpi5CarVoiceAssistantHardwareTest.cpp
│   └── src/xHal_Rpi5CarVoiceAssistantTest.cpp
├── CMakeLists.txt
└── README.md
```

## Completed backend composition

The application composition order is:

1. Create the shared `XWalkAudioAlsa` owner.
2. Create `XWalkSpeechToTextAlsa` with an application-selected recognizer, then
   create `XWalkSpeechToText` from its callbacks.
3. Create `XWalkLanguageModelOllama`, then create `XWalkLanguageModel` from its
   callbacks.
4. Create `XWalkTextToSpeechAlsa` with an application-selected synthesizer,
   then create `XWalkTextToSpeech` with caller-owned board control.
5. Create `XWalkVoiceAssistant` last so it is destroyed before every dependency.

The full-stack host test follows this order with injected ALSA operations, a
fake recognizer, fake Ollama transport, and fake synthesis. It verifies a
complete capture, recognition, model, synthesis, and playback round without
opening a device, contacting a model, or writing a file.

One round performs these operations:

1. Check speech-backend readiness and listen for bounded input.
2. Preserve silence as an empty result without prompting or speaking.
3. Submit non-empty recognized or caller-supplied text to the language model.
4. Optionally parse the final response and speak a non-empty parsed response.
5. Notify synchronous lifecycle callbacks without retaining their text views.

Wake-word detection, keyboard polling, image capture, continuous loops,
streaming tokens, and scheduling remain application or backend responsibilities.
Calls require external serialization and may block in injected backends.

## Host build and test

```sh
cmake -S xWalkVoiceAssistant -B build-va-host -DXWALK_VOICE_ASSISTANT_BUILD_HOST_TESTS=ON
cmake --build build-va-host --parallel
ctest --test-dir build-va-host --output-on-failure
```

The suite includes both neutral coordinator coverage and a completed-backend
composition test. Every backend operation is deterministic and in memory.

## Target compile check

```sh
cmake -S xWalkVoiceAssistant -B build-va-rpi -DXWALK_VOICE_ASSISTANT_BUILD_HARDWARE_TESTS=ON
cmake --build build-va-rpi --parallel
ctest --test-dir build-va-rpi -N -L hardware
```

The last command only lists hardware-labelled tests. It does not execute them.

The hardware-labelled composition executable requires an explicit ALSA capture
device, PCM device, mixer device, mixer element, Ollama endpoint, model, approved
prompt, and raw 16 kHz mono signed-16 little-endian response fixture. It captures
100 milliseconds, validates that microphone PCM was received, maps it to the
approved prompt, performs one non-streaming Ollama request, and plays the fixture
for the non-empty response at 15 percent volume. It prints no microphone, prompt,
model-response, fixture, or credential content.

This smoke executable deliberately uses non-physical board-control seams and
does not claim GPIO or I2C. Confirm speaker power separately through approved
deployment setup before running it:

```sh
cd build-va-rpi
./xWalkVoiceAssistantHardwareTest <capture> <pcm> <mixer> <element> <endpoint> <model> <prompt> <fixture>
```

Run it only after approving microphone privacy, network policy, endpoint, model,
prompt, speaker power, devices, acoustic volume, and fixture content. Real
recognition and synthesis provider operations remain deployment-selected as
defined by `xWalkGPT`; the smoke test validates their composition boundaries.
