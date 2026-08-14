# xWalkUltrasonic

C++17 two-pin ultrasonic ranging for the xWalk Firmware HAL.

`XWalkUltrasonic` emits a bounded trigger pulse, measures the echo with a monotonic clock, and returns
distance in centimeters. It stores non-owning pointers to two caller-created `XWalkGpio` objects.

## Directory layout

```text
xWalkUltrasonic/
├── include/xHal_Rpi5CarUltrasonic.h
├── src/
│   ├── xHal_Rpi5CarUltrasonic.cpp
│   └── xHal_Rpi5CarUltrasonicLifecycle.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarUltrasonicTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/xHal_Rpi5CarUltrasonicTestSupport.h
│   └── src/
│       ├── xHal_Rpi5CarUltrasonicTest.cpp
│       └── xHal_Rpi5CarUltrasonicTestSupport.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarUltrasonic.h` | Public sensor API, constants, and ownership contract |
| `xHal_Rpi5CarUltrasonic.cpp` | Trigger pulse, echo timing, conversion, and retry behavior |
| `xHal_Rpi5CarUltrasonicLifecycle.cpp` | GPIO binding and initial configuration |
| `xHal_Rpi5CarUltrasonicTest.cpp` | Echo, timeout, validation, and trace-selector coverage |
| `xHal_Rpi5CarUltrasonicTestSupport.cpp` | Reusable named in-memory GPIO test callbacks |

## Composition

The application creates the GPIO backends and objects before the sensor, then passes both pins by reference:

```cpp
XWalkGpio trigger(&triggerBackend, triggerCallbacks, "D2");
XWalkGpio echo(&echoBackend, echoCallbacks, "D3");
XWalkUltrasonic ultrasonic(trigger, echo);
const float64 distanceCentimeters = ultrasonic.read();
```

The trigger and echo GPIO objects must outlive the sensor. Construction configures the trigger as an output
and the echo as an input with its internal pull-down enabled. The sensor does not allocate or own either pin.

## Ported behavior

- One-millisecond inactive settling interval
- Ten-microsecond active trigger pulse
- Configurable echo timeout, defaulting to 20 milliseconds
- Distance conversion using a sound speed of 343.3 meters per second
- Distance rounded to two decimal places in centimeters
- Up to ten timeout-only retries by default
- `-1.0` result when every attempt times out
- `-2.0` result when echo transitions do not form a measurable pulse

Callers must serialize access when a sensor is shared between tasks because one reading controls both pins
until the measurement completes.

## Host build and tests

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkUltrasonic -B xWalk-rpi5/xWalkHal/device/xWalkUltrasonic/build-host -DXWALK_ULTRASONIC_BUILD_HOST_TESTS=ON
cmake --build xWalkUltrasonic/build-host --parallel
ctest --test-dir xWalkUltrasonic/build-host --output-on-failure
```

The host test uses callback-driven GPIO simulation. It does not access a Linux GPIO device.
Reusable callback state lives in `xwalk::hal::test::ultrasonic` rather than the
scenario source.

## Standalone host simulation and tracing

The standalone simulation composes the public GPIO and Ultrasonic interfaces
with an in-memory backend. It never opens `/dev/gpiochip*` or drives a pin.

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkUltrasonic/simulation -B build-ultrasonic-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-ultrasonic-simulation --parallel
./build-ultrasonic-simulation/xWalkUltrasonicSimulation --trace RPI.enable
```

Trace selectors accept `RPI.<digits>.enable`, `RPI.enable`, `all.enable`, their
`.disable` counterparts, or a trace-update JSON path. Successful changes update
the generated XML and load automatically on the next run. Enabled traces appear
in the terminal and `build-ultrasonic-simulation/log/xWalkUltrasonicSimulation.log`.

## Hardware compilation without execution

```bash
cmake -S xWalk-rpi5/xWalkHal/device/xWalkUltrasonic -B xWalk-rpi5/xWalkHal/device/xWalkUltrasonic/build-rpi -DXWALK_ULTRASONIC_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkUltrasonic/build-rpi --parallel
ctest --test-dir xWalkUltrasonic/build-rpi -N -L hardware
```

This configuration compiles `xWalkUltrasonic`, the Linux GPIO backend, and its hardware test. It does not
register an automatic ultrasonic measurement test because trigger and echo wiring is application-specific.
