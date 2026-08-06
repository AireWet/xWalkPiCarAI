/******************************************************************************
 * @file        xAgent_Rpi5CarFaceTrackingLifecycle.cpp
 * @brief       Implements face-tracking construction, lifecycle, and timing.
 * @project     xWalk Firmware
 * @module      xWalkFaceTracking
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarFaceTracking.h"

#include "xHal_Rpi5CarExceptions.h"

#include <cmath>

namespace xwalk::agent
{

XWalkFaceTracking::XWalkFaceTracking(XWalkPicarx& picarx,
    agent::contextpointer context,
    const XWalkComputerVisionCallbacks& providerCallbacks,
    const XWalkFaceTrackingConfiguration& configuration):
    picarxObject(&picarx), callbackContext(context), callbacks(providerCallbacks),
    configurationValue(configuration)
{
    validate(callbacks, configurationValue);
}

XWalkFaceTracking::~XWalkFaceTracking() noexcept
{
    if (startedValue)
    {
        callbacks.stop(callbackContext);
    }
    picarxObject->emergencyStop();
}

void XWalkFaceTracking::validate(
    const XWalkComputerVisionCallbacks& providerCallbacks,
    const XWalkFaceTrackingConfiguration& configuration)
{
    if ((providerCallbacks.start == nullptr) ||
        (providerCallbacks.stop == nullptr) ||
        (providerCallbacks.setFace == nullptr) ||
        (providerCallbacks.observe == nullptr) ||
        (providerCallbacks.delay == nullptr) ||
        (providerCallbacks.continueOperation == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Face tracking requires complete callbacks");
    }
    const agent::boolean configurationInvalid =
        static_cast<agent::boolean>(
            !std::isfinite(configuration.correctionSpanDegrees) ||
        !std::isfinite(configuration.maximumAngleDegrees));
    if (configurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Face-tracking angles must be finite");
    }
    if ((configuration.frameWidthPixels < 16U) ||
        (configuration.frameWidthPixels > 7'680U) ||
        (configuration.frameHeightPixels < 16U) ||
        (configuration.frameHeightPixels > 4'320U) ||
        (configuration.correctionSpanDegrees <= 0.0) ||
        (configuration.correctionSpanDegrees > 180.0) ||
        (configuration.maximumAngleDegrees <= 0.0) ||
        (configuration.maximumAngleDegrees > 90.0) ||
        (configuration.sampleDelayMs == 0U) ||
        (configuration.sampleDelayMs > 1'000U) ||
        (configuration.finalDelayMs > 1'000U))
    {
        XHAL_THROW_OUT_OF_RANGE("Face-tracking configuration is out of range");
    }
}

agent::boolean XWalkFaceTracking::wait(agent::uint32 durationMs) const
{
    constexpr agent::uint32 cancellationIntervalMs{20U};
    agent::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        const agent::boolean operationRequested =
            callbacks.continueOperation(callbackContext);
        if (operationRequested == false)
        {
            return false;
        }
        const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        callbacks.delay(callbackContext, sliceMs);
        remainingMs -= sliceMs;
    }
    return callbacks.continueOperation(callbackContext);
}

agent::boolean XWalkFaceTracking::start()
{
    if (startedValue)
    {
        return true;
    }
    panAngleDegreesValue = 0.0;
    tiltAngleDegreesValue = 0.0;
    startedValue = callbacks.start(callbackContext);
    if (startedValue)
    {
        callbacks.setFace(callbackContext, true);
    }
    return startedValue;
}

void XWalkFaceTracking::finish()
{
    picarxObject->stop();
    if (startedValue)
    {
        callbacks.stop(callbackContext);
    }
    startedValue = false;
    panAngleDegreesValue = 0.0;
    tiltAngleDegreesValue = 0.0;
    callbacks.delay(callbackContext, configurationValue.finalDelayMs);
}

agent::float64 XWalkFaceTracking::panAngleDegrees() const noexcept
{
    return panAngleDegreesValue;
}

agent::float64 XWalkFaceTracking::tiltAngleDegrees() const noexcept
{
    return tiltAngleDegreesValue;
}

agent::boolean XWalkFaceTracking::started() const noexcept
{
    return startedValue;
}

} /* namespace xwalk::agent */
