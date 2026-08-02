# xWalkUserButton

C++17 active-low Robot HAT user-button monitoring for the xWalk Firmware HAL.

The submodule monitors one active-low pull-up GPIO input and dispatches press,
release, click, state-change, long-press, and long-press-release callbacks.

## Directory layout

```text
xWalkUserButton/
├── include/
│   ├── xHal_Rpi5CarUserButton.h
│   └── xHal_Rpi5CarUserButtonTypes.h
├── src/
│   ├── xHal_Rpi5CarUserButton.cpp
│   └── xHal_Rpi5CarUserButtonLifecycle.cpp
├── test/
│   ├── hardware/src/xHal_Rpi5CarUserButtonHardwareTest.cpp
│   └── src/xHal_Rpi5CarUserButtonTest.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarUserButton.h` | Public API, synchronized state, and worker ownership |
| `xHal_Rpi5CarUserButtonTypes.h` | Context-based callback function-pointer aliases |
| `xHal_Rpi5CarUserButton.cpp` | Polling, transitions, timing, and callbacks |
| `xHal_Rpi5CarUserButtonLifecycle.cpp` | GPIO binding and deterministic worker cleanup |
| `xHal_Rpi5CarUserButtonTest.cpp` | Short press, long press, validation, and failure tests |

## Composition and ownership

The application creates the GPIO as an input with pull-up bias and passes it by
reference. `XWalkUserButton` stores a non-owning pointer, so the GPIO must
outlive the monitor and its worker.

```cpp
XWalkHal::XWalkGpio buttonGpio(&backend, callbacks, "USER",
    XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up);
XWalkHal::XWalkUserButton button(buttonGpio);
```

The controller owns only its monitoring thread. `stop()` and destruction join
the worker but do not close or release the caller-owned GPIO.

## Ported behavior

- Active-low button state over a pull-up GPIO input
- 50-millisecond polling interval
- Initial GPIO sample used as the transition baseline without dispatching an event
- Press, release, short-click, and combined state callbacks
- Long-press and long-press-release callbacks
- Shared long-press threshold clamped from 2.0 through 5.0 seconds
- Long-press configuration captured when each press begins
- Click suppressed after a recognized long press
- Active and most recently completed press duration in seconds
- Idempotent `start()` while monitoring is already active
- Worker GPIO and callback operations must not throw; a violation terminates the process
- Deterministic worker joining during destruction

A button already held when `start()` samples the GPIO remains the initial baseline. It must be released and
pressed
again before a press transition is dispatched.

## Callback restrictions

Callbacks execute on the monitoring worker. They must return promptly and must
not call `stop()`, `close()`, or destroy the button object because those actions
would attempt to join the current thread. Callback contexts are non-owning and
must remain valid until the callback is cleared and the worker is joined.

Configuration and control methods may be called from one controlling execution
context. State and callback configuration are mutex-protected for observation
while monitoring runs.

## Host build and test

Run these commands from the repository root:

```bash
cmake -S xWalkUserButton -B xWalkUserButton/build-host -DXWALK_USER_BUTTON_BUILD_HOST_TESTS=ON
cmake --build xWalkUserButton/build-host --parallel
ctest --test-dir xWalkUserButton/build-host --output-on-failure
```

The host configuration runs the UserButton and GPIO suites without accessing a
physical GPIO device.

## Hardware compile and test discovery

```bash
cmake -S xWalkUserButton -B xWalkUserButton/build-rpi -DXWALK_USER_BUTTON_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkUserButton/build-rpi --parallel
ctest --test-dir xWalkUserButton/build-rpi -N -L hardware
```

These commands compile and list hardware tests without executing them. Running
the executable accesses `/dev/gpiochip0` and claims the Robot HAT USER input.
