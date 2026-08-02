# `XWalkGpio`

`XWalkGpio` provides digital mode, pull, polarity, value, and edge operations.
`XWalkGpioLinux` is the optional Linux resource-owning backend.

## Public interface

See [xWalkGpio](../../../xWalkHal/xWalkGpio/README.md),
[`xHal_Rpi5CarGpio.h`](../../../xWalkHal/xWalkGpio/core/include/xHal_Rpi5CarGpio.h), and the
Linux backend header when building for Raspberry Pi.

The application owns callback context lifetime. Keep interrupt callbacks short,
non-blocking, and free from filesystem, network, speech, or playback operations.
