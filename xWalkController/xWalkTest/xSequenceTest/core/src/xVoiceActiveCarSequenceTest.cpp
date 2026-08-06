/******************************************************************************
 * @file        xVoiceActiveCarSequenceTest.cpp
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
#include "xHal_Rpi5CarFileFunctions.h"

#include <cassert>

namespace
{
void testVoiceActiveCar(xwalk::agent::test::ControllerCommandTestContext& context)
{
    context.state->operationQueryLimit = 10U;
    context.state->recognitionTranscripts = {"hey rolly", "tell me a joke"};
    context.state->modelResponses = {"Wheels ready!\nACTIONS: stop"};
    const ctrl::uint32 writes = context.state->i2cWriteCount;
    xwalk::agent::test::XWalkControllerSequence sequence(*context.voiceActiveController);
    assert(sequence.run({{"voice-active-car", "start"},
        {"voice-active-car", "stop"}}) == 0);
    assert(context.state->recognitionStopCount > 0U);
    assert(context.state->recognitionTranscriptIndex == 2U);
    assert(context.state->modelPrompts.size() == 1U);
    assert(context.state->modelPrompts.front() == "tell me a joke");
    const ctrl::filesystempath configuredImage =
        ctrl::filesystempath(XWALK_VOICE_ACTIVE_CAR_CONFIG_DIRECTORY) /
        "voice-active-car.jpg";
    assert(context.state->modelImagePaths.front() == configuredImage.string());
    assert(xwalk::hal::isReadableRegularFile(configuredImage));
    assert(context.state->spokenText.size() == 3U);
    assert(context.state->spokenText[0U] ==
        "Hi, I'm Rolly. Wake me up with: hey rolly");
    assert(context.state->spokenText[1U] == "Hi there");
    assert(context.state->spokenText[2U] == "Wheels ready!");
    assert(context.state->i2cWriteCount > writes);
    assert(context.motors->left().speed() == 0.0);
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"controller.continue", "hal.speech.listen", "hal.speech.speak",
            "controller.continue", "hal.speech.listen", "hal.camera.capture",
            "hal.model.prompt", "hal.speech.speak", "hal.speech.stop",
            "controller.output"}));
}
}

/** @brief Runs the voice-active-car controller-to-HAL host sequence. @return Zero on success. */
int xWalkVoiceActiveCarCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testVoiceActiveCar);
}
