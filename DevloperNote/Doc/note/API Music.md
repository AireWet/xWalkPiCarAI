# `XWalkMusic`

`XWalkMusic` provides music-theory conversion, PCM tone generation, and injected
audio operations. It does not own an ALSA device or launch a platform process.

## Public interface

See [xWalkMusic](../../../xWalkHal/xWalkMusic/README.md) and
[`xHal_Rpi5CarMusic.h`](../../../xWalkHal/xWalkMusic/include/xHal_Rpi5CarMusic.h).

The application supplies audio callbacks and keeps callback contexts valid for
the complete object lifetime. Validate note names, tempo, duration, volume, and
sample properties using the ranges documented in the header.
