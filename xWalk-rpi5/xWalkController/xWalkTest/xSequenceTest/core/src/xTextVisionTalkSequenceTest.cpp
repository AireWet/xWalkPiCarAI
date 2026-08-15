/******************************************************************************
 * @file        xTextVisionTalkSequenceTest.cpp
 * @brief       Verifies example-17 CLI sequencing through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

/** @brief Contains the deterministic text-vision-talk scenario. */
namespace
{

    /** @brief Verifies model configuration, image capture, prompting, exit, and stop. */
    void testTextVisionTalk(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->inputLines = {"What can you see?", "  EXIT  "};
        context.state->modelResponses = {"I can see the PiCar-X."};
        xwalk::agent::test::XWalkControllerSequence sequence(*context.textVisionTalkController);
        assert(sequence.run({{"text-vision-talk", "start"}, {"text-vision-talk", "stop"}}) == 0);
        assert(context.state->delays.size() == 1U);
        assert(context.state->delays[0U] == 2'000U);
        assert(context.state->cameraCapturePaths.size() == 1U);
        assert(context.state->cameraCapturePaths[0U] == "/tmp/llm-img.jpg");
        assert(context.state->cameraWidthPixels == 1'280U);
        assert(context.state->cameraHeightPixels == 720U);
        assert(context.state->modelPrompts.size() == 1U);
        assert(context.state->modelPrompts[0U] == "What can you see?");
        assert(context.state->modelImagePaths[0U] == "/tmp/llm-img.jpg");
        assert(context.state->outputLines[0U] == "Hello, I am a helpful assistant. How can I help you?");
        assert(context.state->outputLines[1U] == "I can see the PiCar-X.");

        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"hal.model.configure",
                                                          "hal.model.configure",
                                                          "controller.delay",
                                                          "controller.continue",
                                                          "controller.input",
                                                          "hal.camera.capture",
                                                          "hal.model.prompt",
                                                          "controller.continue",
                                                          "controller.input"}));
    }

} /* namespace */

/** @brief Runs the text-vision-talk controller-to-HAL host sequence. @return Zero on success. */
int xWalkTextVisionTalkCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testTextVisionTalk);
}
