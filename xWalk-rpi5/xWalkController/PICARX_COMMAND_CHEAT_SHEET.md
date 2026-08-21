# PiCar-X Controller Command Cheat Sheet

This quick reference lists the commands provided by `xwalk-picarx-control`.
Run the setup block once in each terminal before using the examples.

## Safety

- Confirm that the configured Raspberry Pi and Robot HAT match the connected hardware.
- Raise the wheels clear of the surface before testing motion, steering, or calibration.
- Run Doctor before any command that activates hardware.
- Stop a foreground command with `Ctrl+C`. Interactive commands also provide their documented exit key.
- Hardware-labelled tests remain opt-in and are not part of this cheat sheet.

## Terminal setup

```bash
cd /repo/joxjoh24/xWalkPiCarAI
XWALK_PICARX_CLI=build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control
```

## Help and preflight

```bash
$XWALK_PICARX_CLI --help
$XWALK_PICARX_CLI --validate-config
$XWALK_PICARX_CLI --print-effective-config
$XWALK_PICARX_CLI --diagnose --no-hardware
$XWALK_PICARX_CLI doctor
$XWALK_PICARX_CLI --trace CTRL.024.enable doctor
```

Use `--deployment-config PATH` to select a different deployment configuration.
The Raspberry Pi build and provisioning sequence is documented in the
[Controller README](README.md#raspberry-pi-compilation-and-test-discovery).

## Basic driving

```bash
$XWALK_PICARX_CLI move forward --speed 40 --duration 2.5
$XWALK_PICARX_CLI move backward --speed 25 --duration 1
$XWALK_PICARX_CLI move demo
$XWALK_PICARX_CLI turn left --angle 20
$XWALK_PICARX_CLI turn right --angle 20
$XWALK_PICARX_CLI keyboard-control
```

Movement speed is from 0 through 100 percent. Duration is in seconds, and turn
angles are in degrees.

## Camera and sensors

```bash
$XWALK_PICARX_CLI cam pan --angle 45
$XWALK_PICARX_CLI cam tilt --angle -15
$XWALK_PICARX_CLI sensor distance
$XWALK_PICARX_CLI sensor grayscale
$XWALK_PICARX_CLI computer-vision
$XWALK_PICARX_CLI record-video
$XWALK_PICARX_CLI video-car
```

## Live video streaming

For a CSI camera, verify the camera stack and xWalk configuration:

```bash
rpicam-hello --list-cameras
rpicam-hello --nopreview --timeout 1000
xWalkTool/shell-agent/deploy-tool/setup-rpi-local.sh --check
$XWALK_PICARX_CLI --print-effective-config | rg '^video_stream_'
```

The output must select `libcamera`, `csi`, and `127.0.0.1`. An RP1 CFE
`/dev/videoN` node is not a CSI camera source. For a USB camera using `v4l2`,
find its exact device with `v4l2-ctl --list-devices`.

Start the loopback-only MJPEG server on the Raspberry Pi. Leave this command
running until you stop it with `Ctrl+C`:

```bash
$XWALK_PICARX_CLI video-stream
```

In another Raspberry Pi terminal, check and play the stream:

```bash
curl --fail http://127.0.0.1:8080/health
ffplay http://127.0.0.1:8080/stream
```

For workstation playback, keep an SSH tunnel running in one workstation
terminal. The current Raspberry Pi command is:

```bash
ssh -N -L 8080:127.0.0.1:8080 xwalk@192.168.1.156
```

Then play the forwarded stream from another workstation terminal:

```bash
ffplay http://127.0.0.1:8080/stream
```

Streaming defaults are in `xWalkConfig/picar-x.d/vision.conf`. Keep the listener
on `127.0.0.1` and use SSH forwarding for remote access.

## Assisted and autonomous driving

Start an activity:

```bash
$XWALK_PICARX_CLI avoid-obstacles start
$XWALK_PICARX_CLI cliff-detection start
$XWALK_PICARX_CLI line-track start
$XWALK_PICARX_CLI stare-at-you start
$XWALK_PICARX_CLI bull-fight start
$XWALK_PICARX_CLI app-control start
$XWALK_PICARX_CLI treasure-hunt
```

Request a stop for an activity that accepts `start|stop`:

```bash
$XWALK_PICARX_CLI avoid-obstacles stop
$XWALK_PICARX_CLI cliff-detection stop
$XWALK_PICARX_CLI line-track stop
$XWALK_PICARX_CLI stare-at-you stop
$XWALK_PICARX_CLI bull-fight stop
$XWALK_PICARX_CLI app-control stop
```

## Preset actions

```bash
$XWALK_PICARX_CLI self-drive shake-head
$XWALK_PICARX_CLI self-drive nod
$XWALK_PICARX_CLI self-drive wave-hands
$XWALK_PICARX_CLI self-drive resist
$XWALK_PICARX_CLI self-drive act-cute
$XWALK_PICARX_CLI self-drive rub-hands
$XWALK_PICARX_CLI self-drive think
$XWALK_PICARX_CLI self-drive twist-body
$XWALK_PICARX_CLI self-drive celebrate
$XWALK_PICARX_CLI self-drive depressed
$XWALK_PICARX_CLI self-drive forward
$XWALK_PICARX_CLI self-drive backward
$XWALK_PICARX_CLI self-drive honking
$XWALK_PICARX_CLI self-drive start-engine
$XWALK_PICARX_CLI self-drive play-background-music
$XWALK_PICARX_CLI self-drive stop-background-music
```

## Sound

```bash
$XWALK_PICARX_CLI sound play sounds/car-double-horn.wav --volume 80
$XWALK_PICARX_CLI sound music music/slow-trail-Ahjay_Stelino.mp3 --volume 20
$XWALK_PICARX_CLI sound volume 60
$XWALK_PICARX_CLI sound stop
$XWALK_PICARX_CLI sound-background-music
```

At the `sound-music>` prompt, enter Space for the foreground horn, `c` for the
background horn, `q` to toggle music, or `x` to stop and exit. Press Enter after
each key. `Ctrl+C` requests the same joined cleanup path.

Volume is from 0 through 100 percent. Relative media paths are resolved through
the configured resource directory; use `--resource-directory PATH` to override it.

## Voice and model features

These commands require the speech, camera, model, or network services described
by the selected configuration. Jarvis defaults to local Ollama and needs no API
key. Optional Gemini uses `GEMINI_API_KEY`; online OpenAI profiles use
`OPENAI_API_KEY`.

```bash
$XWALK_PICARX_CLI voice-chat start
$XWALK_PICARX_CLI voice-active-car start
$XWALK_PICARX_CLI voice-active-car-gpt start
$XWALK_PICARX_CLI gpt-car start
$XWALK_PICARX_CLI gpt-car start --keyboard --no-img
$XWALK_PICARX_CLI voice-controlled-car start
$XWALK_PICARX_CLI voice-prompt-car start
$XWALK_PICARX_CLI storytelling-robot start
$XWALK_PICARX_CLI text-vision-talk start
$XWALK_PICARX_CLI online-llm-test start
```

Replace `start` with `stop` to request shutdown for any command in this group.

## Calibration and servo zeroing

These commands move hardware. Raise the wheels, clear the servo range, and
confirm the correct Robot HAT before running them.

```bash
$XWALK_PICARX_CLI calibrate
$XWALK_PICARX_CLI calibrate grayscale
$XWALK_PICARX_CLI calibrate servo-motor
$XWALK_PICARX_CLI servo-zeroing
```

## Direct SPI diagnostic

This performs a real SPI transfer and is not part of the bounded Doctor check.
Use it only with the intended SPI device connected.

```bash
$XWALK_PICARX_CLI spi transfer 9F000000
```
