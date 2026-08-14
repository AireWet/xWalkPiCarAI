/******************************************************************************
 * @file        xHal_Rpi5CarServoTestMain.cpp
 * @brief       Provides selection and execution for Servo host tests.
 *
 * @details
 * Runs every Servo scenario or dispatches one named scenario for CTest.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo Host Test
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

#include "xHal_Rpi5CarServoTestFunctions.h"

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarServoSimulationArguments.h"
#include "xHal_Rpi5CarServoSimulationConfig.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test {

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains test-runner functions private to this translation unit. */
namespace {

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Runs every registered Servo host-test scenario in sequence. */
void runAllTests() {
  testServoInitialization();
  testServoAngles();
  testServoPulseWidths();
  testServoValidation();
  testServoTraceSelection();
}

/**
 * @brief Dispatches one Servo host-test scenario by name.
 *
 * @param[in] testName
 * Selector matching `initialization`, `angle`, `pulse`, or `validation`.
 *
 * @return
 * Zero for a recognized passing scenario; otherwise one.
 */
int32 runSelectedTest(stringview testName) {
  if (testName == "initialization") {
    testServoInitialization();
  } else if (testName == "angle") {
    testServoAngles();
  } else if (testName == "pulse") {
    testServoPulseWidths();
  } else if (testName == "validation") {
    testServoValidation();
  } else if (testName == "trace") {
    testServoTraceSelection();
  } else {
    return 1;
  }
  return 0;
}

} /* namespace */
} /* namespace xwalk::hal::test */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all Servo host tests or one selected scenario.
 *
 * @param[in] argumentCount
 * Number of command-line arguments, including the executable name.
 *
 * @param[in] argumentValues
 * Non-owning array of command-line string pointers valid for this call.
 *
 * @return
 * Zero when requested assertions pass; otherwise one for invalid usage.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_SERVO_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_SERVO_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .190, "xWalkServo host tests started");
  XWalkHal::int32 result = 0;
  if (argumentCount == 1) {
    xwalk::hal::test::runAllTests();
  } else if (argumentCount == 2) {
    result = xwalk::hal::test::runSelectedTest(argumentValues[1]);
  } else if (argumentCount == 3) {
    const xwalk::hal::sim::XWalkServoSimulationArguments arguments(
        argumentCount, argumentValues);
    if ((arguments.valid() == false) ||
        (arguments.applyTraceUpdate() == false)) {
      result = 1;
    } else {
      xwalk::hal::test::runAllTests();
    }
  } else {
    result = 1;
  }
  XWALK_HAL_TRACE_UID1(
      RPI .191, "xWalkServo host tests completed with status %d", result);
  return result;
}
