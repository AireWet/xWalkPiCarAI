# xWalk Grayscale Calibration

`xWalkGrayscaleCalibration` is the Agent-level port of the supplied
`picar-x/example/1.cali_grayscale.py` helper. It composes a caller-owned `XWalkPicarx` coordinator with
injected timing and cancellation callbacks; it does not own Linux devices or terminal input.

The bounded synchronous port preserves these source behaviors:

- steering verification at -30, +30, and zero degrees;
- automatic line-reference sampling while driving the left and right calibration pattern;
- per-channel midpoint calculation from observed minima and maxima;
- stationary cliff-reference averaging and source-compatible threshold adjustment;
- pending values that are persisted only through an explicit `save()` call;
- best-effort motor stop and centered steering after cancellation or destruction.

The Python worker threads are intentionally represented as deterministic 200-millisecond sampling steps.
Each wait polls cancellation in slices no longer than 20 milliseconds. Applications remain responsible for
interactive prompts and explicit operator confirmation. The `xwalk-picarx-control calibrate grayscale`
command provides that application boundary on Raspberry Pi.
The cliff calculation uses exactly ten samples; the supplied Python loop accumulates eleven samples but divides
by ten, which is treated as an upstream off-by-one defect rather than calibration behavior to preserve.

Run the device-free host verification from the workspace root:

```bash
cmake --build --preset host-debug --target xWalkGrayscaleCalibrationTest --parallel
ctest --preset host-debug -R xWalkGrayscaleCalibrationHostTest
```

The line-calibration operation moves the drive motors. Physical use requires a clear reviewed surface,
correct Raspberry Pi and Robot HAT wiring, and explicit operator approval.
