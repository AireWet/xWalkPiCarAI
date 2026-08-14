/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicTest.cpp
 * @brief       Verifies ultrasonic behavior using simulated GPIO callbacks.
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUltrasonicSimulationArguments.h"
#include "xHal_Rpi5CarUltrasonicSimulationConfig.h"
#include "xHal_Rpi5CarUltrasonicTestSupport.h"
#include <cassert>
namespace {
using xwalk::hal::test::ultrasonic::callbacks;
using xwalk::hal::test::ultrasonic::EchoBehavior;
using xwalk::hal::test::ultrasonic::TestBackend;

/** @brief Verifies configuration, trigger sequencing, and distance conversion.
 */
void testDistanceMeasurement() {
  TestBackend backend;
  backend.pulseDelayMicroseconds = 100U;
  const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
  XWalkHal::XWalkGpio trigger(&backend, callbackSet, "D2");
  XWalkHal::XWalkGpio echo(&backend, callbackSet, "D3");
  XWalkHal::XWalkUltrasonic ultrasonic(trigger, echo);
  assert(backend.triggerMode == XWalkHal::XWalkGpioMode::Output);
  assert(backend.echoMode == XWalkHal::XWalkGpioMode::Input);
  assert(backend.echoPull == XWalkHal::XWalkGpioPull::Down);
  assert(ultrasonic.timeoutMicroseconds() == 20'000U);
  const XWalkHal::float64 distanceCentimeters = ultrasonic.read(1U);
  const XWalkHal::float64 maximumTimeoutDistanceCentimeters = 350.0;
  assert(distanceCentimeters > 1.0);
  assert(distanceCentimeters < maximumTimeoutDistanceCentimeters);
  assert(backend.triggerLevels == XWalkHal::bytevector({0U, 1U, 0U}));
}

/** @brief Verifies timeout-only retry and invalid-pulse behavior. */
void testRetryAndStatusResults() {
  TestBackend backend;
  backend.behavior = EchoBehavior::TimeoutThenInvalid;
  const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
  XWalkHal::XWalkGpio trigger(&backend, callbackSet, "D2");
  XWalkHal::XWalkGpio echo(&backend, callbackSet, "D3");
  XWalkHal::XWalkUltrasonic ultrasonic(trigger, echo, 5'000U);
  const XWalkHal::float64 distanceCentimeters = ultrasonic.read(2U);
  assert(distanceCentimeters ==
         XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM);
  assert(backend.triggerCount == 2U);
  backend.behavior = EchoBehavior::Invalid;
  backend.triggerCount = 0U;
  assert(ultrasonic.read(3U) ==
         XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM);
  assert(backend.triggerCount == 1U);
}

/** @brief Verifies complete timeout and zero-attempt behavior. */
void testTimeoutResults() {
  TestBackend backend;
  backend.behavior = EchoBehavior::Timeout;
  const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
  XWalkHal::XWalkGpio trigger(&backend, callbackSet, "D2");
  XWalkHal::XWalkGpio echo(&backend, callbackSet, "D3");
  XWalkHal::XWalkUltrasonic ultrasonic(trigger, echo, 200U);
  assert(ultrasonic.read(2U) == XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM);
  assert(backend.triggerCount == 2U);
  backend.triggerCount = 0U;
  assert(ultrasonic.read(0U) == XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM);
  assert(backend.triggerCount == 0U);
  ultrasonic.close();
}

/** @brief Verifies persistent ultrasonic trace-selector behavior. */
void testTraceSelection() {
  char executable[] = "xWalkUltrasonicTest";
  char option[] = "--trace";
  char enableSelector[] = "RPI.207.enable";
  char disableSelector[] = "RPI.207.disable";
  char malformedSelector[] = "RPI.invalid.enable";
  XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
  xwalk::hal::sim::XWalkUltrasonicSimulationArguments enable(3,
                                                             enableArguments);
  assert(enable.valid());
  assert(enable.applyTraceUpdate());
  XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
  xwalk::hal::sim::XWalkUltrasonicSimulationArguments disable(3,
                                                              disableArguments);
  assert(disable.valid());
  assert(disable.applyTraceUpdate());
  XWalkHal::charpointer malformedArguments[]{executable, option,
                                             malformedSelector};
  const xwalk::hal::sim::XWalkUltrasonicSimulationArguments malformed(
      3, malformedArguments);
  assert(malformed.valid() == false);
}
} /* namespace */

/** @brief Runs every xWalk ultrasonic host-test scenario. */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_ULTRASONIC_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_ULTRASONIC_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .210, "xWalkUltrasonic host tests started");
  testDistanceMeasurement();
  testRetryAndStatusResults();
  testTimeoutResults();
  testTraceSelection();
  XWALK_HAL_TRACE_UID0(RPI .211, "xWalkUltrasonic host tests completed");
  return 0;
}
