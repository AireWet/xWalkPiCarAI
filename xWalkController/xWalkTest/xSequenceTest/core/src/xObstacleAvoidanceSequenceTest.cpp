/******************************************************************************
 * @file        xObstacleAvoidanceSequenceTest.cpp
 * @brief       Verifies obstacle-avoidance command sequencing through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerCommands.h"
#include "xControllerSequence.h"

#include <cassert>

namespace
{

/** @brief Verifies explicit stop and failed-sensor foreground cleanup. */
void testObstacleAvoidance(
    xwalk::agent::test::ControllerCommandTestContext& context)
{
    xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
    assert(sequence.run({{"avoid-obstacles", "stop"}}) == 0);
    assert(context.state->outputLines.back() == "Obstacle avoidance stopped.");
    assert(xwalk::ctrl::XWALK_runControllerCommand(*context.controller, {"avoid-obstacles", "start"}) == 2);
    assert(context.state->outputLines.back() ==
        "Obstacle avoidance stopped: invalid ultrasonic sample.");
    assert(context.motors->left().speed() == 0.0);
    assert(context.motors->right().speed() == 0.0);
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"hal.i2c.write", "controller.output", "hal.i2c.write",
            "controller.output", "hal.i2c.write", "controller.output"}));
}

} /* namespace */

/**
 * @brief Runs the obstacle-avoidance controller-to-HAL host sequence.
 * @param[in] argc Must be two.
 * @param[in] argv Test name and writable configuration path.
 * @return Zero after successful assertions or one for invalid arguments.
 */
int xWalkObstacleAvoidanceCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(
        argc, argv, &testObstacleAvoidance);
}
