# xWalkGPT

C++17 embedded-oriented speech interface containing the Robot HAT
speech-to-text and text-to-speech coordinators.

The module combines the former `xWalkSpeechToText` and `xWalkTextToSpeech`
libraries while retaining one focused class per header and source file:

Core behavior, simulation, and host tests use unique trace IDs `RPI.357` through
`RPI.369`. The repository validator rejects repeated numeric IDs within `RPI`.

- `XWalkSpeechToText` provides readiness, bounded microphone recognition,
  audio-file transcription, and cancellation through an injected backend.
- `XWalkTextToSpeech` activates Robot HAT speaker output and forwards speech
  text through an injected synthesis backend.
- `XWalkSpeechRecognizerVosk` loads the Vosk C API and one offline model at
  runtime without requiring vendor headers during compilation.
- `XWalkTextToSpeechEspeak` runs Espeak without a shell and converts its WAV
  output into PCM for the shared ALSA playback adapter.
- `XWalkTextToSpeechPiper` runs Piper and WAV playback without a shell using a
  deployment-selected model.
- `XWalkTextToSpeechPico2Wave` runs language-selected Pico2Wave synthesis and
  WAV playback without a shell.

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
│   │   ├── xHal_Rpi5CarTextToSpeechPico2Wave.h
│   │   ├── xHal_Rpi5CarTextToSpeechPiper.h
│   │   ├── xHal_Rpi5CarTextToSpeechAlsa.h
│   │   └── xHal_Rpi5CarTextToSpeechAlsaTypes.h
│   ├── src/
│   │   ├── xHal_Rpi5CarSpeechRecognizerVosk.cpp
│   │   ├── xHal_Rpi5CarVoskRecognizerGuard.cpp
│   │   ├── xHal_Rpi5CarSpeechToTextAlsa.cpp
│   │   ├── xHal_Rpi5CarSpeechToTextAlsaSystem.cpp
│   │   ├── xHal_Rpi5CarTextToSpeechAlsa.cpp
│   │   ├── xHal_Rpi5CarTextToSpeechEspeak.cpp
│   │   ├── xHal_Rpi5CarTextToSpeechPico2Wave.cpp
│   │   └── xHal_Rpi5CarTextToSpeechPiper.cpp
│   └── test/src/
├── simulation/
│   ├── config/xHal_Rpi5CarGptTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/xHal_Rpi5CarGptTestSupport.h
│   ├── hardware/src/
│   └── src/
│       ├── xHal_Rpi5CarGptTestSupport.cpp
│       ├── xHal_Rpi5CarSpeechToTextTest.cpp
│       └── xHal_Rpi5CarTextToSpeechTest.cpp
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
runtimes under `../../../xWalkLibrary/{aarch64,x86_64}` and one shared small US English 0.15 model under
`../../../xWalkLibrary/common/models`;
deployments may override both paths. The Espeak provider requires `espeak-ng`
executable, and the Pico2Wave provider requires `pico2wave` from
`libttspico-utils` plus a WAV playback executable. None of these providers uses
a shell. A scope-bound Vosk recognizer guard
releases each recognizer during normal return and stack cleanup without
exception interception.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalk-rpi5/xWalkHal/layer1/xWalkGPT -B xWalk-rpi5/xWalkHal/layer1/xWalkGPT/build-host -DXWALK_GPT_BUILD_HOST_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/layer1/xWalkGPT/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/layer1/xWalkGPT/build-host --output-on-failure
```

The host tests use deterministic in-memory backends. They perform no microphone,
model, process, network, filesystem, or physical speaker operation.

Reusable core callback state and functions live in
`xwalk::hal::test::gpt`, outside the scenario test sources.

## Trace persistence

The host simulation and core tests use a generated XML catalogue. For example,
`--trace RPI.363.enable` saves the enabled state, and a later no-flag run loads
it automatically. Enabled records are written to the terminal and log. Speech
text, transcripts, captured PCM, and credentials are never included.

## Safe host simulation

The simulation exercises STT and TTS coordination through an in-memory backend.
It opens no microphone, speaker, ALSA device, model, provider, process, or network.

```bash
cmake -S xWalk-rpi5/xWalkHal/layer1/xWalkGPT/simulation -B build/xWalkGPT-simulation
cmake --build build/xWalkGPT-simulation --parallel
build/xWalkGPT-simulation/xWalkGptSimulation
```

## Target compile and test discovery

```bash
cmake -S xWalk-rpi5/xWalkHal/layer1/xWalkGPT -B xWalk-rpi5/xWalkHal/layer1/xWalkGPT/build-rpi -DXWALK_GPT_BUILD_HARDWARE_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/layer1/xWalkGPT/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/layer1/xWalkGPT/build-rpi -N -L hardware
```

The speech-to-text hardware executable captures only 100 milliseconds and
requires an explicit ALSA capture device as its sole argument. CTest deliberately
registers it without a device, so normal verification must list it without
executing it. It sends PCM to a deterministic recognition sink and does not
enable speakers or actuators. On a confirmed safe Raspberry Pi, run it manually:

```bash
./xWalkHal/layer1/xWalkGPT/build-rpi/xWalkSpeechToTextHardwareTest <alsa-capture-device>
```

Use `arecord -L` to discover device names. Do not include captured PCM,
transcripts, credentials, or provider requests in normal diagnostics.

The text-to-speech hardware executable also requires explicit selection. Supply
the PCM device, mixer device, mixer element, and a deployment-owned raw 16 kHz
mono signed-16 little-endian fixture containing the fixed phrase documented by
the executable. It applies 15 percent mixer volume and performs bounded playback:

```bash
./xWalkHal/layer1/xWalkGPT/build-rpi/xWalkTextToSpeechHardwareTest <pcm> <mixer> <element> <fixture.raw>
```

Use `aplay -L` and `amixer` to verify names before execution. Confirm the correct
Robot HAT, speaker-power state, fixture content, and safe acoustic environment.
CTest registers both physical tests without arguments, so listing remains safe;
do not execute either test through ordinary verification.
