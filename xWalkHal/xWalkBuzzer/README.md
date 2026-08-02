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
├── test/
│   ├── hardware/src/xHal_Rpi5CarBuzzerHardwareTest.cpp
│   └── src/xHal_Rpi5CarBuzzerTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarBuzzer.h` | Public API and non-owning dependency contracts |
| `xHal_Rpi5CarBuzzer.cpp` | Output, frequency, validation, and playback behavior |
| `xHal_Rpi5CarBuzzerLifecycle.cpp` | Dependency binding and initial inactive state |
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

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkBuzzer -B xWalkBuzzer/build-host -DXWALK_BUZZER_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkBuzzer/build-host --parallel
ctest --test-dir xWalkBuzzer/build-host --output-on-failure
```

The host configuration runs the Buzzer, PWM, I2C, and GPIO suites without
accessing physical devices.

## Hardware compile and test discovery

```bash
cmake -S xWalkBuzzer -B xWalkBuzzer/build-rpi -DXWALK_BUZZER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkBuzzer/build-rpi --parallel
ctest --test-dir xWalkBuzzer/build-rpi -N -L hardware
```

These commands compile and list hardware tests without executing them. Running
the buzzer hardware executable accesses `/dev/i2c-1` and changes PWM channel zero.
