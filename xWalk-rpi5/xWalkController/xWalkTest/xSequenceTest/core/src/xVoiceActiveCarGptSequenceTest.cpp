/******************************************************************************
 * @file        xVoiceActiveCarGptSequenceTest.cpp
 * @brief       Verifies the example-21 CLI sequence through simulated HAL.
 *
 * @details
 * Exercises one-time Jarvis wake recognition, text-only follow-up rounds,
 * explicit sleep, response actions, speech output, and fail-safe cleanup.
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
        context.state->operationQueryLimit = 22U;
        context.state->recognitionTranscripts = {"HEY JARVIS",
                                                 "What is two plus two?",
                                                 "Honk and play background music",
                                                 "Goodbye Jarvis",
                                                 "HEY JARVIS",
                                                 "round one",
                                                 "round two",
                                                 "round three",
                                                 "round four",
                                                 "round five",
                                                 "round six",
                                                 "round seven",
                                                 "round eight",
                                                 "round nine",
                                                 "round ten",
                                                 "must require wake",
                                                 "HEY JARVIS",
                                                 "",
                                                 "",
                                                 "",
                                                 "HEY JARVIS",
                                                 ""};
        context.state->recognitionDurationsMs.assign(context.state->recognitionTranscripts.size(), 0U);
        context.state->recognitionDurationsMs.back() = 30'000U;
        context.state->modelResponses = {
            "Four.\nACTIONS: stop",
            "Right away, captain!\nACTIONS: honking, play background music, stop background music, stop"};
        xwalk::agent::test::XWalkControllerSequence sequence(*context.voiceActiveGptController);
        const ctrl::int32 result = sequence.run({{"voice-active-car-gpt", "start"}, {"voice-active-car-gpt", "stop"}});
        assert(result == 0);
        assert(context.state->recognitionTranscriptIndex == 22U);
        const ctrl::stringvector expectedPrompts{"What is two plus two?",
                                                 "Honk and play background music",
                                                 "round one",
                                                 "round two",
                                                 "round three",
                                                 "round four",
                                                 "round five",
                                                 "round six",
                                                 "round seven",
                                                 "round eight",
                                                 "round nine",
                                                 "round ten"};
        assert(context.state->modelPrompts == expectedPrompts);
        assert(std::find(context.state->modelPrompts.begin(), context.state->modelPrompts.end(), "Goodbye Jarvis") ==
               context.state->modelPrompts.end());
        assert(std::find(context.state->modelPrompts.begin(), context.state->modelPrompts.end(), "must require wake") ==
               context.state->modelPrompts.end());
        assert(std::count(context.state->spokenText.begin(),
                          context.state->spokenText.end(),
                          "Systems online. Ready when you are, Joxy.") == 4);
        assert(std::find(context.state->spokenText.begin(),
                         context.state->spokenText.end(),
                         "Going to sleep. Say hey Jarvis when you need me, Joxy.") != context.state->spokenText.end());
        assert(context.state->modelImagePaths == ctrl::stringvector(expectedPrompts.size(), ""));
        assert(context.state->cameraCapturePaths.empty());
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
