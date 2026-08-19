/******************************************************************************
 * @file        xVoiceActiveCarGptSequenceTest.cpp
 * @brief       Verifies the example-21 CLI sequence through simulated HAL.
 *
 * @details
 * Exercises Jarvis wake recognition, wake speech, one image-enabled model
 * round, response action parsing, speech output, and fail-safe cleanup.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains the device-free example-21 scenario.
 */
namespace
{

    /**
     * @brief Verifies wake, prompt, action, speech, and cleanup ordering.
     * @param[in,out] context Complete in-memory Controller-to-HAL composition.
     */
    void testVoiceActiveCarGpt(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->operationQueries = 0U;
        context.state->operationQueryLimit = 4U;
        context.state->recognitionTranscripts = {
            "HEY JARVIS", "What is two plus two?", "HEY JARVIS", "Honk and play background music"};
        context.state->modelResponses = {
            "Four.\nACTIONS: stop",
            "Right away, captain!\nACTIONS: honking, play background music, stop background music, stop"};
        xwalk::agent::test::XWalkControllerSequence sequence(*context.voiceActiveGptController);
        const ctrl::int32 result = sequence.run({{"voice-active-car-gpt", "start"}, {"voice-active-car-gpt", "stop"}});
        assert(result == 0);
        assert(context.state->recognitionTranscriptIndex == 4U);
        assert(context.state->modelPrompts ==
               ctrl::stringvector({"What is two plus two?", "Honk and play background music"}));
        assert(context.state->spokenText == ctrl::stringvector({"Hi, I'm Jarvis. Wake me up with: hey jarvis",
                                                                "Systems online. Ready when you are, Joxy.",
                                                                "Four.",
                                                                "Systems online. Ready when you are, Joxy.",
                                                                "Right away, captain!"}));
        assert(context.state->musicSoundFile.find("car-double-horn.wav") != ctrl::string::npos);
        assert(context.state->backgroundMusicFile.find("slow-trail-Ahjay_Stelino.mp3") != ctrl::string::npos);
        assert(std::find(context.state->eventLog.begin(), context.state->eventLog.end(), "hal.music.control") !=
               context.state->eventLog.end());
        assert(context.state->recognitionStopCount > 0U);
        assert(context.motors->left().speed() == 0.0);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the example-21 controller-to-HAL host sequence.
 * @param[in] argc Process argument count.
 * @param[in,out] argv Process argument vector.
 * @return Zero after all assertions pass; one for invalid runner arguments.
 */
int xWalkVoiceActiveCarGptCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testVoiceActiveCarGpt);
}
