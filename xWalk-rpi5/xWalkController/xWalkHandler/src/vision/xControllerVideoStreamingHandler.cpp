/******************************************************************************
 * @file        xControllerVideoStreamingHandler.cpp
 * @brief       Implements foreground MJPEG video streaming.
 * @project     xWalk Firmware
 * @module      xWalkHandler
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

namespace xwalk::ctrl
{

    /**
     * @brief Runs foreground MJPEG video streaming until cancellation.
     * @param[in] request Empty command payload retained for handler consistency.
     * @return Zero after normal cancellation, two after a stream failure, or three when unavailable.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerVideoStreaming(const XWalkNoArgumentRequest& request)
    {
        static_cast<void>(request);
        if (videoStreamingObject == nullptr)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Video-streaming backend unavailable");
            return 3;
        }
        const ::ctrl::boolean started = videoStreamingObject->start();
        if (started == false)
        {
            XWALK_CTRL_ERROR(XWALK_EXCEPTION, "Video-streaming camera or listener could not be started");
            return 2;
        }
        XWALK_CTRL_TRACE_UID1(
            CTRL .089, "MJPEG stream available at http://127.0.0.1:%u/stream", videoStreamingObject->port());
        ::ctrl::boolean running = static_cast<::ctrl::boolean>(operationMayContinue());
        while (running)
        {
            const ::ctrl::boolean stepCompleted = videoStreamingObject->step();
            if (stepCompleted == false)
            {
                videoStreamingObject->stop();
                return 2;
            }
            const ::ctrl::boolean delayed = delayWhileOperationRequested(10U);
            running = static_cast<::ctrl::boolean>(delayed && operationMayContinue());
        }
        videoStreamingObject->stop();
        return 0;
    }

} /* namespace xwalk::ctrl */
