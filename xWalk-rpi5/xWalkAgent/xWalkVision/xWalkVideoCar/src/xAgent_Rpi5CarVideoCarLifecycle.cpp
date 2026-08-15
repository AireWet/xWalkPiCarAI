/******************************************************************************
 * @file        xAgent_Rpi5CarVideoCarLifecycle.cpp
 * @brief       Implements interactive video-car lifecycle and timing.
 * @project     xWalk Firmware
 * @module      xWalkVideoCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoCar.h"

#include "xHal_Rpi5CarTrace.h"
#include <cmath>

namespace xwalk::agent
{

    XWalkVideoCar::XWalkVideoCar(XWalkPicarx& picarx,
                                 agent::contextpointer context,
                                 const XWalkComputerVisionCallbacks& providerCallbacks,
                                 const XWalkVideoCarConfiguration& configuration)
        : picarxObject(&picarx), callbackContext(context), callbacks(providerCallbacks),
          configurationValue(configuration)
    {
        validate(callbacks, configurationValue);
    }

    XWalkVideoCar::~XWalkVideoCar() noexcept
    {
        if (startedValue)
        {
            callbacks.stop(callbackContext);
        }
        static_cast<void>(picarxObject->emergencyStop());
    }

    void XWalkVideoCar::validate(const XWalkComputerVisionCallbacks& providerCallbacks,
                                 const XWalkVideoCarConfiguration& configuration)
    {
        if ((providerCallbacks.start == nullptr) || (providerCallbacks.stop == nullptr) ||
            (providerCallbacks.capture == nullptr) || (providerCallbacks.delay == nullptr) ||
            (providerCallbacks.continueOperation == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video car requires complete callbacks");
        }
        const agent::boolean steeringAngleDegreesNotFinite =
            static_cast<agent::boolean>(!std::isfinite(configuration.steeringAngleDegrees));
        if (steeringAngleDegreesNotFinite)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Video-car steering angle must be finite");
        }
        if ((configuration.speedStepPercent == 0U) ||
            (configuration.speedStepPercent > configuration.maximumSpeedPercent) ||
            (configuration.maximumSpeedPercent > 100U) ||
            (configuration.directionChangeCapPercent > configuration.maximumSpeedPercent) ||
            (configuration.steeringAngleDegrees <= 0.0) || (configuration.steeringAngleDegrees > 45.0) ||
            (configuration.startupDelayMs > 10'000U) || (configuration.keyDelayMs == 0U) ||
            (configuration.keyDelayMs > 1'000U))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Video-car configuration is out of range");
        }
    }

    agent::boolean XWalkVideoCar::wait(agent::uint32 durationMs) const
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

    agent::boolean XWalkVideoCar::start()
    {
        if (startedValue)
        {
            return true;
        }
        motionValue = XWalkVideoCarMotion::Stop;
        speedPercentValue = 0U;
        startedValue = callbacks.start(callbackContext);
        if (startedValue)
        {
            const agent::boolean delayCompleted = wait(configurationValue.startupDelayMs);
            if (delayCompleted == false)
            {
                picarxObject->stop();
                callbacks.stop(callbackContext);
                startedValue = false;
            }
        }
        return startedValue;
    }

    void XWalkVideoCar::finish()
    {
        picarxObject->stop();
        if (startedValue)
        {
            callbacks.stop(callbackContext);
        }
        startedValue = false;
        motionValue = XWalkVideoCarMotion::Stop;
        speedPercentValue = 0U;
    }

    agent::boolean XWalkVideoCar::started() const noexcept
    {
        return startedValue;
    }

} /* namespace xwalk::agent */
