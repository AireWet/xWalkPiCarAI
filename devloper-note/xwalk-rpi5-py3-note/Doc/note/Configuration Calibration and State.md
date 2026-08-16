# Configuration, Calibration, and State

## Configuration ownership

`config/picarx_hardware.yaml` owns project runtime settings. The CLI validates it before backend creation.
Calibration data belongs under `state/`; it is not embedded into vendored SunFounder source.

Inspect configuration safely:

```bash
python3 -m xwalk_rpi5_py3.cli validate-config
```

```bash
python3 -m xwalk_rpi5_py3.cli print-config
```

Select another file without replacing the default:

```bash
python3 -m xwalk_rpi5_py3.cli --config PATH validate-config
```

## Default mappings and limits

The shipped configuration defaults to the simulator and uses these PiCar-X assignments:

| Function | Default |
| --- | --- |
| Steering servo | `P2`, `-30..30` degrees |
| Camera pan servo | `P0`, `-90..90` degrees |
| Camera tilt servo | `P1`, `-35..65` degrees |
| Left and right motors | direction `D4`, `D5`; PWM `P13`, `P12` |
| Grayscale sensors | `A0`, `A1`, `A2` |
| Ultrasonic sensor | trigger `D2`; echo `D3` |
| Battery reading | `A4` |
| Maximum forward/reverse speed | `20` / `20` |
| Watchdog timeout | `0.5` seconds |

These are configuration defaults, not proof of the attached Robot HAT revision or wiring. Compare the actual
board, assembly, and official documentation before physical use.

## Safety controls

The configuration controls backend selection, command limits, device assignments, and permission for physical
tests. Keep motion tests disabled during development. Do not widen angle or speed bounds to work around
mechanical interference, reversed installation, or a failed calibration.

## Project state backup and restore

Validate state without changing it:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate
```

Create a backup before replacing calibration data:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate --backup
```

Restore the exact reviewed backup:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate --restore PATH
```

Use `--overwrite` only when the existing target was separately backed up and replacement is intended:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate --restore PATH --overwrite
```

## Physical calibration boundary

SunFounder's `servo_zeroing.py`, `1.cali_servo_motor.py`, and `1.cali_grayscale.py` are interactive upstream
programs. They can position servos and move the vehicle. They are not invoked by the project calibration-state
command and must be run only under the hardware commissioning controls.

For servo zeroing, power the servo before fitting the horn at mechanical zero. Never rotate a powered servo
by hand. For steering and camera calibration, use the smallest correction that centers the mechanism without
driving it into an end stop.

For grayscale calibration, place the complete sensor array over representative light and dark surfaces. Motion
calibration must use raised wheels or a clear, controlled surface as directed by the approved test plan.

## Change review

When configuration or state changes:

1. Record the board revision and physical wiring used to determine the values.
2. Validate the YAML before initializing hardware.
3. Review limits independently from neutral offsets.
4. Run simulator and mocked tests.
5. Back up current physical calibration state.
6. Apply one bounded physical change at a time.
7. Record the observed result and restore the previous state on failure.
