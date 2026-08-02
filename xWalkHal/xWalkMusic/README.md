# xWalkMusic

C++17 music timing, tone generation, and audio-output coordination for the xWalk Firmware HAL.

The submodule provides music timing and key state, MIDI-compatible note
frequencies, sound-effect and streamed-music control, and signed 16-bit mono
PCM tone generation. Platform audio is supplied through caller-owned callbacks,
so the core library does not depend on `pygame`, `pyaudio`, or a Linux audio API.
The optional `XWalkMusicAlsa` adapter implements every callback through the
shared `XWalkAudioAlsa` owner. Its built-in decoder accepts uncompressed 16-bit
PCM RIFF/WAVE files with one through eight channels. The optional
`XWalkMusicSndFileDecoder` uses libsndfile to add native MP3 and other
libsndfile-supported formats while preserving the same PCM contract.

## Directory layout

```text
xWalkMusic/
├── include/
│   ├── xHal_Rpi5CarMusic.h
│   └── xHal_Rpi5CarMusicTypes.h
├── hardware/
│   ├── include/
│   │   ├── xHal_Rpi5CarMusicAlsa.h
│   │   ├── xHal_Rpi5CarMusicAlsaTypes.h
│   │   └── xHal_Rpi5CarMusicSndFileDecoder.h
│   ├── src/
│   │   ├── xHal_Rpi5CarMusicAlsaCallbacks.cpp
│   │   ├── xHal_Rpi5CarMusicAlsaDecode.cpp
│   │   ├── xHal_Rpi5CarMusicAlsaLifecycle.cpp
│   │   ├── xHal_Rpi5CarMusicAlsaPlayback.cpp
│   │   └── xHal_Rpi5CarMusicSndFileDecoder.cpp
│   └── test/src/
│       ├── xHal_Rpi5CarMusicAlsaTest.cpp
│       ├── xHal_Rpi5CarMusicAlsaHardwareTest.cpp
│       └── xHal_Rpi5CarMusicSndFileDecoderTest.cpp
├── src/
│   ├── xHal_Rpi5CarMusicLifecycle.cpp
│   ├── xHal_Rpi5CarMusicPlayback.cpp
│   ├── xHal_Rpi5CarMusicTheory.cpp
│   └── xHal_Rpi5CarMusicTone.cpp
├── test/src/xHal_Rpi5CarMusicTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarMusic.h` | Public theory, playback, tone, validation, and lifetime contract |
| `xHal_Rpi5CarMusicTypes.h` | Fixed music values and injected audio callback types |
| `xHal_Rpi5CarMusicLifecycle.cpp` | Callback validation, binding, output enable, and destruction |
| `xHal_Rpi5CarMusicPlayback.cpp` | Sound, streamed music, volume, and transport control |
| `xHal_Rpi5CarMusicTheory.cpp` | Time signature, tempo, beat, key, and note calculations |
| `xHal_Rpi5CarMusicTone.cpp` | Signed 16-bit little-endian mono PCM generation and output |
| `xHal_Rpi5CarMusicTest.cpp` | In-memory callback, theory, playback, tone, and validation tests |
| `xHal_Rpi5CarMusicAlsa.h` | Shared-ALSA adapter ownership, callback, and worker contract |
| `xHal_Rpi5CarMusicAlsaTypes.h` | Decoded PCM data and injected decoder operation types |
| `xHal_Rpi5CarMusicAlsaCallbacks.cpp` | Callback routing, volume, transport, and tone output |
| `xHal_Rpi5CarMusicAlsaDecode.cpp` | Bounded RIFF/WAVE chunk parsing and PCM validation |
| `xHal_Rpi5CarMusicAlsaLifecycle.cpp` | Dependency binding, worker shutdown, and callback publication |
| `xHal_Rpi5CarMusicAlsaPlayback.cpp` | Period writes, looping, pause, resume, and stop observation |
| `xHal_Rpi5CarMusicSndFileDecoder.h` | Stateless optional libsndfile operation provider |
| `xHal_Rpi5CarMusicSndFileDecoder.cpp` | Bounded MP3 and audio-file decoding into signed 16-bit PCM |
| `xHal_Rpi5CarMusicAlsaTest.cpp` | Injected decoder and ALSA callback tests without a device |
| `xHal_Rpi5CarMusicAlsaHardwareTest.cpp` | Opt-in short five-percent-volume tone test |
| `xHal_Rpi5CarMusicSndFileDecoderTest.cpp` | Packaged MP3 decoding test without an audio device |

