/******************************************************************************
 * @file        xVideoRecordingSequenceTest.cpp
 * @brief       Verifies every interactive video-recording transition.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerCommands.h"

#include <cassert>

namespace
{

    void testVideoRecording(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->inputLines = {"q", "q", "q", "e", "x"};
        const ctrl::int32 status =
            xwalk::ctrl::XWALK_runControllerCommand(*context.videoRecordingController, {"record-video"});
        assert(status == 0);
        assert(!context.state->visionStarted);
        assert(!context.state->videoRecording);
        assert(!context.state->videoPaused);

        assert(xwalk::agent::test::containsOrderedEvents(
            context.state->eventLog,
            {"vision.start", "video.begin", "video.pause", "video.continue", "video.stop", "vision.stop"}));
    }

} /* namespace */

int xWalkVideoRecordingCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testVideoRecording);
}
