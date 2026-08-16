# xWalk Raspberry Pi 5 Python 3 Notes

This collection explains the Python 3 simulator and opt-in SunFounder PiCar-X hardware backend in
`xWalk-rpi5-py3`. The component [README](../../xWalk-rpi5-py3/README.md) remains the authoritative description
of current implementation status, dependency pins, and verified support.

## Start here

- [Architecture and Safety Model](Doc/note/Architecture%20and%20Safety%20Model.md): backend ownership, arming,
  watchdog, emergency stop, and command safety classes.
- [Installation and Environment Setup](Doc/note/Installation%20and%20Environment%20Setup.md): simulator and
  Raspberry Pi setup, virtual environments, submodules, I2C, camera, and audio choices.
- [Command Reference](Doc/note/Command%20Reference.md): every project CLI command, option, setup script, and
  device-free quality command.
- [Configuration Calibration and State](Doc/note/Configuration%20Calibration%20and%20State.md): YAML settings,
  limits, persistent calibration, backup, and restore.
- [Diagnostics and Troubleshooting](Doc/note/Diagnostics%20and%20Troubleshooting.md): ordered checks for I2C,
  GPIO, camera, audio, permissions, and dependency problems.
- [Hardware Commissioning and
  Safety](Doc/note/Hardware%20Commissioning%20and%20Safety.md): staged first start,
  motion controls, evidence, and stop conditions.
- [SunFounder Compatibility Reference](Doc/note/SunFounder%20Compatibility%20Reference.md): official upstream
  installation and example mapping, with the boundary between upstream and project commands.

## Supported execution paths

Use the simulator on ordinary development hosts. Hardware mode is limited to a Raspberry Pi 5 with the correct
Robot HAT, PiCar-X wiring, power arrangement, permissions, and an approved physical test plan.

```bash
cd xWalk-rpi5-py3
```

```bash
./scripts/setup_ubuntu_picarx.sh --dry-run
```

```bash
./scripts/setup_ubuntu_picarx.sh
```

```bash
source .venv/bin/activate
```

```bash
python3 -m xwalk_rpi5_py3.cli validate-config
```

```bash
pytest -m "not hardware"
```

```bash
python3 -m xwalk_rpi5_py3.cli run --backend sim
```

Do not run hardware-labelled tests, calibration programs, servo tests, motor tests, or the hardware backend as
part of routine documentation verification.
