/******************************************************************************
 * @file        xDoctorSequenceTest.cpp
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
void testDoctor(xwalk::agent::test::ControllerCommandTestContext& context)
{
    xwalk::agent::test::XWalkControllerSequence passing(*context.doctorController);
    xwalk::agent::test::XWalkControllerSequence failing(*context.failingDoctorController);
    assert(passing.run({{"doctor"}}) == 0);
    assert(context.state->outputLines.back() == "[PASS] Configuration: ready");
    assert(failing.run({{"doctor"}, {"help"}}) == 2);
    assert(context.state->outputLines.back() == "[FAIL] I2C: unavailable");
    assert(context.state->outputLines.size() == 2U);
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"controller.output", "controller.output"}));
}
}

/** @brief Runs the doctor controller-to-HAL host sequence. @return Zero on success. */
int xWalkDoctorCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testDoctor);
}
