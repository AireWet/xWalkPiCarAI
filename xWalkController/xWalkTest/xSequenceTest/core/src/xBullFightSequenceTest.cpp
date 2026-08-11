/******************************************************************************
 * @file        xBullFightSequenceTest.cpp
 * @brief       Verifies red-target pursuit sequencing through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerCommands.h"

#include <cassert>

namespace
{

void testBullFight(
    xwalk::agent::test::ControllerCommandTestContext& context)
{
    const ctrl::uint32 queryStart = context.state->operationQueries;
    context.state->operationQueryLimit = queryStart + 5U;
    const ctrl::int32 startStatus =
        xwalk::ctrl::XWALK_runControllerCommand(*context.bullFightController, {"bull-fight", "start"});
    assert(startStatus == 0);
    assert(!context.state->visionStarted);
    assert(context.state->visionObservationCount == 1U);
    assert(context.motors->left().speed() == 0.0);
    assert(context.motors->right().speed() == 0.0);

    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"vision.start", "vision.color", "vision.observe", "hal.i2c.write",
            "hal.i2c.write", "hal.i2c.write", "vision.stop",
            }));

    context.state->visionColorVisible = false;
    context.state->operationQueryLimit = context.state->operationQueries + 5U;
    const ctrl::int32 searchStatus =
        xwalk::ctrl::XWALK_runControllerCommand(*context.bullFightController, {"bull-fight", "start"});
    assert(searchStatus == 0);
    assert(context.state->visionObservationCount == 2U);
    assert(context.motors->left().speed() == 0.0);
    assert(context.motors->right().speed() == 0.0);

    const ctrl::int32 stopStatus =
        xwalk::ctrl::XWALK_runControllerCommand(*context.bullFightController, {"bull-fight", "stop"});
    assert(stopStatus == 0);

}

} /* namespace */

int xWalkBullFightCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(
        argc, argv, &testBullFight);
}
