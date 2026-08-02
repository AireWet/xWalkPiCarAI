# xWalkGpio

C++17 Robot HAT GPIO abstraction for the xWalk Firmware HAL.

`XWalkGpio` represents one Robot HAT GPIO line. It stores a non-owning backend context and callback set.
The application creates the backend first, then passes its pointer and callbacks to the GPIO object.

```cpp
xwalk::hal::XWalkGpioLinux backend;
const xwalk::hal::XWalkGpioCallbacks callbacks = XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux);
xwalk::hal::XWalkGpio directionPin(&backend, callbacks, "D4");
static_cast<void>(directionPin.high());
```

One Linux backend instance is dedicated to one `XWalkGpio` instance because it owns that pin's line or
event descriptor. This makes pin ownership explicit and allows a future `XWalkMotor` to receive existing GPIO
and PWM objects by reference while storing non-owning pointers.

`xWalkUltrasonic` also receives separate trigger and echo `XWalkGpio` objects by reference. The sensor
reconfigures those caller-owned objects but does not own or destroy them.

## Directory layout

```text
xWalkGpio/
├── core/
│   ├── include/xHal_Rpi5CarGpio.h
│   └── src/
│       ├── xHal_Rpi5CarGpio.cpp
│       └── xHal_Rpi5CarGpioLifecycle.cpp
├── hardware/
│   ├── include/xHal_Rpi5CarGpioLinux.h
│   ├── src/
│   │   ├── xHal_Rpi5CarGpioLinux.cpp
│   │   └── xHal_Rpi5CarGpioLinuxLifecycle.cpp
│   └── test/src/xHal_Rpi5CarGpioHardwareTest.cpp
├── test/xHal_Rpi5CarGpioTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarGpio.h` | Public GPIO API, enums, callback contracts, and callback bridge |
| `xHal_Rpi5CarGpioLifecycle.cpp` | Callback validation, named-pin mapping, and construction |
| `xHal_Rpi5CarGpio.cpp` | Mode changes, polarity, digital I/O, and interrupt forwarding |
| `xHal_Rpi5CarGpioLinux.h` | Linux backend ownership and concurrency contract |
| `xHal_Rpi5CarGpioLinuxLifecycle.cpp` | GPIO chip and line descriptor lifecycle |
| `xHal_Rpi5CarGpioLinux.cpp` | Linux line claims, digital I/O, event polling, and debounce |

## Ported behavior

| GPIO contract | C++ behavior |
|---|---|
| Numeric pins from the Robot HAT dictionary | Preserved with validation |
| Names `D0` through `D16` and hardware aliases | Preserved |
| Default output mode and low value | Preserved |
| Reading an output automatically changes it to input | Preserved |
| Writing an input automatically changes it to output | Preserved |
| Pull-up, pull-down, and no-pull configuration | Preserved |
| `on`, `off`, `high`, and `low` aliases | Preserved |
| Rising, falling, and both-edge handlers | Preserved |
| Default 200-millisecond interrupt debounce | Preserved |
| `close` and `deinit` interrupt cancellation | Preserved |
| Logical active state | Implemented as explicit logical polarity |

The supported aliases include `SW`, `USER`, `LED`, `BOARD_TYPE`, `RST`, `BLEINT`, `BLERST`, `MCURST`,
and `CE`. Duplicate aliases intentionally resolve to the same Linux GPIO line.

## Linux backend

The C++ backend uses the standard Linux GPIO character-device API directly. The current implementation uses
the compatible
version-one GPIO userspace ABI exposed by `<linux/gpio.h>`.

The Linux owner accepts a deployment-selected device path plus optional exact
kernel chip name and label checks. It also accepts a minimum required line
count. Identity or size mismatch closes the descriptor and fails construction
before a GPIO line is claimed. Device selection remains an application boot
responsibility; this backend does not scan `/dev/gpiochip*`.

The event worker uses kernel monotonic timestamps for debounce decisions. Application interrupt handlers run
on the backend worker thread and must not throw, block indefinitely, or outlive their context.

## Host build and tests

The host test uses in-memory callbacks and never opens `/dev/gpiochip0`.

```bash
cmake -S xWalkGpio -B xWalkGpio/build-host -DXWALK_GPIO_BUILD_HOST_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkGpio/build-host --parallel
ctest --test-dir xWalkGpio/build-host --output-on-failure
```

## Hardware compilation without execution

The hardware test accepts `XWALK_GPIO_HARDWARE_DEVICE` plus optional exact
`XWALK_GPIO_HARDWARE_CHIP_NAME` and `XWALK_GPIO_HARDWARE_CHIP_LABEL` CMake
values. These should match the same provisioned controller used by boot.

```bash
cmake -S xWalkGpio -B xWalkGpio/build-rpi -DXWALK_GPIO_BUILD_HARDWARE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build xWalkGpio/build-rpi --parallel
ctest --test-dir xWalkGpio/build-rpi -N -L hardware
```

The final command only lists the hardware test. Do not execute it without a connected Robot HAT because the
test claims GPIO26 and drives the LED output low.
