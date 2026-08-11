# xWalkSpeaker standalone simulation

The executable composes the public Speaker API over an in-memory decoder and stream backend. It creates one
temporary fixture below the build tree, does not open ALSA, and cannot produce physical sound.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkSpeakerSimulation --parallel
./build-host/xWalkSpeakerSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkSpeakerSimulation.log`.
