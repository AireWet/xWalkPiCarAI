/******************************************************************************
 * @file        xHal_Rpi5CarLayer1GroupTestSupport.h
 * @brief       Declares reusable Layer 1 group-test support.
 * @project     xWalk Firmware
 * @module      xWalk Layer 1 Group Test
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_LAYER1_GROUP_TEST_SUPPORT_H
#define XHAL_RPI5CAR_LAYER1_GROUP_TEST_SUPPORT_H

#include "xHal_Rpi5CarGptTestSupport.h"
#include "xHal_Rpi5CarLanguageModelTestSupport.h"
#include "xHal_Rpi5CarVoiceAssistantTestSupport.h"

namespace xwalk::hal::test::layer1
{

    /** @brief Owns one complete in-memory VoiceAssistant dependency graph. */
    struct VoicePipelineFixture
    {
            gpt::TestGpioBackend resetBackend{};
            gpt::TestGpioBackend speakerBackend{};
            gpt::TestI2cBackend i2cBackend{};
            gpt::TestSpeakerPrime primeBackend{};
            gpt::TestRecognitionBackend recognitionBackend{};
            language_model::TestLanguageModelBackend modelBackend{};
            gpt::TestSpeechBackend outputBackend{};
            XWalkGpio resetGpio;
            XWalkGpio speakerGpio;
            XWalkI2c i2c;
            XWalkAdc batteryAdc;
            XWalkBoardControl boardControl;
            XWalkSpeechToText speechToText;
            XWalkLanguageModel languageModel;
            XWalkTextToSpeech textToSpeech;
            XWalkVoiceAssistant assistant;

            /** @brief Constructs dependencies in their required lifetime order. */
            VoicePipelineFixture();
    };

} /* namespace xwalk::hal::test::layer1 */

#endif /* XHAL_RPI5CAR_LAYER1_GROUP_TEST_SUPPORT_H */
