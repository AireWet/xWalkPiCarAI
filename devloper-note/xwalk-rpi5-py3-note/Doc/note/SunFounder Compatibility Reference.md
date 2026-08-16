# SunFounder Compatibility Reference

This page summarizes the official SunFounder workflow and maps it to `xWalk-rpi5-py3`. Upstream commands are
reference procedures for Raspberry Pi OS; they are not replacements for the project's pinned Ubuntu setup.

## Official sources

- [PiCar-X V2.0 documentation](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/)
- [Install all
  modules](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/python/install_all_modules.html)
- [Servo adjustment](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/python/py_servo_adjust.html)
- [PiCar-X
  calibration](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/python/python_calibrate.html)
- [Play with Python](https://docs.sunfounder.com/projects/picar-x-v20/en/latest/python/play_with_python.html)
- [Robot HAT hardware
  introduction](https://docs.sunfounder.com/projects/robot-hat-v4/en/stable/hardware_introduction.html)

## Pinned project dependencies

The repository gitlinks and `requirements-sunfounder.lock` are authoritative:

| Project | Compatibility branch | Pinned version | Pinned commit |
| --- | --- | --- | --- |
| `sunfounder/robot-hat` | `2.5.x` | `2.5.5` | `577142df7d3bebd4bc471e35d79f6e1fcf150185` |
| `sunfounder/picar-x` | `2.1.x` | `2.1.0a1` | `897698daf2719d830a5fb0b83951704bfee07d0c` |
| `sunfounder/vilib` | `picamera2` | `0.3.18` | `7d0f77f75cefbdbccf5b5e4398e4664911a2abe3` |

Do not replace these pins with branch tips merely because upstream has newer commits.

## Official Raspberry Pi OS installation

SunFounder's documented procedure updates the system, installs packages, clones compatibility branches, runs
their installers, installs PiCar-X with a system-pip override, and runs the audio script:

```bash
sudo apt update
```

```bash
sudo apt upgrade
```

```bash
sudo apt install git python3-pip python3-setuptools python3-smbus
```

```bash
cd ~
```

```bash
git clone -b 2.5.x https://github.com/sunfounder/robot-hat.git
```

```bash
cd ~/robot-hat
```

```bash
sudo python3 install.py
```

```bash
cd ~
```

```bash
git clone -b picamera2 https://github.com/sunfounder/vilib.git
```

```bash
cd ~/vilib
```

```bash
sudo python3 install.py
```

```bash
cd ~
```

```bash
git clone -b 2.1.x https://github.com/sunfounder/picar-x.git
```

```bash
cd ~/picar-x
```

```bash
sudo pip3 install . --break
```

```bash
cd ~/robot-hat
```

```bash
sudo bash i2samp.sh
```

For this Ubuntu project, use `./scripts/setup_ubuntu_picarx.sh --hardware` instead. It installs pinned
revisions into `.venv`, separates system package management, makes audio opt-in, and supports a dry run.

## Official I2C preparation

SunFounder documents enabling I2C through Raspberry Pi configuration, inspecting modules, installing tools,
scanning bus 1:

```bash
sudo raspi-config
```

```bash
lsmod | grep i2c
```

```bash
sudo apt install i2c-tools
```

```bash
i2cdetect -y 1
```

```bash
sudo apt install python3-smbus2
```

On Ubuntu, preview and use `./scripts/configure_hardware.sh` so boot changes, backups, and device groups are
handled according to this repository's policy.

## Servo zeroing and calibration

Official examples are located under the upstream `example` directory:

```bash
cd ~/picar-x/example
```

```bash
sudo python3 servo_zeroing.py
```

```bash
sudo python3 1.cali_servo_motor.py
```

```bash
sudo python3 1.cali_grayscale.py
```

These programs interact with physical hardware. The servo documentation uses a nominal `-90..90` range and
instructs the installer to establish mechanical zero while powered. The calibration program supports steering,
camera servos, motor direction, and grayscale values. Follow its on-screen keys and stop on mechanical
interference or unexpected motion.

## Official Python examples

Run these only from a correctly installed upstream checkout during an approved physical session:

| Official example | Command | Capability and risk |
| --- | --- | --- |
| Move | `sudo python3 2.move.py` | Drives and steers the car |
| Keyboard control | `sudo python3 3.keyboard_control.py` | Interactive driving |
| Obstacle avoidance | `sudo python3 4.avoiding_obstacles.py` | Ultrasonic-guided motion |
| Cliff detection | `sudo python3 5.cliff_detection.py` | Grayscale-guided motion |
| Line tracking | `sudo python3 6.line_tracking.py` | Continuous sensor-guided motion |
| Computer vision | `sudo python3 7.computer_vision.py` | Camera processing and servos |
| Stare at you | `sudo python3 8.stare_at_you.py` | Camera tracking and servos |
| Record video | `sudo python3 9.record_video.py` | Camera and storage writes |
| Bull fight | `sudo python3 10.bull_fight.py` | Camera-guided vehicle motion |
| Video car | `sudo python3 11.video_car.py` | Network video and remote motion |
| App control | `sudo python3 12.app_control.py` | Network/app-controlled motion |

The commands and numeric filenames belong to SunFounder's upstream examples. They are not
`xwalk_rpi5_py3.cli` subcommands and are not covered by the project's simulator or routine CI.

## Hardware mapping cross-check

The official Robot HAT documentation identifies I2C on Raspberry Pi GPIO 2 and 3, motor PWM channels `P13` and
`P12`, and motor direction GPIO 23 and 24. Project defaults match the PiCar-X library naming, but
the installed HAT revision and cable routing still require physical verification.

## Compatibility boundary

- Official documentation primarily targets Raspberry Pi OS; this project targets Ubuntu 24.04 ARM64 as well.
- Project source review and mocked tests do not prove hardware imports or electrical behavior.
- Vilib's upstream installer and camera packages may not match Ubuntu repositories.
- Robot HAT package metadata does not declare every import-time dependency.
- The upstream `Picarx` constructor can reset the MCU and position servos.
- Audio overlay choice depends on the actual Robot HAT revision.

Use official documents for hardware and examples. Use repository commands for the pinned,
reviewable, rollback-aware execution path.
