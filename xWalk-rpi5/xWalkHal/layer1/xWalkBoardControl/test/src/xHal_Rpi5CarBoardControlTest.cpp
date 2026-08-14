/******************************************************************************
 * @file        xHal_Rpi5CarBoardControlTest.cpp
 * @brief       Verifies board control through named in-memory test adapters.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlTestSupport.h"
#include "xHal_Rpi5CarBoardControlSimulationArguments.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
namespace {
using namespace xwalk::hal;
using namespace xwalk::hal::test::boardcontrol;
void testOperations() {
  TestGpioBackend resetBackend;
  TestGpioBackend speakerBackend;
  TestI2cBackend i2cBackend;
  TestSpeakerPrime primeState;
  const XWalkGpioCallbacks callbacks = gpioCallbacks();
  XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
  XWalkGpio speakerGpio(&speakerBackend, callbacks,
                        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
  XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
  XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
                      XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc, &primeState,
                            &primeSpeaker);
  control.setPin(resetGpio, true);
  assert(resetBackend.physicalValue);
  resetBackend.writeCount = 0U;
  control.resetMcu();
  assert(resetBackend.writeCount == 2U);
  assert(!resetBackend.writes[0U]);
  assert(resetBackend.writes[1U]);
  const float64 difference =
      XHAL_ABSOLUTE_VALUE(control.batteryVoltage() - 9.9);
  assert(difference < 0.000001);
  control.enableSpeaker();
  assert(speakerBackend.physicalValue);
  assert(primeState.callCount == 1U);
  assert(primeState.durationMs ==
         XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS);
  control.disableSpeaker();
  assert(!speakerBackend.physicalValue);
}
void testFailureAndValidation() {
  TestGpioBackend resetBackend;
  TestGpioBackend speakerBackend;
  TestGpioBackend wrongResetBackend;
  TestI2cBackend i2cBackend;
  TestSpeakerPrime primeState;
  const XWalkGpioCallbacks callbacks = gpioCallbacks();
  XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
  XWalkGpio wrongResetGpio(&wrongResetBackend, callbacks, "LED");
  XWalkGpio speakerGpio(&speakerBackend, callbacks,
                        XHAL_RPI5CAR_DEVICE_V5_SPEAKER_ENABLE_PIN);
  XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
  XWalkAdc batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
                      XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkBoardControl control(resetGpio, speakerGpio, batteryAdc, &primeState,
                            &primeSpeaker);
  primeState.fail = true;
  xwalk::hal::test::expectFailure([&]() { control.enableSpeaker(); });
  assert(!speakerBackend.physicalValue);
  xwalk::hal::test::expectFailure([&]() {
    XWalkBoardControl invalid(wrongResetGpio, speakerGpio, batteryAdc,
                              &primeState, &primeSpeaker);
    static_cast<void>(invalid);
  });
  xwalk::hal::test::expectFailure([&]() {
    XWalkBoardControl invalid(resetGpio, speakerGpio, batteryAdc, &primeState,
                              nullptr);
    static_cast<void>(invalid);
  });
}
/** @brief Verifies BoardControl simulation trace argument boundaries. */
void testSimulationArguments() {
  char binary[] = "xWalkBoardControlSimulation";
  char help[] = "--help";
  char shortHelp[] = "-h";
  char trace[] = "--trace";
  char enable[] = "RPI.enable";
  char disable[] = "all.disable";
  char json[] = "trace.json";
  char malformed[] = "RPI.Camera.enable";
  char unknown[] = "--verbose";
  charpointer defaults[]{binary};
  charpointer helpValues[]{binary, help};
  charpointer shortHelpValues[]{binary, shortHelp};
  charpointer enableValues[]{binary, trace, enable};
  charpointer disableValues[]{binary, trace, disable};
  charpointer jsonValues[]{binary, trace, json};
  charpointer malformedValues[]{binary, trace, malformed};
  charpointer unknownValues[]{binary, unknown, enable};
  const sim::XWalkBoardControlSimulationArguments defaultArguments(1, defaults);
  const sim::XWalkBoardControlSimulationArguments helpArguments(2, helpValues);
  const sim::XWalkBoardControlSimulationArguments shortHelpArguments(
      2, shortHelpValues);
  const sim::XWalkBoardControlSimulationArguments enableArguments(
      3, enableValues);
  const sim::XWalkBoardControlSimulationArguments disableArguments(
      3, disableValues);
  const sim::XWalkBoardControlSimulationArguments jsonArguments(3, jsonValues);
  const sim::XWalkBoardControlSimulationArguments malformedArguments(
      3, malformedValues);
  const sim::XWalkBoardControlSimulationArguments unknownArguments(
      3, unknownValues);
  const sim::XWalkBoardControlSimulationArguments nullArguments(3, nullptr);
  assert(defaultArguments.valid() && defaultArguments.applyTraceUpdate());
  assert(helpArguments.valid() && helpArguments.helpRequested());
  assert(shortHelpArguments.valid() && shortHelpArguments.helpRequested());
  assert(enableArguments.valid() && enableArguments.applyTraceUpdate());
  assert(disableArguments.valid() && disableArguments.applyTraceUpdate());
  assert(jsonArguments.valid());
  assert(!malformedArguments.valid());
  assert(!unknownArguments.valid());
  assert(!nullArguments.valid());
}
} /* namespace */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .333, "xWalkBoardControl operation tests started");
  testOperations();
  testFailureAndValidation();
  testSimulationArguments();
  XWALK_HAL_TRACE_UID0(RPI .334, "xWalkBoardControl operation tests completed");
  return 0;
}
