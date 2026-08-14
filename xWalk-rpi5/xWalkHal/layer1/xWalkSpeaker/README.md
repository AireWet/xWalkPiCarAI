# xWalkSpeaker

C++17 bounded asynchronous speaker playback for the xWalk Firmware HAL.

The submodule enables and disables speaker output, validates supported audio
files, starts bounded background playback tasks, reports progress, and supports
pause, resume, stop, and automatic cleanup.

The optional `XWalkSpeakerAlsa` adapter connects every callback to the shared
`XWalkAudioAlsa` owner. It provides bounded 16-bit PCM RIFF/WAVE decoding and an
injected decoder seam for FLAC, OGG, MP3, M4A, AAC, or WMA codec libraries.

## Directory layout

```text
xWalkSpeaker/
├── include/
│   ├── xHal_Rpi5CarSpeaker.h
│   └── xHal_Rpi5CarSpeakerTypes.h
├── hardware/
│   ├── include/
│   │   ├── xHal_Rpi5CarSpeakerAlsa.h
│   │   └── xHal_Rpi5CarSpeakerAlsaTypes.h
│   ├── src/
│   │   ├── xHal_Rpi5CarSpeakerAlsaCallbacks.cpp
│   │   ├── xHal_Rpi5CarSpeakerAlsaDecode.cpp
│   │   └── xHal_Rpi5CarSpeakerAlsaLifecycle.cpp
│   └── test/src/
│       ├── xHal_Rpi5CarSpeakerAlsaTest.cpp
│       └── xHal_Rpi5CarSpeakerAlsaHardwareTest.cpp
├── src/
│   ├── xHal_Rpi5CarSpeakerLifecycle.cpp
│   ├── xHal_Rpi5CarSpeakerPlayback.cpp
│   ├── xHal_Rpi5CarSpeakerTasks.cpp
│   └── xHal_Rpi5CarSpeakerWorker.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarSpeakerTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/xHal_Rpi5CarSpeakerTestSupport.h
│   └── src/
│       ├── xHal_Rpi5CarSpeakerTest.cpp
│       └── xHal_Rpi5CarSpeakerTestSupport.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarSpeaker.h` | Public lifecycle, playback, progress, and task-control contract |
| `xHal_Rpi5CarSpeakerTypes.h` | Audio, progress, callback, and bounded task types |
| `xHal_Rpi5CarSpeakerLifecycle.cpp` | Backend validation and speaker-output lifecycle |
| `xHal_Rpi5CarSpeakerPlayback.cpp` | File validation, format selection, decoding, and task creation |
| `xHal_Rpi5CarSpeakerTasks.cpp` | Progress, pause, resume, stop, listing, joining, and cleanup |
| `xHal_Rpi5CarSpeakerWorker.cpp` | Bounded frame writes and stream cleanup |
| `simulation/` | Silent in-memory decoder/stream backend and persistent trace executable |
| `xHal_Rpi5CarSpeakerTestSupport.*` | Named decoder and stream callbacks shared by host tests |
| `xHal_Rpi5CarSpeakerTest.cpp` | In-memory output, stream, task, and validation coverage |
| `xHal_Rpi5CarSpeakerAlsa.h` | Shared-audio dependency, callbacks, limits, and lifetime contract |
| `xHal_Rpi5CarSpeakerAlsaTypes.h` | Optional bounded decoder operation seam |
| `xHal_Rpi5CarSpeakerAlsaCallbacks.cpp` | Float32 conversion, streams, volume, and task identifiers |
| `xHal_Rpi5CarSpeakerAlsaDecode.cpp` | Bounded PCM RIFF/WAVE decoding and sample validation |
| `xHal_Rpi5CarSpeakerAlsaLifecycle.cpp` | Dependency binding and callback publication |
| `xHal_Rpi5CarSpeakerAlsaTest.cpp` | Device-free decoder, conversion, cancellation, and failure tests |
| `xHal_Rpi5CarSpeakerAlsaHardwareTest.cpp` | Opt-in short silent WAVE playback test |

## Composition

The application creates its platform decoder and audio-output backend, then
passes a non-owning context and complete callback table to `XWalkSpeaker`.

