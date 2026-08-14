/******************************************************************************
 * @file        xHal_Rpi5CarLineTrackerTest.cpp
 * @brief       Verifies grayscale and line-tracker behavior with simulated ADC
 *data.
 *
 * @details
 * Checks raw sensing, threshold status, linear calibration, cliff and line
 * detection, position estimation, adaptive references, and input validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkLineTracker Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGrayscaleModule.h"
#include "xHal_Rpi5CarLineTrackerSimulationArguments.h"
#include "xHal_Rpi5CarLineTrackerSimulationConfig.h"
#include "xHal_Rpi5CarLineTrackerTestSupport.h"
#include "xHal_Rpi5CarTrace.h"

#include "xHal_Rpi5CarLineTracker.h"
#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains simulated ADC state and host-test scenarios.
 */
namespace {
using xwalk::hal::test::linetracker::probe;
using xwalk::hal::test::linetracker::read;
using xwalk::hal::test::linetracker::TestBus;
using xwalk::hal::test::linetracker::writeRegister;

/**
 * @brief Verifies the complete grayscale and line-tracker behavior.
 */
void testLineTracking() {
  TestBus bus;
  XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
  XWalkHal::XWalkAdc left(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkHal::XWalkAdc middle(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkHal::XWalkAdc right(i2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);

  XWalkHal::XWalkGrayscaleModule grayscale(left, middle, right);
  const XWalkHal::linetrackervalues grayscaleData = grayscale.read();
  assert(grayscaleData == XWalkHal::linetrackervalues({1'200, 800, 1'000}));
  const XWalkHal::linetrackerstatus defaultStatus = grayscale.readStatus();
  assert(defaultStatus == XWalkHal::linetrackerstatus({0U, 1U, 1U}));
  grayscale.setReference({1'100, 700, 1'001});
  assert(grayscale.reference() ==
         XWalkHal::linetrackervalues({1'100, 700, 1'001}));
  const XWalkHal::linetrackerstatus updatedStatus = grayscale.readStatus();
  assert(updatedStatus == XWalkHal::linetrackerstatus({0U, 0U, 1U}));
  const XWalkHal::int32 middleValue = grayscale.readChannel(1U);
  assert(middleValue == 800);

  XWalkHal::XWalkLineTracker tracker(left, middle, right);
  const XWalkHal::linetrackervalues rawTrackerData = tracker.read(true);
  assert(rawTrackerData == XWalkHal::linetrackervalues({1'200, 800, 1'000}));
  assert(bus.readCount == 13U);
  const XWalkHal::XWalkLineCalibration manualCalibration{{1.0, 2.0, 0.5},
                                                         {0.0, 10.0, -10.0}};
  tracker.setCalibrationData(manualCalibration);
  assert(tracker.calibrateData({100, 200, 300}) ==
         XWalkHal::linetrackervalues({100, 410, 140}));

  assert(tracker.isOnCliff({119, 500, 500}));
  assert(!tracker.isOnCliff({120, 500, 500}));
  tracker.setCliffThreshold(150);
  assert(tracker.cliffThreshold() == 150);
  assert(tracker.isOnCliff({149, 500, 500}));
  tracker.setCliffThreshold(120);

  assert(tracker.isOnLine({500, 800, 500}));
  assert(!tracker.isOnLine({500, 700, 600}));
  assert(!tracker.isOnLine({119, 800, 500}));
  const XWalkHal::float64 leftPosition =
      tracker.getLinePosition({200, 1'000, 1'000});
  const XWalkHal::float64 rightPosition =
      tracker.getLinePosition({1'000, 1'000, 200});
  const XWalkHal::float64 absentPosition =
      tracker.getLinePosition({1'000, 1'000, 1'000});
  assert(leftPosition == -0.53);
  assert(rightPosition == 0.53);
  assert(absentPosition == 0.0);

  tracker.updateLineBackgroundReference({400, 600, 800});
  tracker.updateLineReference({400, 600, 800});
  assert(XHAL_ABSOLUTE_VALUE(tracker.lineBackgroundReference() - 990.0) <
         0.000001);
  assert(XHAL_ABSOLUTE_VALUE(tracker.lineReference() - 210.0) < 0.000001);

  const XWalkHal::XWalkLineCalibration derived =
      tracker.calibrate({1'000, 800, 900}, {100, 80, 90});
  assert(derived.slopes ==
         XWalkHal::linetrackercalibrationvalues({1.0, 1.25, 1.11}));
  assert(derived.offsets ==
         XWalkHal::linetrackercalibrationvalues({0.0, 0.0, 0.0}));
  assert(tracker.calibrationData().slopes == derived.slopes);
}

/**
 * @brief Verifies channel and calibration validation failures.
 */
void testValidation() {
  TestBus bus;
  XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
  XWalkHal::XWalkAdc left(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkHal::XWalkAdc middle(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkHal::XWalkAdc right(i2c, 2U, XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkHal::XWalkGrayscaleModule grayscale(left, middle, right);
  XWalkHal::XWalkLineTracker tracker(left, middle, right);

  xwalk::hal::test::expectFailure(
      [&]() { static_cast<void>(grayscale.readChannel(3U)); });

  xwalk::hal::test::expectFailure([&]() {
    static_cast<void>(tracker.calibrate({100, 90, 80}, {100, 20, 10}));
  });

  const XWalkHal::float64 infinity = XHAL_POSITIVE_INFINITY(XWalkHal::float64);
  const XWalkHal::XWalkLineCalibration invalidCalibration{{1.0, infinity, 1.0},
                                                          {0.0, 0.0, 0.0}};
  xwalk::hal::test::expectFailure(
      [&]() { tracker.setCalibrationData(invalidCalibration); });
}

/** @brief Verifies persistent LineTracker trace-selector behavior. */
void testTraceSelection() {
  char executable[] = "xWalkLineTrackerTest";
  char option[] = "--trace";
  char enableSelector[] = "RPI.240.enable";
  char disableSelector[] = "RPI.240.disable";
  char malformedSelector[] = "RPI.invalid.enable";
  XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
  xwalk::hal::sim::XWalkLineTrackerSimulationArguments enable(3,
                                                              enableArguments);
  assert(enable.valid());
  assert(enable.applyTraceUpdate());
  XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
  xwalk::hal::sim::XWalkLineTrackerSimulationArguments disable(
      3, disableArguments);
  assert(disable.valid());
  assert(disable.applyTraceUpdate());
  XWalkHal::charpointer malformedArguments[]{executable, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkLineTrackerSimulationArguments malformed(
      3, malformedArguments);
  assert(malformed.valid() == false);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every xWalk line-tracker host-test scenario.
 *
 * @return
 * Zero when every assertion passes.
 */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_LINE_TRACKER_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_LINE_TRACKER_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .243, "xWalkLineTracker host tests started");
  testLineTracking();
  testValidation();
  testTraceSelection();
  XWALK_HAL_TRACE_UID0(RPI .244, "xWalkLineTracker host tests completed");
  return 0;
}
