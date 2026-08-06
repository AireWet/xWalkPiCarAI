/******************************************************************************
 * @file        xControllerVideoRecordingHandler.cpp
 * @brief       Implements the VideoRecordingHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xController.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

::ctrl::int32 XWalkController::XWALK_handlerVideoRecording(
    const XWalkNoArgumentRequest& request)
{
    static_cast<void>(request);
    if (videoRecordingObject == nullptr)
    {
        output("Video-recording backend unavailable");
        return 3;
    }
    const ::ctrl::boolean started = videoRecordingObject->start();
    if (started == false)
    {
        output("Video-recording camera could not be started");
        return 2;
    }

    output("Recording keys: q start/pause/continue; e stop; x exit.");
    const ::ctrl::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const ::ctrl::boolean operationAllowed =
            static_cast<::ctrl::boolean>(
                operationMayContinue());
        if (operationAllowed == false)
        {
            break;
        }
        const ::ctrl::string key = input("record> ");
        if ((key == "x") || (key == "X") || (key == "exit") ||
            (key == "quit") || (key == "skip"))
        {
            break;
        }
        const agent::XWalkVideoRecordingResult result =
            videoRecordingObject->handleKey(key);
        if (result.event == agent::XWalkVideoRecordingEvent::Started)
        {
            output("rec start ...");
        }
        else if (result.event == agent::XWalkVideoRecordingEvent::Paused)
        {
            output("pause");
        }
        else if (result.event == agent::XWalkVideoRecordingEvent::Continued)
        {
            output("continue");
        }
        else if (result.event == agent::XWalkVideoRecordingEvent::Stopped)
        {
            output("The video saved as " + result.videoPath);
        }
        if (result.event == agent::XWalkVideoRecordingEvent::Cancelled)
        {
            break;
        }
    }
    videoRecordingObject->stop();
    output("quit");
    return 0;
}

} /* namespace xwalk::ctrl */
