# xWalkAudio standalone simulation

The executable runs the public `XWalkAudioAlsa` API through its injected operation seam. The `stub` backend mirrors
PCM opens, writes, closes, and mixer volume in memory without opening an audio device or changing mixer state.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DXWALK_AUDIO_SIMULATION_BACKEND=stub -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkAudioSimulation --parallel
./build-host/xWalkAudioSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkAudioSimulation.log`.
