/******************************************************************************
 * @file        xGptCarSequenceTest.cpp
 * @brief       Verifies the upstream GPT-car CLI sequence through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

namespace
{

/**
 * @brief Verifies typed input, disabled images, JSON response, and cleanup.
 * @param[in,out] context Complete in-memory Controller-to-HAL composition.
 */
void testGptCar(xwalk::agent::test::ControllerCommandTestContext& context)
{
    context.state->operationQueries = 0U;
    context.state->operationQueryLimit = 1U;
    context.state->inputLines = {"Wave hello"};
    context.state->inputIndex = 0U;
    context.state->modelResponses = {
        R"({"actions":["stop"],"answer":"Hello, captain!"})"};
    xwalk::agent::test::XWalkControllerSequence sequence(
        *context.gptCarController);
    const ctrl::int32 result = sequence.run(
        {{"gpt-car", "start", "--keyboard", "--no-img"},
            {"gpt-car", "stop"}});
    assert(result == 0);
    assert(context.state->inputIndex == 1U);
    assert(context.state->modelPrompts ==
        ctrl::stringvector({"Wave hello"}));
    assert(context.state->modelImagePaths == ctrl::stringvector({""}));
    assert(context.state->spokenText ==
        ctrl::stringvector({"Hello, captain!"}));
    assert(context.motors->left().speed() == 0.0);
}

} /* namespace */

/**
 * @brief Runs the upstream GPT-car controller-to-HAL host sequence.
 * @param[in] argc Process argument count.
 * @param[in,out] argv Process argument vector.
 * @return Zero after all assertions pass; one for invalid runner arguments.
 */
int xWalkGptCarCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(
        argc, argv, &testGptCar);
}
