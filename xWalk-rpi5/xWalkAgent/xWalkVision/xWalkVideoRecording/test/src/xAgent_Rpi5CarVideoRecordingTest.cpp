/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingTest.cpp
 * @brief       Verifies recording state transitions with in-memory callbacks.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording Host Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecording.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>
#include "xAgent_Rpi5CarVideoRecordingTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestState = ::xwalk::source_types::xagent_rpi5carvideorecordingtest::TestState;

namespace
{

    agent::boolean startCamera(agent::contextpointer context)
    {
        static_cast<TestState*>(context)->cameraStarted = true;
        return true;
    }

    void stopCamera(agent::contextpointer context) noexcept
    {
        static_cast<TestState*>(context)->cameraStarted = false;
    }

    agent::string beginRecording(agent::contextpointer context, agent::stringview name)
    {
        TestState& state = *static_cast<TestState*>(context);
        state.recording = true;
        state.name = name;
        return "/tmp/xwalk-videos/" + agent::string(name) + ".avi";
    }

    void pauseRecording(agent::contextpointer context)
    {
        static_cast<TestState*>(context)->paused = true;
    }

    void continueRecording(agent::contextpointer context)
    {
        static_cast<TestState*>(context)->paused = false;
    }

    void stopRecording(agent::contextpointer context) noexcept
    {
        TestState& state = *static_cast<TestState*>(context);
        state.recording = false;
        state.paused = false;
    }

    void delay(agent::contextpointer context, agent::uint32 durationMs)
    {
        static_cast<TestState*>(context)->delayTotalMs += durationMs;
    }

    agent::boolean continueOperation(agent::contextpointer context)
    {
        TestState& state = *static_cast<TestState*>(context);
        const agent::boolean result = state.continueCount < state.continueLimit;
        ++state.continueCount;
        return result;
    }

    agent::string timestamp(agent::contextpointer context)
    {
        static_cast<void>(context);
        return "2026-08-05-12.30.45";
    }

    xwalk::agent::XWalkVideoRecordingCallbacks callbacks()
    {
        return {&startCamera,
                &stopCamera,
                &beginRecording,
                &pauseRecording,
                &continueRecording,
                &stopRecording,
                &delay,
                &continueOperation,
                &timestamp};
    }

    void testRecording()
    {
        TestState state;
        xwalk::agent::XWalkVideoRecording recording(&state, callbacks());
        const agent::boolean started = recording.start();
        assert(started);
        assert(state.delayTotalMs == 800U);

        xwalk::agent::XWalkVideoRecordingResult result = recording.handleKey("Q");
        assert(result.event == xwalk::agent::XWalkVideoRecordingEvent::Started);
        assert(result.state == xwalk::agent::XWalkVideoRecordingState::Recording);
        assert(result.videoPath == "/tmp/xwalk-videos/2026-08-05-12.30.45.avi");
        result = recording.handleKey("q");
        assert(result.event == xwalk::agent::XWalkVideoRecordingEvent::Paused);
        result = recording.handleKey("q");
        assert(result.event == xwalk::agent::XWalkVideoRecordingEvent::Continued);
        result = recording.handleKey("ignored");
        assert(result.event == xwalk::agent::XWalkVideoRecordingEvent::Ignored);
        result = recording.handleKey("e");
        assert(result.event == xwalk::agent::XWalkVideoRecordingEvent::Stopped);
        assert(result.videoPath == "/tmp/xwalk-videos/2026-08-05-12.30.45.avi");
        assert(state.delayTotalMs == 1'300U);
        recording.stop();
        assert(!state.cameraStarted);
    }

    void testValidationAndCancellation()
    {
        TestState state;
        xwalk::agent::XWalkVideoRecording recording(&state, callbacks());
        xwalk::hal::test::expectFailure(
            [&recording]()
            {
                static_cast<void>(recording.handleKey("q"));
            });
        state.continueLimit = 0U;
        const agent::boolean started = recording.start();
        assert(!started);
        assert(!state.cameraStarted);

        xwalk::agent::XWalkVideoRecordingCallbacks invalid = callbacks();
        invalid.pauseRecording = nullptr;
        xwalk::hal::test::expectFailure(
            [&state, &invalid]()
            {
                xwalk::agent::XWalkVideoRecording rejected(&state, invalid);
                static_cast<void>(rejected);
            });
    }

} /* namespace */

int main()
{
    testRecording();
    testValidationAndCancellation();
    return 0;
}