```cpp
AudioBackend backend;
const XWalkHal::XWalkSpeakerCallbacks callbacks = makeSpeakerCallbacks();
XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
const XWalkHal::string taskId = speaker.play("notification.wav");
```

The backend context must outlive the controller and every playback worker. The
backend owns each opaque stream returned by `openStream`; `XWalkSpeaker` closes
the stream through `closeStream` but never deletes or casts the handle.

For Raspberry Pi composition, create the shared owner before its adapter and
controller:

```cpp
XWalkHal::XWalkAudioAlsa audio(pcmDevice, mixerDevice, mixerElement);
XWalkHal::XWalkSpeakerAlsa adapter(audio, playbackVolumePercent);
XWalkHal::XWalkSpeaker speaker(&adapter, adapter.callbacks());
```

The audio owner must outlive the adapter, and the adapter must outlive the
Speaker controller and all its workers. Disabling Speaker does not close or mute
the shared owner because Music or speech consumers may still use it.

## Ported behavior

- Speaker output is enabled during construction and disabled during destruction.
- WAV, FLAC, and OGG files select the native SoundFile decoder family.
- MP3, M4A, AAC, and WMA files select the native compressed-audio decoder family.
- Decoded samples use interleaved normalized floating-point values.
- Each `play()` call returns a backend-generated unique task identifier.
- Progress includes frame position, total frames, ratio, elapsed and total seconds, and playing state.
- Pause, resume, stop, active-task listing, normal completion, and cleanup are supported.
- Worker backend operations must not throw; a violation terminates the process.

Task storage is deliberately bounded to eight concurrent tasks. Playback writes contain at most 1,024
frames, and paused
workers inspect their state every 10 milliseconds. These limits prevent
unbounded task metadata growth and provide predictable control latency between
backend writes.

Mutating operations must be called from one controlling execution context.
The internal mutex coordinates that context with playback workers. Backend
write operations must return in bounded time so stop and destruction can join
workers safely.

The core library does not link `pyaudio`, `soundfile`, `librosa`, or NumPy. A
platform implementation supplies equivalent decode and stream callbacks.

The built-in adapter checks the file size before reading and accepts at most
16 MiB of input and 2,000,000 decoded interleaved samples per task. It decodes
non-empty 16-bit integer PCM WAVE data with one through eight channels. Other
formats require an explicitly selected bounded decoder callback; optional codec
libraries remain outside `xWalkSpeaker` and `xWalkAudio`.

## Trace output and persistence

The module uses unique `RPI` identifiers for successful controller-thread task
operations. Enabled messages are written to the terminal and configured log
file. A selector such as `RPI.316.enable` updates the generated XML catalogue;
later runs load that state without another selector.

The worker, cleanup, output-disable, worker-callback, and destructor paths
contain no trace operations.

## Silent simulation

The standalone simulation composes the real Speaker task controller over an
in-memory decoder and stream backend. It creates and removes one empty fixture
below the build tree, does not open ALSA, and produces no physical sound. See
[simulation/README.md](simulation/README.md).

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker -B xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker/build-host -DXWALK_SPEAKER_BUILD_HOST_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker/build-host --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker/build-host --output-on-failure
```

The host suites use module-local fixtures plus injected decoder and ALSA
operations. They do not access an audio device.

The concurrency suite remains enabled under ThreadSanitizer. Process-isolated
exception and worker-termination scenarios run in a separate host test because
forking a process with active instrumented worker threads can deadlock the
ThreadSanitizer runtime. Both speaker tests have a 30-second CTest timeout.

## Target compilation

```bash
cmake -S xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker -B xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker/build-rpi -DXWALK_SPEAKER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker/build-rpi --parallel
ctest --test-dir xWalk-rpi5/xWalkHal/layer1/xWalkSpeaker/build-rpi -N -L hardware
```

This requires ALSA development headers. Use `aplay -l`, `aplay -L`, and
`amixer scontrols` to confirm the configured devices and mixer element.

The registered hardware test generates a 256-frame silent WAVE file below
`/tmp`, applies five-percent mixer volume, plays it, and removes the fixture.
Merely list the test during normal verification. Run it only after confirming
the correct Raspberry Pi and Robot HAT audio setup. Optional arguments are PCM
device, mixer device, and mixer element names, in that order.
