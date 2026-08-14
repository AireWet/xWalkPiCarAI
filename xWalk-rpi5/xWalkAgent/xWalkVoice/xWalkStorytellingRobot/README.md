# xWalkStorytellingRobot

`xWalkStorytellingRobot` ports upstream `example/15.storytelling_robot.py`
through caller-owned PiCar-X and text-to-speech services. It preserves the
Piper greeting, two three-second forward legs and jokes, farewell, and final
six-second backward trip at 30-percent requested speed.

Long movements use 20-millisecond cancellation slices. Completion,
cancellation, and exceptions stop both motors and centre steering. Raspberry
Pi composition uses the deployment-selected Piper executable and playback
tool with the source model `en_US-amy-low`; host verification injects speech
and timing without opening audio or moving hardware.
