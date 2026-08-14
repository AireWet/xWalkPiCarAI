# xWalkMusic standalone simulation

The executable composes the public Music API over an in-memory callback backend. It does not open ALSA, read audio
files, or produce physical sound.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkMusicSimulation --parallel
./build-host/xWalkMusicSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkMusicSimulation.log`.
