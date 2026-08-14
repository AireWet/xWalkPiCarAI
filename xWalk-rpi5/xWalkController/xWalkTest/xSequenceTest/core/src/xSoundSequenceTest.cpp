/******************************************************************************
 * @file        xSoundSequenceTest.cpp
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
void testSound(xwalk::agent::test::ControllerCommandTestContext& context)
{
    xwalk::agent::test::XWalkControllerSequence sequence(*context.controller);
    assert(sequence.run({{"sound", "play", "horn.wav", "--volume", "80"},
        {"sound", "volume", "25"}, {"sound", "music", "trail.mp3"},
        {"sound", "stop"}}) == 0);
    assert(context.state->soundOperation == xwalk::ctrl::XWalkSoundOperation::Stop);
    assert(context.state->soundFile.empty());
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"hal.sound", "hal.sound", "hal.sound", "hal.sound"}));
}
}

/** @brief Runs the sound controller-to-HAL host sequence. @return Zero on success. */
int xWalkSoundCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testSound);
}
