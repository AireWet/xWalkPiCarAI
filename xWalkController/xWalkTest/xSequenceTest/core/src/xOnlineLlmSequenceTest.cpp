/******************************************************************************
 * @file        xOnlineLlmSequenceTest.cpp
 * @brief       Verifies example-18 CLI sequencing through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

/** @brief Contains the deterministic online-LLM scenario. */
namespace
{

/** @brief Verifies setup, repeated text-only prompting, cancellation, and stop. */
void testOnlineLlm(
    xwalk::agent::test::ControllerCommandTestContext& context)
{
    context.state->inputLines = {"Hello", "How are you?"};
    context.state->modelResponses = {"Hello from OpenAI", "I am ready"};
    context.state->operationQueryLimit = 2U;
    xwalk::agent::test::XWalkControllerSequence sequence(
        *context.onlineLlmTestController);
    assert(sequence.run({{"online-llm-test", "start"},
        {"online-llm-test", "stop"}}) == 0);
    assert(context.state->modelPrompts ==
        ctrl::stringvector({"Hello", "How are you?"}));
    assert(context.state->modelImagePaths ==
        ctrl::stringvector({"", ""}));
    assert(context.state->outputLines[0U] ==
        "Hello, I am a helpful assistant. How can I help you?");
    assert(context.state->outputLines[1U] == "Hello from OpenAI");
    assert(context.state->outputLines[2U] == "I am ready");

    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"hal.model.configure", "hal.model.configure",
            "controller.continue", "controller.input", "hal.model.prompt",
            "controller.continue", "controller.input",
            "hal.model.prompt", "controller.continue",
            }));
}

} /* namespace */

/** @brief Runs the online-LLM controller-to-HAL host sequence. @return Zero on success. */
int xWalkOnlineLlmTestCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv,
        &testOnlineLlm);
}
