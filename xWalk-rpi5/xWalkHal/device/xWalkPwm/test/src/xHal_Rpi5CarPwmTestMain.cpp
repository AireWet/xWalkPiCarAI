/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestMain.cpp
 * @brief       Provides selection and execution for PWM host-test scenarios.
 *
 * @details
 * Runs all PWM assertions when invoked without an argument or dispatches one
 * named scenario for individual CTest registration.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
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

#include "xHal_Rpi5CarPwmTestFunctions.h"

#include "xHal_Rpi5CarCommon.h"
#include "xHal_Rpi5CarPwmSimulationArguments.h"
#include "xHal_Rpi5CarPwmSimulationConfig.h"
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

/**
 * @brief Contains test-runner functions private to this translation unit.
 */
namespace {

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Runs every registered PWM host-test scenario in sequence. */
void runAllTests() {
  xwalk::hal::test::testAddressAndTimerSelection();
  xwalk::hal::test::testAllTimerMappings();
  xwalk::hal::test::testBigEndianRegisterData();
  xwalk::hal::test::testSharedPeriodAndPercentage();
  xwalk::hal::test::testDefaultFrequency();
  xwalk::hal::test::testValidation();
  xwalk::hal::test::testTraceSelection();
}

/**
 * @brief Dispatches one host-test scenario by its command-line name.
 *
 * @param[in] testName
 * Selector matching `address`, `mapping`, `register`, `percentage`,
 * `frequency`, or `validation`.
 *
 * @return
 * Zero when the selector is recognized and assertions pass; otherwise
 * one for an unknown selector.
 */
int32 runSelectedTest(stringview testName) {
  if (testName == "address") {
    testAddressAndTimerSelection();
  } else if (testName == "mapping") {
    testAllTimerMappings();
  } else if (testName == "register") {
    testBigEndianRegisterData();
  } else if (testName == "percentage") {
    testSharedPeriodAndPercentage();
  } else if (testName == "frequency") {
    testDefaultFrequency();
  } else if (testName == "validation") {
    testValidation();
  } else if (testName == "trace") {
    testTraceSelection();
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
 * @brief Runs all PWM host tests or one selected scenario.
 *
 * @param[in] argumentCount
 * Number of command-line arguments, including the executable name.
 *
 * @param[in] argumentValues
 * Non-owning array of command-line string pointers valid for this call.
 *
 * @return
 * Zero when the requested assertions pass; otherwise one for invalid command
 * usage or an unknown selector.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_PWM_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_PWM_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .172, "xWalkPwm host tests started");
  XWalkHal::int32 result = 0;
  if (argumentCount == 1) {
    xwalk::hal::test::runAllTests();
  } else if (argumentCount == 2) {
    const XWalkHal::stringview option(argumentValues[1]);
    if ((option == "--help") || (option == "-h")) {
      XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [test | --trace <selector>]",
                        argumentValues[0]);
    } else {
      result = xwalk::hal::test::runSelectedTest(option);
    }
  } else if (argumentCount == 3) {
    const xwalk::hal::sim::XWalkPwmSimulationArguments arguments(
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
  XWALK_HAL_TRACE_UID1(RPI .173, "xWalkPwm host tests completed with status %d",
                       result);
  return result;
}
