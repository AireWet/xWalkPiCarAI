# xWalkLanguageModel standalone simulation

The executable exercises `XWalkLanguageModel` through an in-memory callback backend. It performs no provider
selection, credential access, process launch, or network request. Prompt, response, endpoint, model, image, and
credential content are not logged.

## Build and run

Run these commands from this `simulation` directory:

```bash
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target xWalkLanguageModelSimulation --parallel
./build-host/xWalkLanguageModelSimulation --trace RPI.enable
```

Trace changes persist in generated XML. Enabled messages appear in the terminal and
`build-host/log/xWalkLanguageModelSimulation.log`.
