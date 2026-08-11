/******************************************************************************
 * @file        xMoveSequenceTest.cpp
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
void testMove(xwalk::agent::test::ControllerCommandTestContext& context)
{
    const ctrl::uint32 writes = context.state->i2cWriteCount;
    xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
    assert(sequence.run({{"move", "forward", "--speed", "40", "--duration", "0.04"},
        {"move", "backward", "--speed", "30", "--duration", "0.04"},
        {"move", "demo"}}) == 0);
    assert(context.state->i2cWriteCount > writes);
    assert(context.state->delays.size() == 509U);
    assert(context.state->leftSpeeds.front() == 70.0);
    assert(context.state->leftSpeeds[2U] != 0.0);

    assert(context.picarx->directionAngleDegrees() == -1.0);
    assert(context.motors->left().speed() == 0.0);
    assert(context.motors->right().speed() == 0.0);
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"controller.continue", "hal.i2c.write", "controller.continue",
            "controller.delay", "hal.i2c.write", "controller.continue",
            "hal.i2c.write", "controller.delay", "hal.i2c.write"}));
}
}

/** @brief Runs the move controller-to-HAL host sequence. @return Zero on success. */
int xWalkMoveCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testMove);
}
