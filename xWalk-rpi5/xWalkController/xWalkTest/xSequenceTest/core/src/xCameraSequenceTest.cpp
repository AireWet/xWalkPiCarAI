/******************************************************************************
 * @file        xCameraSequenceTest.cpp
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
void testCamera(xwalk::agent::test::ControllerCommandTestContext& context)
{
    const ctrl::uint32 writes = context.state->i2cWriteCount;
    xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
    assert(sequence.run({{"cam", "pan", "--angle", "60"},
        {"cam", "tilt", "--angle", "-20"}}) == 0);
    assert(context.state->i2cWriteCount >= (writes + 2U));
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"hal.i2c.write", "hal.i2c.write"}));
}
}

/** @brief Runs the camera controller-to-HAL host sequence. @return Zero on success. */
int xWalkCameraCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testCamera);
}
