# Command Reference

Activate `.venv` and run commands from `xWalk-rpi5-py3`. The module form shown here is canonical. An editable
installation also provides the equivalent `xwalk-rpi5-py3` console command.

## Global interface

```bash
python3 -m xwalk_rpi5_py3.cli [--config PATH] [--verbose] COMMAND
```

- `--config PATH` selects a YAML configuration instead of `config/picarx_hardware.yaml`.
- `--verbose` enables additional diagnostic logging.
- `--help` is available globally and after every subcommand.

## Runtime and configuration commands

Run the selected backend:

```bash
python3 -m xwalk_rpi5_py3.cli run [--backend {sim,hardware}]
```

The configured backend is used when `--backend` is omitted. Prefer the explicit simulator selection on hosts:

```bash
python3 -m xwalk_rpi5_py3.cli run --backend sim
```

Validate configuration without creating a backend:

```bash
python3 -m xwalk_rpi5_py3.cli validate-config
```

Print the normalized configuration:

```bash
python3 -m xwalk_rpi5_py3.cli print-config
```

## Diagnostics

Report environment and configured capabilities:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose
```

Read sensor values through the chosen backend:

```bash
python3 -m xwalk_rpi5_py3.cli test-sensors [--backend {sim,hardware}]
```

Discover the camera without capture:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose-camera
```

Open the camera and save a diagnostic frame:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose-camera --capture-frame
```

Inspect the Raspberry Pi, device nodes, and Robot HAT MCU response without constructing `Picarx`:

```bash
python3 scripts/check_hardware.py [--config PATH]
```

## Stop and actuation commands

Request stop and disarm through the selected backend:

```bash
python3 -m xwalk_rpi5_py3.cli emergency-stop [--backend {sim,hardware}]
```

The hardware form may position servos during backend construction. It is not a substitute for disconnecting
power when initialization itself is unsafe.

Test exactly one servo at a bounded angle:

```bash
python3 -m xwalk_rpi5_py3.cli test-servos --servo {steering,pan,tilt} --angle ANGLE [--yes]
```

Test motors with explicit motion authorization:

```bash
python3 -m xwalk_rpi5_py3.cli test-motors --allow-motion [--yes]
```

`--allow-motion` is mandatory for motor output. `--yes` accepts the confirmation but does not bypass limits.
Use `--speed SPEED`, `--duration SECONDS`, and `--direction {forward,reverse}` to bound the test. Start with
low speed, short duration, and raised wheels.

## Calibration-state command

Validate current state without writing:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate
```

Create a timestamped state backup:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate --backup
```

Restore a named backup, rejecting replacement unless explicitly approved:

```bash
python3 -m xwalk_rpi5_py3.cli calibrate --restore PATH [--overwrite]
```

This command manages project state. SunFounder's interactive physical calibration programs are separate.

## Setup scripts

```bash
./scripts/setup_ubuntu_picarx.sh [--dry-run] [--skip-camera] [--skip-audio] [--non-interactive] [--hardware]
```

```bash
./scripts/configure_hardware.sh [--dry-run]
```

```bash
./scripts/setup_robot_hat_audio.sh [--dry-run] [--with-mic|--without-mic] [--rollback BACKUP]
```

Always run the relevant `--dry-run` before privileged package, boot, device-permission, or audio changes.

## Device-free quality commands

```bash
python3 -m pip install -e '.[dev]'
```

```bash
pytest -m "not hardware"
```

```bash
ruff check .
```

```bash
mypy src
```

```bash
python3 -m compileall -q src tests scripts
```

```bash
bash -n scripts/setup_ubuntu_picarx.sh scripts/configure_hardware.sh scripts/setup_robot_hat_audio.sh
```

The following command executes physical tests and is documentation only unless a hardware session is approved:

```bash
pytest -m hardware
```
