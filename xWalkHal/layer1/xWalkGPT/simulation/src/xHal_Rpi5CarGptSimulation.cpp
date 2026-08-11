/******************************************************************************
 * @file        xHal_Rpi5CarGptSimulation.cpp
 * @brief       Implements the device-free xWalkGPT simulation.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarGptSimulation.h"
#include "xHal_Rpi5CarGptHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim {
int32 runGptSimulation() {
  XWalkGptHostStub backend;
  XWalkGptHostStub resetBackend;
  const XWalkGpioCallbacks callbacks = XWalkGptHostStub::gpioCallbacks();
  XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
  XWalkGpio speakerGpio(&backend, callbacks,
                        XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
  XWalkI2c i2c(&backend, &XWalkGptHostStub::probeI2c,
               &XWalkGptHostStub::writeI2c, &XWalkGptHostStub::readI2c);
  XWalkAdc adc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
               XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkBoardControl control(resetGpio, speakerGpio, adc, &backend,
                            &XWalkGptHostStub::primeSpeaker);
  string microphoneResult;
  string fileResult;
  {
    XWalkSpeechToText recognition(&backend,
                                  XWalkGptHostStub::recognitionCallbacks());
    microphoneResult = recognition.listen(100U);
    fileResult = recognition.transcribeFile("simulation.wav");
    recognition.stop();
  }
  XWalkTextToSpeech speech(control, &backend, &XWalkGptHostStub::speak);
  speech.speak("device-free speech");
  const boolean valid = (microphoneResult == "simulated recognition") &&
                        (fileResult == "simulated transcription") &&
                        backend.gpioValue() && (backend.primeCount() == 1U) &&
                        (backend.spokenCount() == 1U) &&
                        (backend.stopCount() == 2U);
  XWALK_HAL_TRACE_UID0(RPI .363, "xWalkGPT host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
