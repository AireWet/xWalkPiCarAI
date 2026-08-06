/******************************************************************************
 * @file        xVoiceControlledCarSequenceTest.cpp
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
void testVoiceControlledCar(xwalk::agent::test::ControllerCommandTestContext& context)
{
    context.state->recognitionTranscripts = {"background noise", "HEY ROBOT",
        "", "go forward", "backward now", "turn left", "turn right",
        "dance", "sleep now"};
    context.state->operationQueryLimit = 209U;
    xwalk::agent::test::XWalkControllerSequence sequence(
        *context.voiceControlledController);
    assert(sequence.run({{"voice-controlled-car", "start"},
        {"voice-controlled-car", "stop"}}) == 0);
    assert(context.state->recognitionTranscriptIndex == 9U);
    assert(context.state->delays.size() == 200U);
    for (const ctrl::uint32 durationMs : context.state->delays)
    {
        assert(durationMs == 20U);
    }
    assert(context.state->leftSpeeds[0U] > 0.0);
    assert(context.state->rightSpeeds[0U] > 0.0);
    assert(context.state->leftSpeeds[50U] < 0.0);
    assert(context.state->rightSpeeds[50U] < 0.0);
    assert(context.state->steeringAngles[0U] == 0.0);
    assert(context.state->steeringAngles[50U] == 0.0);
    assert(context.state->steeringAngles[100U] == -25.0);
    assert(context.state->steeringAngles[150U] == 25.0);
    assert(context.state->recognitionStopCount >= 2U);
    assert(context.motors->left().speed() == 0.0);
    assert(context.picarx->directionAngleDegrees() == 0.0);
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"controller.output", "controller.continue", "hal.speech.listen",
            "controller.continue", "hal.speech.listen", "controller.output",
            "controller.continue", "hal.speech.listen", "controller.continue",
            "hal.speech.listen", "controller.output", "hal.i2c.write",
            "controller.delay", "hal.i2c.write", "hal.speech.stop",
            "hal.i2c.write", "controller.output"}));
}
}

/** @brief Runs the voice-controlled-car controller-to-HAL host sequence. @return Zero on success. */
int xWalkVoiceControlledCarCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv,
        &testVoiceControlledCar);
}
