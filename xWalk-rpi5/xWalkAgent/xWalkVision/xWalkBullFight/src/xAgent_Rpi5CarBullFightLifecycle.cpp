/******************************************************************************
 * @file        xAgent_Rpi5CarBullFightLifecycle.cpp
 * @brief       Implements red-target pursuit lifecycle and timing.
 * @project     xWalk Firmware
 * @module      xWalkBullFight
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarBullFight.h"

#include "xHal_Rpi5CarTrace.h"
#include <cmath>

namespace xwalk::agent
{

    XWalkBullFight::XWalkBullFight(XWalkPicarx& picarx,
                                   agent::contextpointer context,
                                   const XWalkComputerVisionCallbacks& providerCallbacks,
                                   const XWalkBullFightConfiguration& configuration)
        : picarxObject(&picarx), callbackContext(context), callbacks(providerCallbacks),
          configurationValue(configuration)
    {
        validate(callbacks, configurationValue);
    }

    XWalkBullFight::~XWalkBullFight() noexcept
    {
        if (startedValue)
        {
            callbacks.stop(callbackContext);
        }
        static_cast<void>(picarxObject->emergencyStop());
    }

    void XWalkBullFight::validate(const XWalkComputerVisionCallbacks& providerCallbacks,
                                  const XWalkBullFightConfiguration& configuration)
    {
        if ((providerCallbacks.start == nullptr) || (providerCallbacks.stop == nullptr) ||
            (providerCallbacks.setColor == nullptr) || (providerCallbacks.observe == nullptr) ||
            (providerCallbacks.delay == nullptr) || (providerCallbacks.continueOperation == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Bull fight requires complete callbacks");
        }
        const agent::boolean configurationInvalid = static_cast<agent::boolean>(
            !std::isfinite(configuration.correctionSpanDegrees) ||
            !std::isfinite(configuration.maximumCameraAngleDegrees) || !std::isfinite(configuration.speedPercent));
        if (configurationInvalid)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Bull-fight numeric settings must be finite");
        }
        if ((configuration.frameWidthPixels < 16U) || (configuration.frameWidthPixels > 7'680U) ||
            (configuration.frameHeightPixels < 16U) || (configuration.frameHeightPixels > 4'320U) ||
            (configuration.correctionSpanDegrees <= 0.0) || (configuration.correctionSpanDegrees > 180.0) ||
            (configuration.maximumCameraAngleDegrees <= 0.0) || (configuration.maximumCameraAngleDegrees > 90.0) ||
            (configuration.speedPercent < 0.0) || (configuration.speedPercent > 100.0) ||
            (configuration.sampleDelayMs == 0U) || (configuration.sampleDelayMs > 1'000U) ||
            (configuration.finalDelayMs > 1'000U))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Bull-fight configuration is out of range");
        }
    }

    agent::boolean XWalkBullFight::wait(agent::uint32 durationMs) const
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

    agent::boolean XWalkBullFight::start()
    {
        if (startedValue)
        {
            return true;
        }
        panAngleDegreesValue = 0.0;
        tiltAngleDegreesValue = 0.0;
        directionAngleDegreesValue = 0.0;
        startedValue = callbacks.start(callbackContext);
        if (startedValue)
        {
            callbacks.setColor(callbackContext, XWalkComputerVisionColor::Red);
        }
        return startedValue;
    }

    void XWalkBullFight::finish()
    {
        picarxObject->stop();
        if (startedValue)
        {
            callbacks.stop(callbackContext);
        }
        startedValue = false;
        panAngleDegreesValue = 0.0;
        tiltAngleDegreesValue = 0.0;
        directionAngleDegreesValue = 0.0;
        callbacks.delay(callbackContext, configurationValue.finalDelayMs);
    }

    agent::boolean XWalkBullFight::started() const noexcept
    {
        return startedValue;
    }

} /* namespace xwalk::agent */
