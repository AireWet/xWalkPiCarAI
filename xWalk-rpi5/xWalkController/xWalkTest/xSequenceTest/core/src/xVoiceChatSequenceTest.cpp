/******************************************************************************
 * @file        xVoiceChatSequenceTest.cpp
 * @brief       Verifies one public CLI command sequence through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

namespace
{
    void testVoiceChat(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->operationQueryLimit = 2U;
        context.state->recognitionTranscripts = {{}, "hello robot"};
        context.state->modelResponses = {"<think>private</think> Hello there."};
        xwalk::agent::test::XWalkControllerSequence sequence(*context.voiceChatController);
        assert(sequence.run({{"voice-chat", "start"}, {"voice-chat", "stop"}}) == 0);
        assert(context.state->recognitionTranscriptIndex == 2U);
        assert(context.state->recognitionStopCount == 1U);
        assert(context.state->speakerPrimeCount == 1U);
        assert(context.state->modelPrompts == ctrl::stringvector{"hello robot"});
        assert(context.state->modelImagePaths == ctrl::stringvector{{}});
        assert(context.state->delays == ctrl::uint32vector({100U, 50U}));
        assert(
            context.state->spokenText ==
            ctrl::stringvector({xwalk::agent::XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME, "Hello there.", "Goodbye!"}));
        assert(context.state->outputLines.front() == xwalk::agent::XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME);
        assert(context.state->outputLines[1U] == "🎤 Listening... (Press Ctrl+C to stop)");
        assert(context.state->outputLines[2U] == "[INFO] Nothing recognized. Try again.");
        assert(context.state->outputLines[4U] == "[YOU] hello robot");
        assert(context.state->outputLines[5U] == "<think>private</think> Hello there.");
        assert(context.state->outputLines[6U] == "[INFO] Stopping...");
        assert(context.state->outputLines[7U] == "Bye.");

        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {
                                                             "hal.speech.speak",
                                                             "controller.continue",
                                                             "hal.speech.listen",
                                                             "controller.delay",
                                                             "controller.continue",
                                                             "hal.speech.listen",
                                                             "hal.model.prompt",
                                                             "hal.speech.speak",
                                                             "controller.delay",
                                                             "controller.continue",
                                                             "hal.speech.speak",
                                                             "hal.speech.stop",
                                                         }));
    }
} // namespace

/** @brief Runs the voice-chat controller-to-HAL host sequence. @return Zero on success. */
int xWalkVoiceChatCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testVoiceChat);
}
