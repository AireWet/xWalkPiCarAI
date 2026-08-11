# xWalkGPT standalone simulation

The device-free executable exercises speech recognition, file transcription, cancellation, speaker priming, and
speech output through in-memory callbacks. It opens no microphone, speaker, ALSA, Vosk, or synthesis provider.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkGptSimulation --parallel
./build-host/xWalkGptSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkGptSimulation.log`.
