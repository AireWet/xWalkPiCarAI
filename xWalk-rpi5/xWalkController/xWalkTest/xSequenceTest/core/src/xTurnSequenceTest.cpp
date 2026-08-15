/******************************************************************************
 * @file        xTurnSequenceTest.cpp
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

#include <algorithm>
#include <cassert>

namespace
{
    void testTurn(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
        assert(sequence.run({{"turn", "left", "--angle", "20"}, {"turn", "right", "--angle", "15"}}) == 0);
        assert(std::find(context.state->steeringAngles.begin(), context.state->steeringAngles.end(), -20.0) !=
               context.state->steeringAngles.end());
        assert(std::find(context.state->steeringAngles.begin(), context.state->steeringAngles.end(), 15.0) !=
               context.state->steeringAngles.end());
        assert(context.picarx->directionAngleDegrees() == 0.0);
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"controller.continue",
                                                          "hal.i2c.write",
                                                          "controller.delay",
                                                          "hal.i2c.write",
                                                          "controller.delay",
                                                          "hal.i2c.write",
                                                          "controller.delay",
                                                          "hal.i2c.write"}));
    }
} // namespace

/** @brief Runs the turn controller-to-HAL host sequence. @return Zero on success. */
int xWalkTurnCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testTurn);
}
