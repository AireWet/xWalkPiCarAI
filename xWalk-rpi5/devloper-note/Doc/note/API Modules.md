# Sensors and actuators

## `XWalkUltrasonic`

Uses caller-created trigger and echo `XWalkGpio` objects. Successful results are
in centimetres; documented negative values distinguish timeout and invalid
pulse measurements. See [xWalkUltrasonic](../../../xWalkHal/device/xWalkUltrasonic/README.md).

## `XWalkAdxl345`

Uses a caller-created `XWalkI2c` object to acquire raw and converted acceleration
data. See [xWalkAdxl345](../../../xWalkHal/device/xWalkAdxl345/README.md).

## `XWalkLed` and `XWalkRgbLed`

`XWalkLed` controls one GPIO LED. `XWalkRgbLed` coordinates three non-owning PWM
pointers. See [xWalkLed](../../../xWalkHal/sensor/xWalkLed/README.md).

## `XWalkBuzzer`

Supports active GPIO and passive PWM operation without owning either dependency.
See [xWalkBuzzer](../../../xWalkHal/sensor/xWalkBuzzer/README.md).

## `XWalkGrayscaleModule` and `XWalkLineTracker`

The grayscale component calibrates three ADC channels. The line tracker derives
channel status and normalized line position. See
[xWalkLineTracker](../../../xWalkHal/sensor/xWalkLineTracker/README.md).

## `XWalkUserButton`

Interprets active-low button events and bounded press timing through a
caller-created GPIO dependency. See [xWalkUserButton](../../../xWalkHal/device/xWalkUserButton/README.md).
