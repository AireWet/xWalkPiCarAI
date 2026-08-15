/******************************************************************************
 * @file        xVoicePromptCarSequenceTest.cpp
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
    void testVoicePromptCar(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->operationQueryLimit = 4U;
        xwalk::agent::test::XWalkControllerSequence sequence(*context.voicePromptController);
        assert(sequence.run({{"voice-prompt-car", "start"}, {"voice-prompt-car", "stop"}}) == 0);
        assert(context.state->spokenText ==
               ctrl::stringvector(
                   {"Hello! I'm PiCar-X.", "Moving forward", "Moving backward", "Turning left", "Turning right"}));
        assert(context.state->delays == ctrl::uint32vector({2'000U, 2'000U, 2'000U, 2'000U}));
        assert(context.state->leftSpeeds.size() == 4U);
        assert(context.state->leftSpeeds[0U] > 0.0);
        assert(context.state->leftSpeeds[1U] < 0.0);
        assert(context.state->leftSpeeds[2U] > 0.0);
        assert(context.state->leftSpeeds[3U] > 0.0);
        assert(context.state->steeringAngles == ctrl::float64vector({0.0, 0.0, -20.0, 20.0}));
        assert(context.motors->left().speed() == 0.0);
        assert(context.picarx->directionAngleDegrees() == 0.0);
        assert(xwalk::agent::test::containsOrderedEvents(
            context.state->eventLog, {"hal.speech.speak", "controller.continue", "hal.i2c.write", "hal.i2c.write"}));
    }
} // namespace

/** @brief Runs the voice-prompt-car controller-to-HAL host sequence. @return Zero on success. */
int xWalkVoicePromptCarCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testVoicePromptCar);
}
