# Installation and Environment Setup

Run project commands from `xWalk-rpi5-py3` unless a command explicitly says otherwise.

## Simulator host

On Ubuntu 24.04 x86_64, preview and apply only the simulator setup:

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

Do not pass `--hardware` on an ordinary host. Missing Pi devices are expected there.

## Raspberry Pi 5 software preparation

The target procedure is for Ubuntu 24.04 ARM64. First preview every planned package and project change:

```bash
cd xWalk-rpi5-py3
```

```bash
./scripts/setup_ubuntu_picarx.sh --hardware --dry-run
```

Apply the reviewed plan:

```bash
./scripts/setup_ubuntu_picarx.sh --hardware
```

The setup initializes and verifies the pinned Robot HAT, PiCar-X, and Vilib submodules. A new checkout can
materialize them explicitly:

```bash
git submodule update --init --recursive
```

Preview and apply I2C, boot-file, and device-group configuration separately:

```bash
./scripts/configure_hardware.sh --dry-run
```

```bash
./scripts/configure_hardware.sh
```

The script backs up `/boot/firmware/config.txt` before changes and never reboots. Log out and back in after
group changes. Reboot manually only when the script reports that firmware configuration changed.

After returning to the project:

```bash
source .venv/bin/activate
```

```bash
python3 -m xwalk_rpi5_py3.cli validate-config
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

Stop at passive checks until the physical commissioning prerequisites are satisfied.

## Setup options

`setup_ubuntu_picarx.sh` accepts:

- `--dry-run`: print the plan without applying it;
- `--hardware`: install the pinned hardware dependency set;
- `--skip-camera`: omit optional camera dependencies;
- `--skip-audio`: omit optional audio diagnostics;
- `--non-interactive`: reject operations that would require an answer.

Options can be combined:

```bash
./scripts/setup_ubuntu_picarx.sh --hardware --skip-camera --skip-audio --non-interactive --dry-run
```

## Manual virtual environment equivalent

The setup script performs the following project-local operations:

```bash
python3 -m venv --system-site-packages .venv
```

```bash
source .venv/bin/activate
```

```bash
python3 -m pip install --upgrade pip
```

```bash
python3 -m pip install --upgrade setuptools
```

```bash
python3 -m pip install --upgrade wheel
```

```bash
python3 -m pip install -e .
```

System site packages keep Ubuntu camera and I2C bindings managed by `apt`. The project procedure does
not use system-wide `sudo pip`, `--break`, or `--break-system-packages` installation.

## Optional camera and audio setup

Camera setup is dependency installation only. Inspect the camera before capture:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose-camera
```

Audio boot-overlay changes are opt-in and revision-dependent. Preview before applying:

```bash
./scripts/setup_robot_hat_audio.sh --with-mic --dry-run
```

```bash
./scripts/setup_robot_hat_audio.sh --with-mic
```

Use `--without-mic` when only playback is required. The script prints the exact backup path for rollback:

```bash
./scripts/setup_robot_hat_audio.sh --rollback /boot/firmware/config.txt.xwalk-audio-YYYYMMDDTHHMMSSZ.bak
```

## Removal and rollback

Remove only the local environment when it is no longer needed:

```bash
rm -r .venv
```

Deinitialize retained upstream worktrees without changing their committed gitlinks:

```bash
git submodule deinit vendor/sunfounder/robot-hat
```

```bash
git submodule deinit vendor/sunfounder/picar-x
```

```bash
git submodule deinit vendor/sunfounder/vilib
```

Restore an I2C boot backup only after substituting the exact reported timestamp:

```bash
sudo cp -a /boot/firmware/config.txt.xwalk-YYYYMMDDTHHMMSSZ.bak /boot/firmware/config.txt
```
