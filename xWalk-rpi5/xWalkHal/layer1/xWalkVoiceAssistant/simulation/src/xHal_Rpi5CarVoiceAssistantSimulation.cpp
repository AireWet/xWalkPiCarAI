#include "xHal_Rpi5CarVoiceAssistantSimulation.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarVoiceAssistantHostStub.h"
namespace xwalk::hal::sim {
int32 runVoiceAssistantSimulation() {
  XWalkVoiceAssistantHostStub backend;
  const XWalkGpioCallbacks gpio = XWalkVoiceAssistantHostStub::gpioCallbacks();
  XWalkGpio reset(&backend, gpio, "MCURST");
  XWalkGpio speaker(&backend, gpio,
                    XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN);
  XWalkI2c i2c(&backend, &XWalkVoiceAssistantHostStub::probeI2c,
               &XWalkVoiceAssistantHostStub::writeI2c,
               &XWalkVoiceAssistantHostStub::readI2c);
  XWalkAdc adc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL,
               XHAL_RPI5CAR_ADC_ADDRESS_1);
  XWalkBoardControl board(reset, speaker, adc, &backend,
                          &XWalkVoiceAssistantHostStub::primeSpeaker);
  XWalkSpeechToText recognition(
      &backend, XWalkVoiceAssistantHostStub::recognitionCallbacks());
  XWalkLanguageModel model(&backend,
                           XWalkVoiceAssistantHostStub::modelCallbacks());
  XWalkTextToSpeech speech(board, &backend,
                           &XWalkVoiceAssistantHostStub::speak);
  XWalkVoiceAssistant assistant(recognition, model, speech,
                                {"Answer briefly", ""});
  assistant.start();
  const string response = assistant.runRound(100U);
  assistant.stop();
  const boolean valid = (response == "simulated answer") && backend.valid();
  XWALK_HAL_TRACE_UID0(RPI .379,
                       "xWalkVoiceAssistant host simulation completed");
  return valid ? 0 : 1;
}
} /* namespace xwalk::hal::sim */
