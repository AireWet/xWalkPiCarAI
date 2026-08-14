# xWalkVoiceAssistant standalone simulation

The device-free executable composes the coordinator with in-memory GPIO, I2C, recognition, language-model, and
speech-output callbacks. It opens no device, model, process, filesystem input, or network connection and produces
no audio.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkVoiceAssistantSimulation --parallel
./build-host/xWalkVoiceAssistantSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkVoiceAssistantSimulation.log`.
