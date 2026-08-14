/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechTest.cpp
 * @brief       Verifies text-to-speech through named in-memory adapters.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Test
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarGptTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
namespace {
using namespace xwalk::hal;
using namespace xwalk::hal::test::gpt;
void testCompositionAndSpeech() {
  TestGpioBackend resetBackend;
  TestGpioBackend speakerBackend;
  TestI2cBackend i2cBackend;
  TestSpeakerPrime prime;
  TestSpeechBackend speech;
  const XWalkGpioCallbacks callbacks = gpioCallbacks();
  XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
  XWalkGpio speakerGpio(&speakerBackend, callbacks,
                        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
  XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
  XWalkAdc adc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
               XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkBoardControl control(resetGpio, speakerGpio, adc, &prime, &primeSpeaker);
  {
    XWalkTextToSpeech textToSpeech(control, &speech, &speakText);
    assert(speakerBackend.physicalValue);
    assert(prime.callCount == 1U);
    assert(prime.durationMs ==
           XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS);
    textToSpeech.speak("Robot ready");
    assert(speech.callCount == 1U);
    assert(speech.text == "Robot ready");
    textToSpeech.speak("");
    assert(speech.callCount == 2U);
    assert(speech.text.empty());
    speech.fail = true;
    xwalk::hal::test::expectFailure([&]() { textToSpeech.speak("failure"); });
  }
  assert(speakerBackend.physicalValue);
}
void testCallbackValidation() {
  TestGpioBackend resetBackend;
  TestGpioBackend speakerBackend;
  TestI2cBackend i2cBackend;
  TestSpeakerPrime prime;
  const XWalkGpioCallbacks callbacks = gpioCallbacks();
  XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
  XWalkGpio speakerGpio(&speakerBackend, callbacks,
                        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
  XWalkI2c i2c(&i2cBackend, &probeI2c, &writeI2c, &readI2c);
  XWalkAdc adc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
               XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkBoardControl control(resetGpio, speakerGpio, adc, &prime, &primeSpeaker);
  xwalk::hal::test::expectFailure([&]() {
    XWalkTextToSpeech textToSpeech(control, nullptr, nullptr);
    static_cast<void>(textToSpeech);
  });
  assert(!speakerBackend.physicalValue);
  assert(prime.callCount == 0U);
}
} /* namespace */
XWalkHal::int32 main() {
  xwalk::hal::XWalkTrace::configureGlobal(
      XWALK_GPT_SIMULATION_TRACE_CONFIG_PATH,
      XWALK_GPT_SIMULATION_TRACE_LOG_PATH);
  XWALK_HAL_TRACE_UID0(RPI .368, "xWalkTextToSpeech host tests started");
  testCompositionAndSpeech();
  testCallbackValidation();
  XWALK_HAL_TRACE_UID0(RPI .369, "xWalkTextToSpeech host tests completed");
  return 0;
}
