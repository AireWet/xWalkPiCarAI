# xWalk HAL Layer 1 group

The Layer 1 group contains higher-level robot services and features:

- `xWalkMusic`
- `xWalkSpeaker`
- `xWalkBoardControl`
- `xWalkRobot`
- `xWalkGPT`
- `xWalkVoiceAssistant`

## Group interaction test

`test` builds `xWalkLayer1GroupTest`. The suite complements the individual
module tests by checking board readiness and configured Robot initialization,
silent Music-to-Speaker flow, and the complete speech-to-model-to-speech Voice
Assistant workflow. It also verifies that critical failures prevent later
output and that shutdown restores a safe state.

Audio decoding, ALSA, speech recognition, speech synthesis, language models,
I2C, and GPIO use deterministic callbacks. A condition-variable handshake lets
the fake speaker confirm worker output without timing sleeps or real playback.

From the repository root:

```bash
cmake -S . -B build-host/group-tests -DBUILD_TESTING=ON -DXWALK_BUILD_RPI=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host/group-tests --target xWalkLayer1GroupTest --parallel
build-host/group-tests/xWalkHal/layer1/test/xWalkLayer1GroupTest
ctest --test-dir build-host/group-tests -L layer1-group --output-on-failure
```
