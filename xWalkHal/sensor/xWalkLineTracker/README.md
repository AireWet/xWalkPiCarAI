# xWalkLineTracker

C++17 fixed-channel grayscale classification and line-tracking module.

The submodule provides fixed three-channel grayscale classification, linear sensor calibration, cliff
detection, line detection, adaptive references, and normalized line-position estimation.

## Directory layout

```text
xWalkLineTracker/
├── include/xHal_Rpi5CarGrayscaleModule.h
├── include/xHal_Rpi5CarLineTracker.h
├── include/xHal_Rpi5CarLineTrackerTypes.h
├── src/
│   ├── xHal_Rpi5CarGrayscaleModule.cpp
│   ├── xHal_Rpi5CarLineTrackerCalibration.cpp
│   ├── xHal_Rpi5CarLineTrackerLifecycle.cpp
│   └── xHal_Rpi5CarLineTrackerReading.cpp
├── simulation/
│   ├── config/xHal_Rpi5CarLineTrackerTraceConfig.py
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
├── test/
│   ├── include/xHal_Rpi5CarLineTrackerTestSupport.h
│   └── src/
│       ├── xHal_Rpi5CarLineTrackerTest.cpp
│       └── xHal_Rpi5CarLineTrackerTestSupport.cpp
├── CMakeLists.txt
└── README.md
```

| File | Responsibility |
|---|---|
| `xHal_Rpi5CarGrayscaleModule.h` | Raw grayscale acquisition and classification API |
| `xHal_Rpi5CarLineTracker.h` | Calibrated line-tracking API and numeric contracts |
| `xHal_Rpi5CarLineTrackerTypes.h` | Fixed arrays and linear calibration structure |
| `xHal_Rpi5CarGrayscaleModule.cpp` | Raw sampling, references, and black/white status |
| `xHal_Rpi5CarLineTrackerCalibration.cpp` | Linear calibration and adaptive references |
| `xHal_Rpi5CarLineTrackerLifecycle.cpp` | ADC binding, validation, and conversion helpers |
| `xHal_Rpi5CarLineTrackerReading.cpp` | Calibrated sampling, detection, and position |
| `xHal_Rpi5CarLineTrackerTest.cpp` | Simulated ADC behavior and validation coverage |
| `xHal_Rpi5CarLineTrackerTestSupport.cpp` | Named in-memory I2C/ADC test bus |

## Composition

The application creates the I2C interface and three ADC channels before constructing either consumer:

```cpp
XWalkAdc left(i2c, 0U);
XWalkAdc middle(i2c, 1U);
XWalkAdc right(i2c, 2U);
XWalkGrayscaleModule grayscale(left, middle, right);
XWalkLineTracker tracker(left, middle, right);
```

Both classes store bounded non-owning ADC pointers. The three ADC objects and their I2C dependency must
outlive the grayscale module and line tracker. Neither class creates, owns, or releases hardware objects.

## Ported behavior

- Left, middle, and right channel order
- Default grayscale references of 1,000 ADC counts
- Black status at or below the reference and white status above it
- Identity slope and zero offset calibration by default
- Calibrated readings rounded to signed counts
- Cliff detection when any value is below the configured threshold
- Line detection when the channel spread is greater than 200 counts and no cliff exists
- Weighted line position from -1.0 at the left to 1.0 at the right
- Adaptive background and dark-line references using a five-percent update rate
- Light/dark calibration aligned to the channel with the largest light sample

Fixed-size arrays prevent incorrectly sized channel data. Non-finite coefficients, invalid light/dark
ranges, invalid channel indices, and calibrated signed-count overflow are rejected with project exceptions.
The calibrated output constraint is implemented as a protected member so conversion remains local to the
tracker contract.

## Host build and tests

```bash
cmake -S xWalkHal/sensor/xWalkLineTracker -B xWalkHal/sensor/xWalkLineTracker/build-host -DXWALK_LINE_TRACKER_BUILD_HOST_TESTS=ON
cmake --build xWalkLineTracker/build-host --parallel
ctest --test-dir xWalkLineTracker/build-host --output-on-failure
```

The host tests use callback-driven I2C samples and do not access physical hardware.

Reusable callback state lives in `xwalk::hal::test::linetracker`. The host suite
also verifies persistent trace-selector behavior.

## Standalone host simulation and tracing

```bash
cmake -S xWalkHal/sensor/xWalkLineTracker/simulation -B build-line-tracker-simulation -DCMAKE_BUILD_TYPE=Debug
cmake --build build-line-tracker-simulation --parallel
./build-line-tracker-simulation/xWalkLineTrackerSimulation --trace RPI.enable
```

The simulation composes real I2C, ADC, grayscale, and tracker interfaces with
an in-memory bus. Trace changes persist in generated XML and load automatically
on the next run. Enabled messages appear in the terminal and
`build-line-tracker-simulation/log/xWalkLineTrackerSimulation.log`.

## Hardware compilation without execution

```bash
cmake -S xWalkHal/sensor/xWalkLineTracker -B xWalkHal/sensor/xWalkLineTracker/build-rpi -DXWALK_LINE_TRACKER_BUILD_HARDWARE_TESTS=ON
cmake --build xWalkLineTracker/build-rpi --parallel
ctest --test-dir xWalkLineTracker/build-rpi -N -L hardware
```

This compiles the line tracker and its ADC and Linux I2C dependencies. It does not register an automatic
line-tracker hardware test because safe analog thresholds and sensor placement are application-specific.
