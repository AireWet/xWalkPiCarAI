# xWalkBuzzer

C++17 active and passive buzzer control for the xWalk Firmware HAL.

The submodule controls active buzzers through GPIO and passive buzzers through
PWM. Passive buzzers additionally support frequency and duration-based playback.

## Directory layout

```text
xWalkBuzzer/
├── include/xHal_Rpi5CarBuzzer.h
├── src/
│   ├── xHal_Rpi5CarBuzzer.cpp
│   └── xHal_Rpi5CarBuzzerLifecycle.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarBuzzerTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/xHal_Rpi5CarBuzzerTestSupport.h
│   ├── hardware/src/xHal_Rpi5CarBuzzerHardwareTest.cpp
│   └── src/
│       ├── xHal_Rpi5CarBuzzerTest.cpp
│       └── xHal_Rpi5CarBuzzerTestSupport.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarBuzzer.h` | Public API and non-owning dependency contracts |
| `xHal_Rpi5CarBuzzer.cpp` | Output, frequency, validation, and playback behavior |
| `xHal_Rpi5CarBuzzerLifecycle.cpp` | Dependency binding and initial inactive state |
| `simulation/` | Silent GPIO/I2C backend, persistent trace configuration, and executable |
| `xHal_Rpi5CarBuzzerTestSupport.*` | Named GPIO/I2C callbacks shared by Buzzer host tests |
| `xHal_Rpi5CarBuzzerTest.cpp` | In-memory PWM and GPIO behavior coverage |

## Composition and ownership

The application creates either a PWM object for a passive buzzer or a GPIO
object for an active buzzer. The selected object is passed by reference and
must outlive the buzzer controller.

```cpp
XWalkHal::XWalkPwm buzzerPwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
XWalkHal::XWalkBuzzer passiveBuzzer(buzzerPwm);

XWalkHal::XWalkGpio buzzerGpio(&backend, callbacks, "D4");
XWalkHal::XWalkBuzzer activeBuzzer(buzzerGpio);
```

Construction immediately requests the inactive state. Destruction does not perform hardware I/O.

## Ported behavior

- Passive-buzzer activation uses a 50 percent PWM duty cycle
- Passive-buzzer deactivation uses a zero percent PWM duty cycle
- Active buzzers use logical GPIO `on()` and `off()` operations
- Passive frequency selection is expressed in Hertz
- Playback without a duration remains active until `off()` is called
- Finite playback divides the duration into equal sounding and silent halves
- Frequency and playback operations are rejected for active buzzers
- Non-finite, negative, and unrepresentable durations are rejected before output

The GPIO object's configured logical polarity determines the physical active
level. `XWalkBuzzer` does not duplicate or override that polarity setting.

## Trace output and persistence

The module uses unique `RPI` identifiers for ordinary Buzzer operations.
Enabled messages are written to the terminal and configured log file. A
selector such as `RPI.283.enable` updates the generated XML catalogue; later
runs load that state without requiring the selector again.

The destructor contains no trace or hardware operations.

## Device-free simulation

The standalone simulation composes real GPIO, I2C, PWM, active-buzzer, and
passive-buzzer interfaces over an in-memory backend. It does not open Linux
device nodes or produce physical sound. See [simulation/README.md](simulation/README.md).

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkHal/sensor/xWalkBuzzer -B xWalkHal/sensor/xWalkBuzzer/build-host -DXWALK_BUZZER_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkBuzzer/build-host --parallel
ctest --test-dir xWalkBuzzer/build-host --output-on-failure
```

The host configuration runs the Buzzer, PWM, I2C, and GPIO suites without
accessing physical devices.

## Hardware compile and test discovery

```bash
cmake -S xWalkHal/sensor/xWalkBuzzer -B xWalkHal/sensor/xWalkBuzzer/build-rpi -DXWALK_BUZZER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkBuzzer/build-rpi --parallel
ctest --test-dir xWalkBuzzer/build-rpi -N -L hardware
```

These commands compile and list hardware tests without executing them. Running
the buzzer hardware executable accesses `/dev/i2c-1` and changes PWM channel zero.
