/******************************************************************************
 * @file        xSelfDriveSequenceTest.cpp
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
    void testSelfDrive(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        const ctrl::stringvector actions{"shake-head",
                                         "nod",
                                         "wave-hands",
                                         "resist",
                                         "act-cute",
                                         "rub-hands",
                                         "think",
                                         "twist-body",
                                         "celebrate",
                                         "depressed",
                                         "forward",
                                         "backward",
                                         "honking",
                                         "start-engine",
                                         "play-background-music",
                                         "stop-background-music"};
        xwalk::agent::test::controllercommandsequence commands;
        for (const ctrl::string& action : actions)
        {
            commands.push_back({"self-drive", action});
        }
        xwalk::agent::test::XWalkControllerSequence sequence(*context.selfDriveController);
        assert(sequence.run(commands) == 0);
        assert(!context.state->delays.empty());
        assert(context.motors->left().speed() == 0.0);
        assert(context.state->musicSoundFile.find("car-start-engine.wav") != ctrl::string::npos);
        assert(xwalk::agent::test::containsOrderedEvents(
            context.state->eventLog,
            {"controller.continue", "hal.i2c.write", "controller.delay", "controller.continue", "hal.music.sound"}));
    }
} // namespace

/** @brief Runs the self-drive controller-to-HAL host sequence. @return Zero on success. */
int xWalkSelfDriveCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testSelfDrive);
}
