/******************************************************************************
 * @file        xAgent_Rpi5CarAppControlLifecycle.cpp
 * @brief       Implements mobile-app control lifecycle and validation.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControl.h"

#include "xHal_Rpi5CarExceptions.h"

#include <cmath>

namespace xwalk::agent
{

XWalkAppControl::XWalkAppControl(XWalkPicarx& picarx,
    const XWalkAppControlCallbacks& providerCallbacks,
    const XWalkAppControlConfiguration& configuration):
    picarxObject(&picarx), callbacks(providerCallbacks),
    configurationValue(configuration)
{
    validate(callbacks, configurationValue);
}

XWalkAppControl::~XWalkAppControl() noexcept
{
    if (startedValue)
    {
        callbacks.vision.stop(callbacks.visionContext);
        callbacks.stop(callbacks.transportContext);
    }
    static_cast<void>(picarxObject->emergencyStop());
}

void XWalkAppControl::validate(
    const XWalkAppControlCallbacks& providerCallbacks,
    const XWalkAppControlConfiguration& configuration)
{
    if ((providerCallbacks.start == nullptr) ||
        (providerCallbacks.stop == nullptr) ||
        (providerCallbacks.poll == nullptr) ||
        (providerCallbacks.publish == nullptr) ||
        (providerCallbacks.vision.start == nullptr) ||
        (providerCallbacks.vision.stop == nullptr) ||
        (providerCallbacks.vision.setColor == nullptr) ||
        (providerCallbacks.vision.setFace == nullptr) ||
        (providerCallbacks.vision.delay == nullptr) ||
        (providerCallbacks.vision.continueOperation == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("App control requires complete callbacks");
    }
    const agent::boolean configurationInvalid =
        static_cast<agent::boolean>(
            configuration.controllerName.empty() ||
        configuration.controllerType.empty() ||
        !std::isfinite(configuration.lineTrackingSpeedPercent) ||
        !std::isfinite(configuration.lineTrackingAngleDegrees) ||
        !std::isfinite(configuration.obstacleSpeedPercent));
    if (configurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("App-control configuration is invalid");
    }
    if ((configuration.controllerPort == 0U) ||
        (configuration.lineTrackingSpeedPercent < 0.0) ||
        (configuration.lineTrackingSpeedPercent > 100.0) ||
        (configuration.lineTrackingAngleDegrees < 0.0) ||
        (configuration.lineTrackingAngleDegrees > 30.0) ||
        (configuration.obstacleSpeedPercent < 0.0) ||
        (configuration.obstacleSpeedPercent > 100.0) ||
        (configuration.maximumLineRecoverySamples == 0U) ||
        (configuration.maximumLineRecoverySamples > 100'000U) ||
        (configuration.sampleDelayMs == 0U) ||
        (configuration.sampleDelayMs > 1'000U))
    {
        XHAL_THROW_OUT_OF_RANGE("App-control configuration is out of range");
    }
}

agent::boolean XWalkAppControl::wait(agent::uint32 durationMs) const
{
    constexpr agent::uint32 cancellationIntervalMs{20U};
    agent::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        const agent::boolean operationRequested =
            callbacks.vision.continueOperation(callbacks.visionContext);
        if (operationRequested == false)
        {
            return false;
        }
        const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        callbacks.vision.delay(callbacks.visionContext, sliceMs);
        remainingMs -= sliceMs;
    }
    return callbacks.vision.continueOperation(callbacks.visionContext);
}

agent::boolean XWalkAppControl::start()
{
    if (startedValue)
    {
        return true;
    }
    speedPercentValue = 0.0;
    lastColorEnabled = false;
    lastFaceEnabled = false;
    lastObjectEnabled = false;
    lastLineState = 0U;
    const agent::boolean transportStarted = callbacks.start(callbacks.transportContext,
        configurationValue.controllerName, configurationValue.controllerType,
        configurationValue.controllerPort);
    if (transportStarted == false)
    {
        return false;
    }
    const agent::boolean visionStarted = callbacks.vision.start(callbacks.visionContext);
    if (visionStarted == false)
    {
        callbacks.stop(callbacks.transportContext);
        return false;
    }
    startedValue = true;
    return true;
}

void XWalkAppControl::finish()
{
    picarxObject->stop();
    if (startedValue)
    {
        callbacks.vision.stop(callbacks.visionContext);
        callbacks.stop(callbacks.transportContext);
    }
    startedValue = false;
    speedPercentValue = 0.0;
    lastLineState = 0U;
}

agent::boolean XWalkAppControl::started() const noexcept
{
    return startedValue;
}

} /* namespace xwalk::agent */
