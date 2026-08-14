# Safety and troubleshooting

## Can the battery remain connected during external Raspberry Pi power?

The documented board has an anti-backflow path. Confirm the exact board
revision and wiring before combining power sources.

## Can the board operate while charging?

The documented charging power is approximately 10 watts. A sustained load can
exceed that power and continue discharging the pack. Monitor voltage and
temperature when motors, servos, or audio are active.

## Why is there no speaker output?

1. Confirm the detected board revision and speaker-enable GPIO.
2. Confirm the operating-system I2S and ALSA configuration.
3. Confirm the application-created prime callback succeeds.
4. Confirm `XWalkBoardControl::enableSpeaker()` completes.
5. Confirm the injected speech or playback callback receives the expected data.

## Why are hardware tests not run automatically?

Hardware tests can claim GPIO lines, access I2C, move actuators, or enable audio
power. Build them explicitly and use `ctest -N -L hardware` to list them without
execution during ordinary verification.
