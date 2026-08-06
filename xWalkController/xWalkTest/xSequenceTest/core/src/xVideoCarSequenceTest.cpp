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
    context.state->inputLines = {
        "o", "o", "o", "o", "o", "o", "o", "o", "o", "o",
        "w", "o", "o", "o", "o", "w", "s",
        "p", "p", "p", "p", "p", "p", "a", "d", "f", "t", "z", "x"};
    const ctrl::int32 status =
        xwalk::ctrl::XWALK_runControllerCommand(*context.videoCarController, {"video-car"});

    assert(status == 0);
    assert(!context.state->visionStarted);
    assert(context.state->visionCaptureCount == 1U);
    assert(context.motors->left().speed() == 0.0);
    assert(context.motors->right().speed() == 0.0);
    assert(std::find(context.state->outputLines.begin(),
        context.state->outputLines.end(), "status: forward , speed: 100") !=
        context.state->outputLines.end());
    assert(std::find(context.state->outputLines.begin(),
        context.state->outputLines.end(), "status: backward , speed: 60") !=
        context.state->outputLines.end());
    assert(std::find(context.state->outputLines.begin(),
        context.state->outputLines.end(), "status: stop , speed: 0") !=
        context.state->outputLines.end());
    assert(std::find(context.state->outputLines.begin(),
        context.state->outputLines.end(),
        "photo save as /tmp/photo_2026-08-04-12-00-00.jpg") !=
        context.state->outputLines.end());
    assert(context.state->outputLines.back() == "quit");
    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"vision.start", "controller.delay", "controller.input",
            "hal.i2c.write", "vision.capture", "vision.stop",
            "controller.output"}));
}

} /* namespace */

int xWalkVideoCarCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(
        argc, argv, &testVideoCar);
}
