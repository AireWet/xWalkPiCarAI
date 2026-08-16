# Diagnostics and Troubleshooting

Start with configuration and passive inspection. Do not use calibration or motor motion to diagnose missing
devices, permissions, power, or wiring.

## Ordered diagnostic sequence

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
ls -l /dev/i2c-1 /dev/gpiochip0
```

```bash
i2cdetect -y 1
```

Only after those checks pass should a secured hardware session proceed to sensor and actuator tests.

## Configuration failures

Use the exact reported key and range. Compare a custom file with the normalized default:

```bash
python3 -m xwalk_rpi5_py3.cli print-config
```

Do not disable validation or widen limits merely to make an invalid file load.

## I2C and Robot HAT

Confirm the boot setting:

```bash
grep -n '^dtparam=i2c_arm=on$' /boot/firmware/config.txt
```

Confirm the kernel modules:

```bash
lsmod | grep i2c
```

Confirm bus 1 exists and scan it:

```bash
ls -l /dev/i2c-1
```

```bash
i2cdetect -y 1
```

The pinned Robot HAT code probes MCU addresses `0x14` and `0x15`. Another address does not establish Robot HAT
identity or revision. If neither responds, stop and inspect HAT power, seating, wiring, boot settings,
and permissions.

## Permission failures

Inspect ownership rather than running the application as root:

```bash
ls -l /dev/i2c-1 /dev/gpiochip0
```

Preview the repository's group and boot changes:

```bash
./scripts/configure_hardware.sh --dry-run
```

After applying approved group changes, log out and back in. Do not grant world-write device permissions.

## Camera

List camera devices first:

```bash
v4l2-ctl --list-devices
```

Run discovery without capture:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose-camera
```

Capture only after discovery succeeds and the destination is writable:

```bash
python3 -m xwalk_rpi5_py3.cli diagnose-camera --capture-frame
```

Power down before reseating a CSI cable. Raspberry Pi OS documentation may name `rpicam-hello` or the older
`libcamera-hello`; Ubuntu package availability differs.

## Audio

List playback devices without playing sound:

```bash
aplay -l
```

Inspect relevant overlays:

```bash
grep -n 'dtoverlay=.*soundcard\|dtoverlay=hifiberry-dac' /boot/firmware/config.txt
```

Preview project-owned audio configuration before changing it:

```bash
./scripts/setup_robot_hat_audio.sh --with-mic --dry-run
```

The official `i2samp.sh` can replace global audio configuration and perform record/play tests. Do not rerun it
blindly on Ubuntu. Retain the backup path printed by the project script.

## Servo or motor anomalies

Immediately stop and remove motor power when motion is unexpected. Check mechanical installation, neutral
offsets, direction mapping, angle limits, and obstruction before trying again. A blocked servo can enter a
protection state; remove the obstruction rather than forcing its arm.

Use only one servo at a time and begin with a small angle. Test motors with raised wheels, minimum configured
speed, a short duration, watchdog supervision, and a reachable physical disconnect.

## Dependency and platform failures

Confirm architecture and Python environment:

```bash
uname -m
```

```bash
python3 --version
```

```bash
python3 -m pip show xwalk-rpi5-py3 PyYAML
```

The upstream packages have incomplete dependency metadata. Camera availability differs between Raspberry Pi OS
and Ubuntu. Use the pinned repository setup instead of unreviewed latest versions.
