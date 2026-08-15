/******************************************************************************
 * @file        xSensorSequenceTest.cpp
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
    void testSensor(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
        assert(sequence.run({{"sensor", "distance"}, {"sensor", "grayscale"}}) == 0);

        assert(context.configuration->get("line_reference") == "[1000,1000,1000]");
        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"hal.i2c.read", "hal.i2c.read", "hal.i2c.read"}));
    }
} // namespace

/** @brief Runs the sensor controller-to-HAL host sequence. @return Zero on success. */
int xWalkSensorCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testSensor);
}
