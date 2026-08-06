/******************************************************************************
 * @file        xAgent_Rpi5CarVoicePromptCarLifecycle.cpp
 * @brief       Implements spoken movement-example construction and validation.
 * @project     xWalk Firmware
 * @module      xWalkVoicePromptCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVoicePromptCar.h"

#include "xHal_Rpi5CarExceptions.h"

#include <cmath>

namespace xwalk::agent
{

XWalkVoicePromptCar::XWalkVoicePromptCar(XWalkPicarx& picarx,
    hal::XWalkTextToSpeech& textToSpeech, agent::contextpointer context,
    const XWalkVoicePromptCarCallbacks& backendCallbacks,
    const XWalkVoicePromptCarConfiguration& carConfiguration):
    picarxObject(&picarx), textToSpeechObject(&textToSpeech),
    callbackContext(context), callbacks(backendCallbacks),
    configuration(carConfiguration)
{
    validate(callbacks, configuration);
}

void XWalkVoicePromptCar::validate(
    const XWalkVoicePromptCarCallbacks& backendCallbacks,
    const XWalkVoicePromptCarConfiguration& carConfiguration)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr) ||
        (backendCallbacks.delay == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-prompt-car callbacks must be complete");
    }
    const agent::boolean vehicleConfigurationInvalid =
        static_cast<agent::boolean>(
            !std::isfinite(carConfiguration.speedPercent) ||
        !std::isfinite(carConfiguration.steeringAngle));
    if (vehicleConfigurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Voice-prompt-car configuration must be finite");
    }
    if ((carConfiguration.speedPercent < 0.0) ||
        (carConfiguration.speedPercent > 100.0) ||
        (carConfiguration.steeringAngle <= 0.0) ||
        (carConfiguration.steeringAngle > 40.0) ||
        (carConfiguration.driveDurationMs == 0U))
    {
        XHAL_THROW_OUT_OF_RANGE(
            "Voice-prompt-car configuration is outside its range");
    }
}

} /* namespace xwalk::agent */
