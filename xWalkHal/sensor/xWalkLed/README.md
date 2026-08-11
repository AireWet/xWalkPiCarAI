# xWalkLed

C++17 single-color and RGB LED control for the xWalk Firmware HAL.

This combined submodule provides a GPIO-backed single-color LED with background
blinking and a three-channel PWM RGB LED with common-terminal polarity support.
The compatibility target name `xWalkRgbLed` aliases the combined `xWalkLed`
library, while configuration and testing use only the `XWALK_LED_*` options.

## Directory layout

```text
xWalkLed/
├── include/
│   ├── xHal_Rpi5CarLed.h
│   ├── xHal_Rpi5CarRgbLed.h
│   └── xHal_Rpi5CarRgbLedTypes.h
├── src/
│   ├── xHal_Rpi5CarLed.cpp
│   ├── xHal_Rpi5CarLedLifecycle.cpp
│   ├── xHal_Rpi5CarRgbLed.cpp
│   └── xHal_Rpi5CarRgbLedLifecycle.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarLedTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/xHal_Rpi5CarLedTestSupport.h
│   ├── hardware/src/
│   │   ├── xHal_Rpi5CarLedHardwareTest.cpp
│   │   └── xHal_Rpi5CarRgbLedHardwareTest.cpp
│   └── src/
│       ├── xHal_Rpi5CarLedTest.cpp
│       ├── xHal_Rpi5CarLedTestSupport.cpp
│       └── xHal_Rpi5CarRgbLedTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarLed.h` | Single-color LED API, worker state, and GPIO ownership |
| `xHal_Rpi5CarRgbLed.h` | RGB LED API and three non-owning PWM dependencies |
| `xHal_Rpi5CarRgbLedTypes.h` | Common-terminal enumeration and fixed RGB color type |
| `xHal_Rpi5CarLed.cpp` | Direct output, blinking, and worker control |
| `xHal_Rpi5CarLedLifecycle.cpp` | Single-color LED initialization and worker cleanup |
| `xHal_Rpi5CarRgbLed.cpp` | Color decoding, polarity conversion, and PWM output |
| `xHal_Rpi5CarRgbLedLifecycle.cpp` | RGB PWM binding and common-mode validation |
| `simulation/` | Device-free GPIO/I2C backend, persistent trace configuration, and executable |
| `xHal_Rpi5CarLedTestSupport.*` | Named GPIO/I2C callback state shared by LED host tests |
| `xHal_Rpi5CarLedTest.cpp` | GPIO LED timing, validation, and failure coverage |
| `xHal_Rpi5CarRgbLedTest.cpp` | RGB decoding, polarity, output, and validation coverage |

## Single-color LED composition

The application creates and configures the GPIO object, then passes it by
reference. `XWalkLed` stores a non-owning pointer, so the GPIO must outlive the
LED controller and its blink worker.

```cpp
XWalkHal::XWalkGpio ledGpio(&backend, callbacks, "LED");
XWalkHal::XWalkLed led(ledGpio);
```

The controller owns only its worker thread. `close()` stops blinking and
requests the inactive output but does not release the caller-owned GPIO.

### Single-color behavior

- `on()`, `off()`, and `toggle()` logical output operations
- Existing blinking stopped before direct output operations
- Configurable complete blink cycles, transition delay, and inactive pause
- Repeated sequences until `stopBlinking()` or another output command
- Worker stop checks at most every 10 milliseconds during configured delays
- Normal worker shutdown leaves the LED logically inactive
- Worker hardware operations must not throw; a violation terminates the process
- Destruction joins a remaining worker without releasing the GPIO

The GPIO object's logical polarity determines the physical active level.
Transition delays shorter than 10 milliseconds use the polling interval to
avoid an unbounded busy loop.

Calls that mutate an `XWalkLed` must come from one controlling execution
context. Its atomic state accessors may be read while the worker runs.

## RGB LED composition

The application creates the I2C interface, shared PWM timer state, and three
PWM objects before constructing `XWalkRgbLed`. All three PWM objects must outlive
the RGB controller.

```cpp
XWalkHal::XWalkPwm red(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
XWalkHal::XWalkPwm green(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
XWalkHal::XWalkPwm blue(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
XWalkHal::XWalkRgbLed rgbLed(red, green, blue, XWalkHal::XWalkRgbLedCommon::Anode);
```

### RGB behavior

- Fixed component arrays ordered red, green, and blue, from 0 to 255
- Packed colors encoded as `0xRRGGBB`
- Six-digit hexadecimal text with optional surrounding `#` removal
- Common-anode inversion before PWM percentage conversion
- Common-cathode active-high output without inversion
- Common-anode mode as the default
- Invalid common modes, packed values, and hexadecimal text rejected

RGB PWM writes are sequential. If a later output throws, an earlier physical
channel may already be updated; the stored logical color changes only after all
three writes succeed.

## Trace output and persistence

The module uses unique `RPI` identifiers for ordinary LED operations. Enabled
messages are written to the terminal and configured log file. A selector such
as `RPI.268.enable` updates the generated XML catalogue; later runs load that
state without requiring the selector again.

The destructor and `noexcept` blink worker contain no trace operations.

## Device-free simulation

The standalone simulation composes the real GPIO, I2C, PWM, LED, and RGB LED
interfaces over an in-memory backend. It does not open Linux device nodes or
change a physical light output. See [simulation/README.md](simulation/README.md).

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkHal/sensor/xWalkLed -B xWalkHal/sensor/xWalkLed/build-host -DXWALK_LED_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkLed/build-host --parallel
ctest --test-dir xWalkLed/build-host --output-on-failure
```

The host configuration runs both LED suites and their GPIO, PWM, and I2C
dependency suites without accessing physical devices.

## Hardware compile and test discovery

```bash
cmake -S xWalkHal/sensor/xWalkLed -B xWalkHal/sensor/xWalkLed/build-rpi -DXWALK_LED_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkLed/build-rpi --parallel
ctest --test-dir xWalkLed/build-rpi -N -L hardware
```

These commands compile and list hardware tests without executing them. The
hardware executables access `/dev/gpiochip0` or `/dev/i2c-1` and change outputs.
