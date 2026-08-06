/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceControlledCarLifecycle.cpp
 * @brief       Implements voice-controlled-car lifecycle and validation.
 * @project     xWalk Firmware
 * @module      xWalkVoiceControlledCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceControlledCar.h"

#include "xHal_Rpi5CarExceptions.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace xwalk::agent
{

XWalkVoiceControlledCar::XWalkVoiceControlledCar(XWalkPicarx& picarx,
    hal::XWalkSpeechToText& speechToText, agent::contextpointer context,
    const XWalkVoiceControlledCarCallbacks& backendCallbacks,
    const XWalkVoiceControlledCarConfiguration& carConfiguration):
    picarxObject(&picarx), speechToTextObject(&speechToText),
    callbackContext(context), callbacks(backendCallbacks),
    configuration(carConfiguration)
{
    validate(callbacks, configuration);
}

agent::string XWalkVoiceControlledCar::normalize(agent::stringview text)
{
    agent::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](char value)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    });
    const auto isNotSpace = [](char value)
    {
        return std::isspace(static_cast<unsigned char>(value)) == 0;
    };
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), isNotSpace));
    result.erase(std::find_if(result.rbegin(), result.rend(), isNotSpace).base(),
        result.end());
    return result;
}

void XWalkVoiceControlledCar::validate(
    const XWalkVoiceControlledCarCallbacks& backendCallbacks,
    const XWalkVoiceControlledCarConfiguration& carConfiguration)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr) ||
        (backendCallbacks.delay == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-controlled-car callbacks must be complete");
    }
    const agent::boolean carConfigurationWakeWordSleepWordInvalid =
        static_cast<agent::boolean>(
            carConfiguration.wakeWord.empty() || carConfiguration.sleepWord.empty());
    if (carConfigurationWakeWordSleepWordInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-controlled-car phrases must not be empty");
    }
    const agent::boolean vehicleConfigurationInvalid =
        static_cast<agent::boolean>(
            !std::isfinite(carConfiguration.speedPercent) ||
        !std::isfinite(carConfiguration.steeringAngle) ||
        (carConfiguration.speedPercent < 0.0) ||
        (carConfiguration.speedPercent > 100.0) ||
        (carConfiguration.steeringAngle <= 0.0) ||
        (carConfiguration.steeringAngle > 40.0) ||
        (carConfiguration.driveDurationMs == 0U) ||
        (carConfiguration.listenTimeoutMs == 0U) ||
        (carConfiguration.listenTimeoutMs >
            XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS));
    if (vehicleConfigurationInvalid)
    {
        XHAL_THROW_OUT_OF_RANGE("Voice-controlled-car configuration is outside its range");
    }
}

} /* namespace xwalk::agent */
