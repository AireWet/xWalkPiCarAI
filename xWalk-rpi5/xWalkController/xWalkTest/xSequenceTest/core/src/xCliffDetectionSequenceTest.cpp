/******************************************************************************
 * @file        xCliffDetectionSequenceTest.cpp
 * @brief       Verifies cliff-detection command sequencing through simulated HAL.
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

    /** @brief Verifies explicit stop, bounded foreground cancellation, and cleanup. */
    void testCliffDetection(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
        assert(sequence.run({{"cliff-detection", "stop"}}) == 0);

        const ctrl::uint32 queryStart = context.state->operationQueries;
        context.state->operationQueryLimit = queryStart + 2U;
        assert(xwalk::ctrl::XWALK_runControllerCommand(*context.controller, {"cliff-detection", "start"}) == 0);
        assert(context.state->operationQueries == (queryStart + 3U));

        assert(context.motors->left().speed() == 0.0);
        assert(context.motors->right().speed() == 0.0);
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {
                                                             "controller.continue",
                                                             "hal.i2c.write",
                                                             "controller.continue",
                                                             "hal.i2c.write",
                                                             "controller.continue",
                                                             "hal.i2c.write",
                                                         }));
    }

} /* namespace */

/**
 * @brief Runs the cliff-detection controller-to-HAL host sequence.
 * @param[in] argc Must be two.
 * @param[in] argv Test name and writable configuration path.
 * @return Zero after successful assertions or one for invalid arguments.
 */
int xWalkCliffDetectionCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testCliffDetection);
}
