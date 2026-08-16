# Architecture and Safety Model

## Purpose

`xWalk-rpi5-py3` provides one Python API with a deterministic simulator and an explicitly selected PiCar-X
hardware backend. Configuration, validation, command limits, watchdog handling, emergency stop, and shutdown
rules are shared so application behavior can be exercised without importing Raspberry Pi libraries.

The component is software-ready. Physical commissioning is separate evidence and must not be inferred from
mocked tests, source review, successful packaging, or an I2C scan.

## Execution flow

1. The CLI loads `config/picarx_hardware.yaml`, or the path supplied with `--config`.
2. Configuration validation rejects invalid pins, ranges, limits, and incompatible safety settings.
3. `--backend sim` creates the device-free simulator.
4. `--backend hardware` imports the pinned SunFounder packages and constructs hardware only after explicit
   initialization.
5. Motor output remains disarmed until application code calls `arm()` and provides valid commands.
6. The watchdog requires continued refresh while motion is intended.
7. Emergency stop and shutdown force a stop and disarm.

The upstream `Picarx` constructor resets its MCU and positions the steering, pan, and tilt servos.
Consequently,
constructing the hardware backend is not physically passive even when the motors remain disarmed.

## Safety classes

### Device-free

These commands do not require Pi hardware:

```bash
python3 -m xwalk_rpi5_py3.cli --help
```

```bash
python3 -m xwalk_rpi5_py3.cli validate-config
```

```bash
python3 -m xwalk_rpi5_py3.cli print-config
```

```bash
python3 -m xwalk_rpi5_py3.cli run --backend sim
```

```bash
python3 -m xwalk_rpi5_py3.cli test-sensors --backend sim
```

### Passive device inspection

The project checker reads platform identity, device nodes, and I2C responses without constructing `Picarx`:

```bash
python3 scripts/check_hardware.py
```

Camera capture opens a camera and writes a frame when requested, but does not actuate the vehicle:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose-camera --capture-frame
```

### Hardware initialization

Commands that select the hardware backend can position servos during construction. Secure the car before using
them, even if their advertised operation is sensor reading or stopping:

```bash
python3 -m xwalk_rpi5_py3.cli run --backend hardware
```

```bash
python3 -m xwalk_rpi5_py3.cli test-sensors --backend hardware
```

```bash
python3 -m xwalk_rpi5_py3.cli emergency-stop --backend hardware
```

### Explicit physical motion

Servo and motor tests require configuration permission, user confirmation, and command-specific opt-in.
Only run them during an approved hardware session with raised wheels and a reachable power disconnect.

## Backend invariants

- Speed and angle inputs are bounded by configuration.
- Startup is disarmed.
- A stale watchdog stops and disarms motor output.
- Emergency stop is idempotent.
- Shutdown attempts to leave outputs stopped.
- The simulator never imports SunFounder hardware packages.
- Hardware tests carry the `hardware` marker and are excluded from routine CI.

## Readiness vocabulary

Use precise evidence levels:

- **Software ready:** implementation, setup logic, safe tests, and mocked hardware behavior pass.
- **Hardware detected:** the intended Pi and Robot HAT respond through the expected interfaces.
- **Hardware partially verified:** named sensors or actuators have passed bounded physical checks.
- **Hardware fully commissioned:** the complete checklist has passed and its evidence is recorded.

Do not promote the readiness level without the corresponding physical results.
