/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsaHardwareTest.cpp
 * @brief       Provides an opt-in bounded ALSA microphone capture test.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Hardware Test
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 * @copyright Copyright (c) 2026 Joxy John. All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include "xHal_Rpi5CarTrace.h"
/** @brief Contains the deterministic recognition sink for this test. */
namespace {
XWalkHal::boolean ready(XWalkHal::contextpointer context) {
  static_cast<void>(context);
  return true;
}

XWalkHal::string recognizePcm(XWalkHal::contextpointer context,
                              const XWalkHal::bytevector &pcm,
                              XWalkHal::uint32 rate, XWalkHal::uint8 channels) {
  static_cast<void>(context);
  const hal::boolean pcmRateChannelsInvalid = static_cast<hal::boolean>(
      pcm.empty() || rate != 16'000U || channels != 1U);
  if (pcmRateChannelsInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Speech hardware test received invalid microphone PCM");
  }
  return "microphone capture received";
}

XWalkHal::string recognizeFile(XWalkHal::contextpointer context,
                               XWalkHal::stringview path) {
  static_cast<void>(context);
  static_cast<void>(path);
  return {};
}

void cancel(XWalkHal::contextpointer context) { static_cast<void>(context); }
} /* namespace */

/**
 * @brief Captures 100 milliseconds from an explicitly selected microphone.
 * @param[in] argumentCount Exactly two arguments are required.
 * @param[in] argumentValues Program name and explicit ALSA capture device.
 * @return Zero after bounded PCM reaches the recognition sink.
 * @warning Run only after confirming the intended microphone device and privacy
 * context.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
                     XWalkHal::charpointer argumentValues[]) {
  if (argumentCount != 2) {
    XWALK_HAL_ERROR(
        XWALK_INVAL,
        "Speech hardware test requires an explicit ALSA microphone");
  }
  XWalkHal::XWalkSpeechToTextAlsaOperations recognizer{};
  recognizer.recognizerReady = &ready;
  recognizer.recognizePcm = &recognizePcm;
  recognizer.recognizeFile = &recognizeFile;
  recognizer.cancelRecognition = &cancel;
  XWalkHal::XWalkSpeechToTextAlsa adapter(argumentValues[1], nullptr,
                                          recognizer);
  XWalkHal::XWalkSpeechToText speech(&adapter, adapter.callbacks());
  const hal::boolean speechIsReadyListenInvalid = static_cast<hal::boolean>(
      !speech.isReady() || speech.listen(100U).empty());
  if (speechIsReadyListenInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Speech hardware test did not receive microphone PCM");
  }
  return 0;
}
