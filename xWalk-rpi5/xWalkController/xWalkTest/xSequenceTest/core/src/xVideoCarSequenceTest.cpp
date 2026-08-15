/******************************************************************************
 * @file        xVideoCarSequenceTest.cpp
 * @brief       Verifies every interactive video-car command and transition.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerCommands.h"

#include <algorithm>
#include <cassert>

namespace
{

    void testVideoCar(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->inputLines = {"o", "o", "o", "o", "o", "o", "o", "o", "o", "o", "w", "o", "o", "o", "o",
                                     "w", "s", "p", "p", "p", "p", "p", "p", "a", "d", "f", "t", "z", "x"};
        const ctrl::int32 status = xwalk::ctrl::XWALK_runControllerCommand(*context.videoCarController, {"video-car"});

        assert(status == 0);
        assert(!context.state->visionStarted);
        assert(context.state->visionCaptureCount == 1U);
        assert(context.motors->left().speed() == 0.0);
        assert(context.motors->right().speed() == 0.0);

        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {
                                                             "vision.start",
                                                             "controller.delay",
                                                             "controller.input",
                                                             "hal.i2c.write",
                                                             "vision.capture",
                                                             "vision.stop",
                                                         }));
    }

} /* namespace */

int xWalkVideoCarCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testVideoCar);
}
