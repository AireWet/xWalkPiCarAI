/******************************************************************************
 * @file        xKeyboardControlSequenceTest.cpp
 * @brief       Verifies keyboard-control command sequencing through simulated HAL.
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

    /** @brief Verifies every upstream key, invalid-key handling, and centered cleanup. */
    void testKeyboardControl(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->inputLines = {"w", "a", "s", "d", "i", "k", "j", "l", "invalid", "q"};
        const ctrl::uint32 writes = context.state->i2cWriteCount;
        xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
        assert(sequence.run({{"keyboard-control"}}) == 0);
        assert(context.state->i2cWriteCount > writes);
        assert(context.state->delays.size() == 201U);

        assert(context.picarx->directionAngleDegrees() == 0.0);
        assert(context.motors->left().speed() == 0.0);
        assert(context.motors->right().speed() == 0.0);
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"controller.input",
                                                          "hal.i2c.write",
                                                          "controller.continue",
                                                          "controller.delay",
                                                          "hal.i2c.write",
                                                          "controller.input"}));
    }

} /* namespace */

/**
 * @brief Runs the keyboard-control controller-to-HAL host sequence.
 * @param[in] argc Must be two.
 * @param[in] argv Test name and writable configuration path.
 * @return Zero after successful assertions or one for invalid arguments.
 */
int xWalkKeyboardControlCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testKeyboardControl);
}
