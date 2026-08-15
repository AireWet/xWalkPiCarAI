/******************************************************************************
 * @file        xStorytellingRobotSequenceTest.cpp
 * @brief       Verifies the storytelling command through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <algorithm>
#include <cassert>

namespace
{
    void testStorytellingRobot(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        xwalk::agent::test::XWalkControllerSequence sequence(*context.storytellingController);
        assert(sequence.run({{"storytelling-robot", "start"}, {"storytelling-robot", "stop"}}) == 0);
        assert(context.state->spokenText ==
               ctrl::stringvector({"Hello! I'm PiCar-X speaking with Piper.",
                                   "Why can't your nose be twelve inches long? Because then it would be a foot!",
                                   "Why did the cow go to outer space? To see the moooon!",
                                   "That's all for today. Goodbye, let's go home and sleep."}));
        assert(context.state->delays.size() == 600U);
        assert(std::all_of(context.state->delays.begin(),
                           context.state->delays.end(),
                           [](ctrl::uint32 delayMs)
                           {
                               return delayMs == 20U;
                           }));
        assert(context.state->leftSpeeds.size() == 600U);
        assert(std::all_of(context.state->leftSpeeds.begin(),
                           context.state->leftSpeeds.begin() + 300,
                           [](ctrl::float64 speed)
                           {
                               return speed > 0.0;
                           }));
        assert(std::all_of(context.state->leftSpeeds.begin() + 300,
                           context.state->leftSpeeds.end(),
                           [](ctrl::float64 speed)
                           {
                               return speed < 0.0;
                           }));
        assert(context.motors->left().speed() == 0.0);
        assert(context.picarx->directionAngleDegrees() == 0.0);
    }
} // namespace

/** @brief Runs the storytelling controller-to-HAL host sequence. */
int xWalkStorytellingRobotCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testStorytellingRobot);
}
