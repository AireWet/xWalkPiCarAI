# xWalkAudio

`xWalkAudio` provides shared Linux ALSA PCM and mixer ownership for xWalk audio
consumers. It keeps libasound out of the hardware-independent Music, Speaker,
GPT, and VoiceAssistant targets.

`XWalkAudioAlsa` owns one configured mixer for its complete lifetime and up to
eight PCM playback handles opened on demand. It validates sample format, rate,
channel count, period size, latency, and payload length; completes short writes;
and bounds ALSA underrun recovery to three attempts.

The PCM, mixer, and simple-element names are deployment configuration. The
defaults are `default`, `default`, and `PCM`; they do not assume ALSA card zero.
Use `aplay -l`, `aplay -L`, and `amixer scontrols` on the deployed Raspberry Pi
to identify the overlay-provided device and playback element.

This module establishes shared resource ownership only. Music and Speaker
callback adapters remain separate work so neither feature module depends on the
other. Create the audio backend before every adapter or consumer and destroy it
after they have stopped all playback workers and closed their streams.

## Layout

```text
include/xHal_Rpi5CarAudioTypes.h                 Shared configuration and callback seam
hardware/include/xHal_Rpi5CarAudioAlsa.h         ALSA ownership and playback interface
hardware/src/xHal_Rpi5CarAudioAlsa.cpp           Stream, write, recovery, and mixer behavior
hardware/src/xHal_Rpi5CarAudioAlsaLifecycle.cpp  Validation and deterministic cleanup
hardware/src/xHal_Rpi5CarAudioAlsaSystem.cpp     Real libasound operations
hardware/test/src/xHal_Rpi5CarAudioAlsaTest.cpp  Injected software test
hardware/test/src/xHal_Rpi5CarAudioAlsaHardwareTest.cpp Opt-in silent hardware test
```

| File | Responsibility |
| --- | --- |
| `xHal_Rpi5CarAudioTypes.h` | Declares PCM configuration and injected ALSA operations. |
| `xHal_Rpi5CarAudioAlsa.h` | Declares bounded PCM and persistent mixer ownership. |
| `xHal_Rpi5CarAudioAlsa.cpp` | Completes writes and bounds underrun recovery. |
| `xHal_Rpi5CarAudioAlsaLifecycle.cpp` | Validates and releases all handles. |
| `xHal_Rpi5CarAudioAlsaSystem.cpp` | Maps the operation seam to libasound. |
| `xHal_Rpi5CarAudioAlsaTest.cpp` | Tests ownership without opening a sound device. |
| `xHal_Rpi5CarAudioAlsaHardwareTest.cpp` | Writes silence through configured hardware. |

## Software test

The host test links libasound so the real backend is compile-checked, but it
injects every ALSA operation and does not open a sound device or change volume.

```bash
cmake -S xWalkHal/xWalkAudio -B xWalkHal/xWalkAudio/build-host -DXWALK_AUDIO_BUILD_HOST_TESTS=ON
cmake --build xWalkHal/xWalkAudio/build-host --parallel
ctest --test-dir xWalkHal/xWalkAudio/build-host --output-on-failure
```

## Backend-only build

```bash
cmake -S xWalkHal/xWalkAudio -B xWalkHal/xWalkAudio/build-linux -DXWALK_AUDIO_BUILD_LINUX_BACKEND=ON
cmake --build xWalkHal/xWalkAudio/build-linux --parallel
```

## Raspberry Pi hardware test

```bash
cmake -S xWalkHal/xWalkAudio -B xWalkHal/xWalkAudio/build-rpi -DXWALK_AUDIO_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkHal/xWalkAudio/build-rpi --parallel
ctest --test-dir xWalkHal/xWalkAudio/build-rpi -N -L hardware
```

The registered test opens the configured ALSA devices, writes 256 zero-valued
mono frames at 44,100 Hertz, drains and closes the PCM stream, and sets playback
volume to fifty percent. It emits no audible sample, but it remains opt-in
because it claims a physical audio device and changes mixer state.

To select non-default deployment names, run the hardware executable manually
with `PCM_DEVICE MIXER_DEVICE MIXER_ELEMENT` after completing the hardware
safety review.
