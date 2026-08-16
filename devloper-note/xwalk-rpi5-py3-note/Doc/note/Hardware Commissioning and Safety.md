# Hardware Commissioning and Safety

This procedure contains physical actuation and is intentionally not automated. It requires explicit approval,
the correct Raspberry Pi 5 and Robot HAT, known wiring, stable support, and a person able to disconnect power.

## Before power-up

- Identify the Pi, Robot HAT revision, battery, motors, servos, sensors, camera, and audio wiring.
- Disconnect or disable motor power and raise every wheel clear of the surface.
- Support the chassis so steering and camera servos cannot strike the bench, cables, or hands.
- Keep the battery switch and a physical power disconnect reachable.
- Define the expected result and stop condition for each command.
- Do not work on live connectors or manually rotate a powered servo.

## Stage 1: software and passive checks

```bash
source .venv/bin/activate
```

```bash
python3 -m xwalk_rpi5_py3.cli validate-config
```

```bash
python3 -m xwalk_rpi5_py3.cli diagnose
```

```bash
python3 scripts/check_hardware.py
```

```bash
ls -l /dev/i2c-1
```

```bash
i2cdetect -y 1
```

Stop if the platform, permissions, expected MCU response, HAT power, or wiring is uncertain.

## Stage 2: hardware initialization and sensors

Hardware initialization can position all three servos. Clear their travel before continuing:

```bash
python3 -m xwalk_rpi5_py3.cli test-sensors --backend hardware
```

Verify ultrasonic distance, all three grayscale readings, and battery data against known stimuli. A plausible
value is not sufficient if the wrong channel could produce it.

## Stage 3: individual servos

Enable the repository's physical-test permission in the reviewed configuration. Exercise one named servo with
a small angle and interactive confirmation:

```bash
python3 -m xwalk_rpi5_py3.cli test-servos --servo steering --angle 5
```

Repeat separately for pan and tilt only after checking mechanical direction and limits. Stop on chatter,
collision, binding, unexpected direction, or a failure to return to the expected neutral position.

## Stage 4: stop path

Verify the software stop before enabling motor power:

```bash
python3 -m xwalk_rpi5_py3.cli emergency-stop --backend hardware
```

Confirm the physical disconnect independently. Software emergency stop cannot recover from loss of process,
kernel control, board power integrity, or incorrect wiring.

## Stage 5: bounded motor test

With wheels raised, enable motor power and run a short low-speed test:

```bash
python3 -m xwalk_rpi5_py3.cli test-motors --allow-motion --speed 5 --duration 0.25 --direction forward
```

Verify each wheel's direction and stop behavior. Verify watchdog expiry and emergency stop. Do not put the
car on the ground until both motors, direction mapping, steering, watchdog, and physical stop path are proven.

## Stage 6: application start

Only after the earlier stages pass:

```bash
python3 -m xwalk_rpi5_py3.cli run --backend hardware
```

The application starts disarmed. Application code must arm explicitly and refresh the watchdog while motor
motion is intended.

## Hardware test suite

This test selection may actuate attached hardware and must not run in normal CI or documentation checks:

```bash
pytest -m hardware
```

Use `pytest -m "not hardware"` for routine verification.

## Evidence record

Record the date, operator, Pi model, OS image, kernel, Robot HAT revision, battery, wiring, project commit,
submodule commits, configuration checksum, command, expected and observed results, and rollback. Mark only
the individually demonstrated capabilities as verified.
