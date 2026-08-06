# xWalkFaceTracking

`xWalkFaceTracking` ports `example/8.stare_at_you.py` behind the Agent boundary.
It consumes face coordinates from `xWalkComputerVision`, updates the caller-owned
PiCar-X camera servos, and retains the upstream 640-by-480 correction formula,
35-degree limits, 50-millisecond sample delay, and final 100-millisecond delay.

The module owns no camera or vehicle hardware. Raspberry Pi composition is
provided by `xWalkBoot`; deterministic verification uses injected callbacks.
