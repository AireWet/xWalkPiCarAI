# xWalkCliffDetection

`xWalkCliffDetection` ports the state machine from upstream
`example/5.cliff_detection.py`. Each bounded step reads all three grayscale
channels. A safe result stops the motors; a cliff result reverses at 80-percent
requested power and waits 100 milliseconds only on a safe-to-danger transition.

The Agent uses the active persisted `cliff_reference` values. It deliberately
does not overwrite calibrated references with the Python example's illustrative
`[200,200,200]` values. Run `calibrate grayscale` before physical use.

## Layout and responsibilities

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarCliffDetection.h` | Public bounded step and stop API |
| `include/xAgent_Rpi5CarCliffDetectionTypes.h` | Callback aliases and step result type |
| `src/xAgent_Rpi5CarCliffDetection.cpp` | Grayscale sampling and safe/danger state machine |
| `src/xAgent_Rpi5CarCliffDetectionLifecycle.cpp` | Dependency validation and cancellation polling |
| `test/src/xAgent_Rpi5CarCliffDetectionTest.cpp` | Device-free state-transition verification |

Use `xwalk-picarx-control cliff-detection start` for the foreground loop and
SIGINT or SIGTERM to exit. `cliff-detection stop` performs an immediate motor
stop. Physical execution requires calibrated references and a protected test
area where reverse motion cannot drive the vehicle off another edge.
