# xWalkVideoCar

`xWalkVideoCar` ports upstream `example/11.video_car.py` behind the existing
PiCar-X and computer-vision interfaces. It preserves the interactive speed,
motion, steering, photo, warm-up, and per-key delay behavior.

The module deliberately does not start Vilib's implicit web display server.
Camera acquisition and timestamped photo storage are supplied by the existing
OpenCV provider, whose output directory is configured through the CLI YAML.

The `video-car` CLI command accepts `o`, `p`, `w`, `s`, `a`, `d`, `f`, and `t`.
Use `x` to stop safely and leave the interactive command.
