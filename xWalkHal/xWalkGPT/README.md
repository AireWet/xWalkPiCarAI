# xWalkGPT

C++17 embedded-oriented speech interface containing the Robot HAT
speech-to-text and text-to-speech coordinators.

The module combines the former `xWalkSpeechToText` and `xWalkTextToSpeech`
libraries while retaining one focused class per header and source file:

- `XWalkSpeechToText` provides readiness, bounded microphone recognition,
  audio-file transcription, and cancellation through an injected backend.
- `XWalkTextToSpeech` activates Robot HAT speaker output and forwards speech
  text through an injected synthesis backend.
- `XWalkSpeechRecognizerVosk` loads the Vosk C API and one offline model at
  runtime without requiring vendor headers during compilation.
- `XWalkTextToSpeechEspeak` runs Espeak without a shell and converts its WAV
  output into PCM for the shared ALSA playback adapter.

## Directory layout

```text
xWalkGPT/
├── include/
│   ├── xHal_Rpi5CarSpeechToText.h
│   ├── xHal_Rpi5CarSpeechToTextTypes.h
│   ├── xHal_Rpi5CarTextToSpeech.h
│   └── xHal_Rpi5CarTextToSpeechTypes.h
├── src/
│   ├── xHal_Rpi5CarSpeechToText.cpp
│   ├── xHal_Rpi5CarSpeechToTextLifecycle.cpp
│   ├── xHal_Rpi5CarTextToSpeech.cpp
│   └── xHal_Rpi5CarTextToSpeechLifecycle.cpp
├── hardware/
│   ├── include/
│   │   ├── xHal_Rpi5CarSpeechRecognizerVosk.h
│   │   ├── xHal_Rpi5CarSpeechRecognizerVoskTypes.h
│   │   ├── xHal_Rpi5CarVoskRecognizerGuard.h
│   │   ├── xHal_Rpi5CarSpeechToTextAlsa.h
│   │   ├── xHal_Rpi5CarSpeechToTextAlsaTypes.h
│   │   ├── xHal_Rpi5CarTextToSpeechEspeak.h
│   │   ├── xHal_Rpi5CarTextToSpeechAlsa.h
│   │   └── xHal_Rpi5CarTextToSpeechAlsaTypes.h
│   ├── src/
│   │   ├── xHal_Rpi5CarSpeechRecognizerVosk.cpp
│   │   ├── xHal_Rpi5CarVoskRecognizerGuard.cpp
│   │   ├── xHal_Rpi5CarSpeechToTextAlsa.cpp
│   │   ├── xHal_Rpi5CarSpeechToTextAlsaSystem.cpp
│   │   ├── xHal_Rpi5CarTextToSpeechAlsa.cpp
│   │   └── xHal_Rpi5CarTextToSpeechEspeak.cpp
│   └── test/src/
├── test/
│   ├── hardware/src/
│   └── src/
├── CMakeLists.txt
└── README.md
```

## Composition and ownership

Create board control and speech backends in `main()`. Pass board control by
reference and backend state through documented non-owning context pointers.
The application must keep every referenced object alive for the complete
coordinator lifetime.

`XWalkSpeechToTextAlsa` owns one ALSA capture handle for each bounded listen
request. It captures 16 kHz mono signed-16 PCM in reads of no more than 1,024
frames, closes the handle before recognition, and forwards PCM or an audio-file
path to one injected recognizer. The recognizer owns its model, process or HTTP
transport, credentials, language policy, and provider-specific conversion.

The recognition context remains non-owning and must outlive the adapter. The
application selects one local or remote recognizer and supplies all recognition
operations.

`XWalkTextToSpeechAlsa` observes one caller-owned `XWalkAudioAlsa`. It forwards
text to one injected local or remote synthesis provider, validates at most 16
MiB of interleaved signed-16 PCM, and writes no more than 1,024 frames per shared
ALSA operation. Empty provider PCM completes without opening an audio stream.
The provider owns its model, voice, credentials, process or HTTP transport, and
decoding policy. Construct the shared audio owner before the adapter and destroy
the adapter first.

The offline Raspberry Pi graphs are:

```text
Vosk model → XWalkSpeechRecognizerVosk → XWalkSpeechToTextAlsa
           → XWalkSpeechToText → XWalkVoiceControlledCar

Espeak → XWalkTextToSpeechEspeak → XWalkTextToSpeechAlsa
       → XWalkTextToSpeech → XWalkVoicePromptCar
```

The Vosk provider resolves a caller-supplied shared-library path at runtime and
therefore requires a target-compatible Vosk runtime plus a model directory.
The aggregate repository provides separate ARM64 and x86-64 Vosk 0.3.45
runtimes and one shared small US English 0.15 model under `../../xWalkLibrary/vosk`;
deployments may override both paths. The Espeak provider requires `espeak-ng`
executable. Neither provider uses a shell. A scope-bound Vosk recognizer guard
releases each recognizer during normal return and stack cleanup without
exception interception.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkHal/xWalkGPT -B xWalkHal/xWalkGPT/build-host -DXWALK_GPT_BUILD_HOST_TESTS=ON
cmake --build xWalkHal/xWalkGPT/build-host --parallel
ctest --test-dir xWalkHal/xWalkGPT/build-host --output-on-failure
```

The host tests use deterministic in-memory backends. They perform no microphone,
model, process, network, filesystem, or physical speaker operation.

## Target compile and test discovery

```bash
cmake -S xWalkHal/xWalkGPT -B xWalkHal/xWalkGPT/build-rpi -DXWALK_GPT_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkHal/xWalkGPT/build-rpi --parallel
ctest --test-dir xWalkHal/xWalkGPT/build-rpi -N -L hardware
```

The speech-to-text hardware executable captures only 100 milliseconds and
requires an explicit ALSA capture device as its sole argument. CTest deliberately
registers it without a device, so normal verification must list it without
executing it. It sends PCM to a deterministic recognition sink and does not
enable speakers or actuators. On a confirmed safe Raspberry Pi, run it manually:

```bash
./xWalkHal/xWalkGPT/build-rpi/xWalkSpeechToTextHardwareTest <alsa-capture-device>
```

Use `arecord -L` to discover device names. Do not include captured PCM,
transcripts, credentials, or provider requests in normal diagnostics.

The text-to-speech hardware executable also requires explicit selection. Supply
the PCM device, mixer device, mixer element, and a deployment-owned raw 16 kHz
mono signed-16 little-endian fixture containing the fixed phrase documented by
the executable. It applies 15 percent mixer volume and performs bounded playback:

```bash
./xWalkHal/xWalkGPT/build-rpi/xWalkTextToSpeechHardwareTest <pcm> <mixer> <element> <fixture.raw>
```

Use `aplay -L` and `amixer` to verify names before execution. Confirm the correct
Robot HAT, speaker-power state, fixture content, and safe acoustic environment.
CTest registers both physical tests without arguments, so listing remains safe;
do not execute either test through ordinary verification.
