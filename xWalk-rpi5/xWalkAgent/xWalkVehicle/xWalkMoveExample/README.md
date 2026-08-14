# xWalkMoveExample

`xWalkMoveExample` ports the bounded movement sequence from upstream
`example/2.move.py` into a reusable Agent. It drives forward at 30 percent,
sweeps steering, stops, then sweeps camera pan and tilt with the original timing.

The Agent observes a caller-owned `XWalkPicarx` and caller-owned scheduling
callbacks. Cancellation and destruction perform a best-effort motor stop.

The `xWalkController` command `move demo` delegates to this module. Host tests use only
the deterministic in-memory HAL graph; running the command on Raspberry Pi
physically moves the car and all three servos.
