# xWalkTreasureHunt

`xWalkTreasureHunt` ports `example/20.treasure_hunt.py` into a hardware-independent Agent. It preserves the
six random color targets, greater-than-100-pixel success threshold, spoken prompts, 80-percent requested
movement, minus/plus 30-degree turns, bounded half-second motion, space-key repeat, and deterministic cleanup.

The Agent owns no camera, random generator, speech provider, PiCar-X hardware, keyboard thread, or console.
The composition root supplies those services and retains them until the Agent is destroyed. Every delay polls
cancellation in slices no longer than 20 milliseconds, and every movement ends with a motor stop.

## Source layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarTreasureHuntTypes.h` | Callback, configuration, and result contracts |
| `include/xAgent_Rpi5CarTreasureHunt.h` | Public coordinator contract |
| `src/xAgent_Rpi5CarTreasureHunt.cpp` | Detection, key handling, movement, and color naming |
| `src/xAgent_Rpi5CarTreasureHuntLifecycle.cpp` | Validation, startup, timing, and cleanup |
