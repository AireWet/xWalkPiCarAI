/******************************************************************************
 * @file        xHelpSequenceTest.cpp
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
void testHelp(xwalk::agent::test::ControllerCommandTestContext& context)
{
    xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
    assert(sequence.run({{"help"}, {"--help"}, {"-h"}}) == 0);
    assert(context.state->outputLines.size() == 3U);
    assert(context.state->outputLines.back().find("Commands:\n") != ctrl::string::npos);
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"controller.output", "controller.output", "controller.output"}));
}
}

/** @brief Runs the help controller-to-HAL host sequence. @return Zero on success. */
int xWalkHelpCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testHelp);
}
