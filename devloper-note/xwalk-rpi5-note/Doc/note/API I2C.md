# `XWalkI2c`

`XWalkI2c` is the hardware-independent callback-driven bus abstraction.
`XWalkI2cLinux` optionally owns a Linux I2C file descriptor.

## Public interface

See [`xWalkI2c`](../../../../xWalk-rpi5/xWalkHal/interface/xWalkI2c/README.md) and the public headers under
its `core`
and `hardware` include directories.

Create the backend before `XWalkI2c`, pass its pointer and callback bridges to
the constructor, and keep it alive until all dependent ADC, PWM, firmware, and
sensor objects are destroyed. Serialize transactions that share a bus.
