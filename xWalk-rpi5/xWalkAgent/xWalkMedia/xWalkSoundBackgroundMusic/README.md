# xWalkSoundBackgroundMusic

`xWalkSoundBackgroundMusic` ports upstream
`example/13.sound_background_music.py` through one caller-owned `XWalkMusic`.
The `q` key toggles the supplied MP3, space plays the horn synchronously, and
`c` starts the horn in the background. Both horn operations preserve the
source's 50-millisecond post-play delay.

The Agent resolves reviewed sound and music assets below caller-owned resource
directories, sets streamed-music volume to 20 percent at startup, and stops
active music during normal or cancelled cleanup. It owns no ALSA device,
decoder, terminal, worker, or filesystem deployment policy.

The Raspberry Pi application keeps `SIGINT` and `SIGTERM` on its Controller
thread. ALSA playback workers block those signals, allowing `Ctrl+C` to
interrupt terminal input and complete the same joined cleanup path as `x`.
