# xWalk HAL interface group

The interface group contains low-level platform interfaces and common services:

- `xWalkI2c`
- `xWalkSpi`
- `xWalkGpio`
- `xWalkAudio`
- `xWalkConfig`
- `xWalkUtils`
- `xWalkLanguageModel`

`xWalk-rpi5/xWalkLibrary/common` remains repository-wide build infrastructure outside this
group because HAL, Agent, Controller, IW, and Trace components all consume it.

## Group interaction test

`test` builds `xWalkInterfaceGroupTest`. The suite complements the individual
module tests by checking configuration-driven I2C, SPI, and GPIO initialization,
utility validation, injected ALSA negotiation, and language-model request and
failure flow.

All operating-system, audio, model, and hardware operations use deterministic
in-memory callbacks. The test does not access physical devices or services.

From the repository root:

```bash
cmake -S . -B build-host/group-tests -DBUILD_TESTING=ON -DXWALK_BUILD_RPI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host/group-tests --target xWalkInterfaceGroupTest --parallel
build-host/group-tests/xWalkHal/interface/test/xWalkInterfaceGroupTest
ctest --test-dir build-host/group-tests -L interface-group --output-on-failure
```
