# xWalkAppControl

`xWalkAppControl` ports upstream `example/12.app_control.py` into a bounded,
provider-neutral foreground coordinator. It publishes speed, grayscale, and
distance telemetry and consumes the SunFounder A-Q control state through an
injected transport.

The coordinator preserves joystick driving, camera pan/tilt, voice movement,
line tracking, obstacle avoidance, horn requests, and red/face detection. Line
recovery is bounded and failed ultrasonic samples stop the vehicle. TensorFlow
object detection is reported as unavailable because the current OpenCV provider
does not expose the upstream TFLite model.

Network binding belongs to the application-owned provider and must be explicitly
configured. The Agent itself never opens a listener or starts Vilib's web server.
