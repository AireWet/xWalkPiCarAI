/******************************************************************************
 * @file        xAppControlSequenceTest.cpp
 * @brief       Verifies mobile-app telemetry and control sequencing.
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

    void testAppControl(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        xwalk::agent::XWalkAppControlInput joystick;
        joystick.hornRequested = true;
        joystick.driveJoystickAvailable = true;
        joystick.driveX = 100.0;
        joystick.driveY = 80.0;
        joystick.cameraJoystickAvailable = true;
        joystick.cameraPanDegrees = 100.0;
        joystick.cameraTiltDegrees = -100.0;
        joystick.colorDetectionEnabled = true;
        joystick.faceDetectionEnabled = true;
        joystick.objectDetectionEnabled = true;

        xwalk::agent::XWalkAppControlInput reverse;
        reverse.driveJoystickAvailable = true;
        reverse.driveX = -100.0;
        reverse.driveY = -30.0;

        xwalk::agent::XWalkAppControlInput voice;
        voice.spokenCommand = "stop";

        xwalk::agent::XWalkAppControlInput line;
        line.lineTrackingEnabled = true;

        xwalk::agent::XWalkAppControlInput obstacle;
        obstacle.obstacleAvoidanceEnabled = true;

        context.state->appInputs = {joystick, reverse, voice, line, obstacle};
        context.state->operationQueryLimit = context.state->operationQueries + 35U;
        const ctrl::int32 status =
            xwalk::ctrl::XWALK_runControllerCommand(*context.appControlController, {"app-control", "start"});

        assert(status == 0);
        assert(!context.state->appTransportStarted);
        assert(!context.state->visionStarted);
        assert(context.state->appPublishCount >= 5U);
        assert(context.state->visionColor == xwalk::agent::XWalkComputerVisionColor::Close);
        assert(context.state->soundOperation == xwalk::ctrl::XWalkSoundOperation::Play);
        assert(context.state->soundFile == "car-double-horn.wav");
        assert(context.motors->left().speed() == 0.0);
        assert(context.motors->right().speed() == 0.0);

        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
                                                         {"app.start",
                                                          "vision.start",
                                                          "app.publish",
                                                          "app.poll",
                                                          "hal.i2c.write",
                                                          "vision.color",
                                                          "vision.face",
                                                          "vision.stop",
                                                          "app.stop"}));

        const ctrl::int32 stopStatus =
            xwalk::ctrl::XWALK_runControllerCommand(*context.appControlController, {"app-control", "stop"});
        assert(stopStatus == 0);
    }

} /* namespace */

int xWalkAppControlCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testAppControl);
}
