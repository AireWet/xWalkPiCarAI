# xWalkLineTracking

`xWalkLineTracking` is a C++17 Agent submodule that ports
`example/6.line_tracking.py` into forward, left, right, and line-lost recovery commands while
keeping sensor, motor, servo, configuration, scheduling, and diagnostic ownership outside the module.

## Behavior

- preserves the upstream status priority: all background means stop, then middle means forward, left sensor
  means right, and right sensor means left;
- preserves default forward power 10 percent, steering offset 20 degrees, reverse-recovery power 10 percent,
  reverse-recovery steering 30 degrees, and the one-millisecond post-recovery delay;
- retains the last non-stop direction to choose the upstream reverse-recovery steering sign;
- exposes one deterministic `step()` while the application owns repeated scheduling and cancellation;
- returns the final grayscale readings, decision, recovery-attempt flag, and recovery-timeout flag so the
  application can provide the example's console diagnostics;
- injects timing and stores only a non-owning pointer to caller-created `XWalkPicarx`;
- stops the motors when destroyed or explicitly stopped;
- exposes `finish()` to preserve the source example's final 100-millisecond delay after stopping.

Recovery is bounded to at most 100,000 configured samples and stops on timeout or
when recovery has no directional history. Applications select their own repeated scheduling and cancellation
policy.

## Safety

Calibrate `line_reference` for the actual surface before enabling motion. Start with the wheels lifted, use
low power, keep the steering and camera mechanisms clear, and call `finish()` when application scheduling
ends. Do not run the physical gesture or line-tracking path through ordinary host verification.

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarLineTracking.h` | Step, classification, recovery, and dependency contract |
| `include/xAgent_Rpi5CarLineTrackingTypes.h` | State, configuration, timing callback, and step result |
| `src/xAgent_Rpi5CarLineTracking.cpp` | Classification, movement, sampling, and bounded recovery |
| `src/xAgent_Rpi5CarLineTrackingLifecycle.cpp` | Configuration validation, stop, and retained state |
| `test/src/xAgent_Rpi5CarLineTrackingTest.cpp` | In-memory state, movement, recovery, and validation tests |

Use `XWALK_LINE_TRACKING_BUILD_HOST_TESTS=ON` for standalone host verification. Use
`XWALK_LINE_TRACKING_BUILD_HARDWARE_TESTS=ON` only to compile the module and Linux/RPi dependencies. No
physical line-following test is registered because unattended drive motion is unsafe.
