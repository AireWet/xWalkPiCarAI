/******************************************************************************
 * @file        xControllerVideoRecordingHandler.cpp
 * @brief       Implements the VideoRecordingHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
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

#include "xHal_Rpi5CarTrace.h"

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

    ::ctrl::int32 XWalkController::XWALK_handlerVideoRecording(const XWalkNoArgumentRequest& request)
    {
        static_cast<void>(request);
        if (videoRecordingObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Video-recording backend unavailable");
            return 3;
        }
        const ::ctrl::boolean started = videoRecordingObject->start();
        if (started == false)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Video-recording camera could not be started");
            return 2;
        }

        XWALK_CTRL_TRACE_UID0(CTRL .071, "Recording keys: q start/pause/continue; e stop; x exit.");
        const ::ctrl::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const ::ctrl::boolean operationAllowed = static_cast<::ctrl::boolean>(operationMayContinue());
            if (operationAllowed == false)
            {
                break;
            }
            const ::ctrl::string key = input("record> ");
            if ((key == "x") || (key == "X") || (key == "exit") || (key == "quit") || (key == "skip"))
            {
                break;
            }
            const agent::XWalkVideoRecordingResult result = videoRecordingObject->handleKey(key);
            if (result.event == agent::XWalkVideoRecordingEvent::Started)
            {
                XWALK_CTRL_TRACE_UID0(CTRL .072, "rec start ...");
            }
            else if (result.event == agent::XWalkVideoRecordingEvent::Paused)
            {
                XWALK_CTRL_TRACE_UID0(CTRL .073, "pause");
            }
            else if (result.event == agent::XWalkVideoRecordingEvent::Continued)
            {
                XWALK_CTRL_TRACE_UID0(CTRL .074, "continue");
            }
            else if (result.event == agent::XWalkVideoRecordingEvent::Stopped)
            {
                XWALK_CTRL_TRACE_UID1(CTRL .075, "The video saved as %s", result.videoPath.c_str());
            }
            if (result.event == agent::XWalkVideoRecordingEvent::Cancelled)
            {
                break;
            }
        }
        videoRecordingObject->stop();
        XWALK_CTRL_TRACE_UID0(CTRL .076, "quit");
        return 0;
    }

} /* namespace xwalk::ctrl */
