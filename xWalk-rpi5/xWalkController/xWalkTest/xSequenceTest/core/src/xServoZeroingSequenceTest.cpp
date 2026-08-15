/******************************************************************************
 * @file        xServoZeroingSequenceTest.cpp
 * @brief       Verifies the servo-zeroing CLI sequence through simulated HAL.
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

namespace
{

    /** @brief Verifies every source servo command and its order. */
    void testServoZeroing(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->operationQueryLimit = 120U;
        xwalk::agent::test::XWalkControllerSequence sequence(*context.servoZeroingController);
        assert(sequence.run({{"servo-zeroing"}}) == 0);

        assert(context.state->servoZeroingIds.size() == 24U);
        assert(context.state->servoZeroingAngles.size() == 24U);
        for (ctrl::uint32 servoId = 0U; servoId < 12U; ++servoId)
        {
            const ctrl::size index = static_cast<ctrl::size>(servoId) * 2U;
            assert(context.state->servoZeroingIds[index] == servoId);
            assert(context.state->servoZeroingIds[index + 1U] == servoId);
            assert(context.state->servoZeroingAngles[index] == 10.0);
            assert(context.state->servoZeroingAngles[index + 1U] == 0.0);
        }
        assert(context.state->delays.size() == 120U);
        assert(xwalk::agent::test::containsOrderedEvents(
            context.state->eventLog,
            {"servo-zeroing.angle", "controller.continue", "controller.delay", "servo-zeroing.angle"}));
    }

} /* namespace */

/**
 * @brief Runs the servo-zeroing controller-to-HAL host sequence.
 * @param[in] argc Process argument count.
 * @param[in] argv Process argument values.
 * @return Zero after all assertions pass.
 */
int xWalkServoZeroingCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testServoZeroing);
}
