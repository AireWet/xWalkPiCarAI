/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecording.cpp
 * @brief       Implements source-compatible recording key transitions.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecording.h"

#include "xHal_Rpi5CarExceptions.h"

namespace xwalk::agent
{

XWalkVideoRecordingResult XWalkVideoRecording::handleKey(
    agent::stringview keyText)
{
    if (!startedValue)
    {
        XHAL_THROW_LOGIC_ERROR("Video-recording camera is not started");
    }
    XWalkVideoRecordingResult result{};
    if ((keyText == "q") || (keyText == "Q"))
    {
        if (stateValue == XWalkVideoRecordingState::Stopped)
        {
            const agent::string name = callbacks.timestamp(callbackContext);
            const agent::boolean nameEmpty =
                static_cast<agent::boolean>(
                    name.empty());
            if (nameEmpty)
            {
                XHAL_THROW_RUNTIME_ERROR("Video-recording timestamp is empty");
            }
            videoPathValue = callbacks.beginRecording(callbackContext, name);
            const agent::boolean videoPathEmpty =
                static_cast<agent::boolean>(
                    videoPathValue.empty());
            if (videoPathEmpty)
            {
                XHAL_THROW_RUNTIME_ERROR("Video-recording path is empty");
            }
            stateValue = XWalkVideoRecordingState::Recording;
            result.event = XWalkVideoRecordingEvent::Started;
        }
        else if (stateValue == XWalkVideoRecordingState::Recording)
        {
            callbacks.pauseRecording(callbackContext);
            stateValue = XWalkVideoRecordingState::Paused;
            result.event = XWalkVideoRecordingEvent::Paused;
        }
        else
        {
            callbacks.continueRecording(callbackContext);
            stateValue = XWalkVideoRecordingState::Recording;
            result.event = XWalkVideoRecordingEvent::Continued;
        }
    }
    else if (((keyText == "e") || (keyText == "E")) &&
        (stateValue != XWalkVideoRecordingState::Stopped))
    {
        callbacks.stopRecording(callbackContext);
        stateValue = XWalkVideoRecordingState::Stopped;
        result.event = XWalkVideoRecordingEvent::Stopped;
        result.videoPath = videoPathValue;
    }
    const agent::boolean delayCompleted = wait(100U);
    if (delayCompleted == false)
    {
        result.event = XWalkVideoRecordingEvent::Cancelled;
    }
    result.state = stateValue;
    const agent::boolean startedEventMissingPath =
        static_cast<agent::boolean>(
            result.videoPath.empty() &&
        (result.event == XWalkVideoRecordingEvent::Started));
    if (startedEventMissingPath)
    {
        result.videoPath = videoPathValue;
    }
    return result;
}

} /* namespace xwalk::agent */
