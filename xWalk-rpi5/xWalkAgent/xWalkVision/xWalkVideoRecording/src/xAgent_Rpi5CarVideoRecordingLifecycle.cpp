/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingLifecycle.cpp
 * @brief       Implements video-recording validation and camera lifecycle.
 * @project     xWalk Firmware
 * @module      xWalkVideoRecording
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecording.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::agent
{

    XWalkVideoRecording::XWalkVideoRecording(agent::contextpointer context,
                                             const XWalkVideoRecordingCallbacks& providerCallbacks)
        : callbackContext(context), callbacks(providerCallbacks)
    {
        validateCallbacks(callbacks);
    }

    XWalkVideoRecording::~XWalkVideoRecording() noexcept
    {
        stop();
    }

    void XWalkVideoRecording::validateCallbacks(const XWalkVideoRecordingCallbacks& providerCallbacks)
    {
        if ((providerCallbacks.startCamera == nullptr) || (providerCallbacks.stopCamera == nullptr) ||
            (providerCallbacks.beginRecording == nullptr) || (providerCallbacks.pauseRecording == nullptr) ||
            (providerCallbacks.continueRecording == nullptr) || (providerCallbacks.stopRecording == nullptr) ||
            (providerCallbacks.delay == nullptr) || (providerCallbacks.continueOperation == nullptr) ||
            (providerCallbacks.timestamp == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video recording requires complete callbacks");
        }
    }

    agent::boolean XWalkVideoRecording::wait(agent::uint32 durationMs) const
    {
        constexpr agent::uint32 cancellationIntervalMs{20U};
        agent::uint32 remainingMs = durationMs;
        while (remainingMs > 0U)
        {
            const agent::boolean operationRequested = callbacks.continueOperation(callbackContext);
            if (operationRequested == false)
            {
                return false;
            }
            const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ? remainingMs : cancellationIntervalMs;
            callbacks.delay(callbackContext, sliceMs);
            remainingMs -= sliceMs;
        }
        return callbacks.continueOperation(callbackContext);
    }

    agent::boolean XWalkVideoRecording::start()
    {
        if (startedValue)
        {
            return true;
        }
        stateValue = XWalkVideoRecordingState::Stopped;
        videoPathValue.clear();
        startedValue = callbacks.startCamera(callbackContext);
        if (startedValue)
        {
            const agent::boolean delayCompleted = wait(800U);
            if (delayCompleted == false)
            {
                stop();
                return false;
            }
        }
        return startedValue;
    }

    void XWalkVideoRecording::stop() noexcept
    {
        if (startedValue)
        {
            if (stateValue != XWalkVideoRecordingState::Stopped)
            {
                callbacks.stopRecording(callbackContext);
            }
            callbacks.stopCamera(callbackContext);
        }
        stateValue = XWalkVideoRecordingState::Stopped;
        videoPathValue.clear();
        startedValue = false;
    }

    XWalkVideoRecordingState XWalkVideoRecording::state() const noexcept
    {
        return stateValue;
    }

    agent::boolean XWalkVideoRecording::started() const noexcept
    {
        return startedValue;
    }

} /* namespace xwalk::agent */