## Composition

The application creates its audio backend, exposes it through
`XWalkMusicCallbacks`, and then creates the controller. The callback context is
non-owning and must outlive the controller. All callback entries are required.

```cpp
AudioBackend backend;
const XWalkHal::XWalkMusicCallbacks callbacks = makeMusicCallbacks();
XWalkHal::XWalkMusic music(&backend, callbacks);
const XWalkHal::float64 frequencyHz = music.noteFrequencyHz("A4");
music.playToneFor(frequencyHz, music.beatDurationSeconds(XHAL_RPI5CAR_MUSIC_QUARTER_NOTE));
```

Construction invokes `enableOutput` so speaker power is active before playback.
Destruction does not disable or release the caller-owned backend.

For Raspberry Pi composition, create dependencies in ownership order and
destroy them in reverse order:

```cpp
XWalkHal::XWalkAudioAlsa audio(pcmDevice, mixerDevice, mixerElement);
XWalkHal::XWalkMusicAlsa adapter(audio, nullptr,
    XWalkHal::XWalkMusicSndFileDecoder::operations());
XWalkHal::XWalkMusic music(&adapter, adapter.callbacks());
```

`XWalkAudioAlsa` owns PCM and mixer handles. `XWalkMusicAlsa` only observes that
owner and retains decoded data and at most one background-sound worker plus one
streamed-music worker. The adapter must outlive `XWalkMusic`, and the audio owner
must outlive both. Sound effects may temporarily change the shared mixer volume.

## Ported behavior

- 4/4 default time signature and 120 quarter-note beats per minute
- Named major-key macros, integers, and repeated `#` or `b` keys from minus seven through seven
- Named notes from `A0` through `C8`, including sharp spellings
- Equal-temperament conversion from the A4 reference of 440 Hertz at MIDI note 69
- Synchronous and background sound-effect operations with optional volume
- Streamed-music play, volume, stop, pause, resume, and unpause operations
- Sound duration rounded to two decimal places
- Signed 16-bit mono PCM tone data at 44,100 Hertz

Tone generation halves the requested duration, generates that many sine frames, and then appends
`frameCount % 44,100` silent frames. This observable byte-count and silence behavior is intentional.

Inputs that could otherwise cause undefined, unbounded, or ambiguous behavior
are validated. Time-signature values must be non-zero; tempo and note values
must be finite and positive; named notes, MIDI indices, key signatures, volume,
loop counts, offsets, frequency, duration, and backend results are range checked.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkHal/xWalkMusic -B xWalkHal/xWalkMusic/build-host -DXWALK_MUSIC_BUILD_HOST_TESTS=ON
cmake --build xWalkHal/xWalkMusic/build-host --parallel
ctest --test-dir xWalkHal/xWalkMusic/build-host --output-on-failure
```

The default host suite uses injected decoder and ALSA operation tables. Enable
`XWALK_MUSIC_BUILD_SNDFILE_DECODER` to compile the optional decoder and verify
the packaged MP3. Neither path opens an audio device or accesses Robot HAT output.

## Target compilation

```bash
sudo apt install libasound2-dev libsndfile1-dev
cmake -S xWalkHal/xWalkMusic -B xWalkHal/xWalkMusic/build-rpi -DXWALK_MUSIC_BUILD_HARDWARE_TESTS=ON -DXWALK_MUSIC_BUILD_SNDFILE_DECODER=ON
cmake --build xWalkHal/xWalkMusic/build-rpi --parallel
ctest --test-dir xWalkHal/xWalkMusic/build-rpi -N -L hardware
```

This requires ALSA development headers. Before deployment, use `aplay -l`,
`aplay -L`, and `amixer scontrols` to select the PCM, mixer device, and playback
element. Do not assume ALSA card zero.

The registered hardware test sets the configured mixer to five percent and
writes a short 220-Hertz generated tone. Merely list it during normal builds.
Run it only after confirming the correct Raspberry Pi, Robot HAT, speaker, and
safe volume setup. Optional arguments are PCM device, mixer device, and mixer
element names, in that order.
