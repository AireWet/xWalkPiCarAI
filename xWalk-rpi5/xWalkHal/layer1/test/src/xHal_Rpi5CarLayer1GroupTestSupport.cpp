/******************************************************************************
 * @file        xHal_Rpi5CarLayer1GroupTestSupport.cpp
 * @brief       Implements reusable Layer 1 group-test support.
 * @project     xWalk Firmware
 * @module      xWalk Layer 1 Group Test
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarLayer1GroupTestSupport.h"

namespace xwalk::hal::test::layer1
{

    VoicePipelineFixture::VoicePipelineFixture()
        : resetGpio(&resetBackend, gpt::gpioCallbacks(), "MCURST"),
          speakerGpio(&speakerBackend, gpt::gpioCallbacks(), XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN),
          i2c(&i2cBackend, &gpt::probeI2c, &gpt::writeI2c, &gpt::readI2c),
          batteryAdc(i2c, XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL, XHAL_RPI5CAR_ADC_ADDRESS_1),
          boardControl(resetGpio, speakerGpio, batteryAdc, &primeBackend, &gpt::primeSpeaker),
          speechToText(&recognitionBackend, gpt::recognitionCallbacks()),
          languageModel(&modelBackend, language_model::backendCallbacks()),
          textToSpeech(boardControl, &outputBackend, &gpt::speakText),
          assistant(
              speechToText, languageModel, textToSpeech, XWalkVoiceAssistantConfiguration{"Answer briefly", "Ready"})
    {
    }

} /* namespace xwalk::hal::test::layer1 */
