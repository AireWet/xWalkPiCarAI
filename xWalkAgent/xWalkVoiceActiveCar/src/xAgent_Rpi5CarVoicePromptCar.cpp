/******************************************************************************
 * @file        xAgent_Rpi5CarVoicePromptCar.cpp
 * @brief       Implements the spoken PiCar-X movement demonstration.
 *
 * @details
 * Implements synchronous speech prompts, bounded vehicle movements,
 * cancellation checks, and safe shutdown.
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
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

#include "xAgent_Rpi5CarVoicePromptCar.h"

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"
#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains application coordinators for xWalk firmware. */
namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds caller-owned vehicle, speech, and callback services.
 * @param[in,out] picarx Vehicle coordinator that must outlive this object.
 * @param[in,out] textToSpeech Speech coordinator that must outlive this object.
 * @param[in,out] context Nullable callback context that must outlive callback use.
 * @param[in] backendCallbacks Complete synchronous application callbacks.
 * @param[in] carConfiguration Owned timing, speed, and steering settings.
 */
XWalkVoicePromptCar::XWalkVoicePromptCar(XWalkPicarx& picarx,
    hal::XWalkTextToSpeech& textToSpeech, hal::contextpointer context,
    const XWalkVoiceActiveCarCallbacks& backendCallbacks,
    const XWalkVoicePromptCarConfiguration& carConfiguration):
    picarxObject(&picarx), textToSpeechObject(&textToSpeech),
    callbackContext(context), callbacks(backendCallbacks),
    configuration(carConfiguration)
{
    validate(callbacks, configuration);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/** @brief Runs the four spoken movements once. @return Zero after safe shutdown. */
hal::int32 XWalkVoicePromptCar::run()
{
    XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
    textToSpeechObject->speak("Hello! I'm PiCar-X.");
    if (callbacks.shouldContinue(callbackContext))
    {
        drive("Moving forward", true);
    }
    if (callbacks.shouldContinue(callbackContext))
    {
        drive("Moving backward", false);
    }
    if (callbacks.shouldContinue(callbackContext))
    {
        turn("Turning left", -configuration.steeringAngle);
    }
    if (callbacks.shouldContinue(callbackContext))
    {
        turn("Turning right", configuration.steeringAngle);
    }
    stop();
    callbacks.output(callbackContext, "Voice-prompt car demonstration stopped");
    return 0;
}

/** @brief Stops the motors and centres steering. */
void XWalkVoicePromptCar::stop()
{
    picarxObject->stop();
    picarxObject->setDirectionServoAngle(0.0);
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Speaks and executes one straight movement.
 * @param[in] prompt Text spoken before movement.
 * @param[in] forward `true` for forward movement; `false` for backward.
 */
void XWalkVoicePromptCar::drive(hal::stringview prompt, hal::boolean forward)
{
    textToSpeechObject->speak(prompt);
    picarxObject->setDirectionServoAngle(0.0);
    if (forward)
    {
        picarxObject->forward(configuration.speedPercent);
    }
    else
    {
        picarxObject->backward(configuration.speedPercent);
    }
    callbacks.delay(callbackContext, configuration.driveDurationMs);
    picarxObject->stop();
}

/**
 * @brief Speaks and executes one steered movement.
 * @param[in] prompt Text spoken before movement.
 * @param[in] angle Signed steering angle in degrees.
 */
void XWalkVoicePromptCar::turn(hal::stringview prompt, hal::float64 angle)
{
    textToSpeechObject->speak(prompt);
    picarxObject->setDirectionServoAngle(angle);
    picarxObject->forward(configuration.speedPercent);
    callbacks.delay(callbackContext, configuration.driveDurationMs);
    picarxObject->stop();
    picarxObject->setDirectionServoAngle(0.0);
}

/**
 * @brief Validates callbacks and runtime configuration.
 * @param[in] backendCallbacks Application callback table.
 * @param[in] carConfiguration Movement configuration.
 * @throws std::invalid_argument If a required callback is null.
 * @throws std::out_of_range If a numeric value is outside its range.
 */
void XWalkVoicePromptCar::validate(
    const XWalkVoiceActiveCarCallbacks& backendCallbacks,
    const XWalkVoicePromptCarConfiguration& carConfiguration)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr) ||
        (backendCallbacks.delay == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-prompt-car callbacks must be complete");
    }
    if ((carConfiguration.speedPercent < 0.0) ||
        (carConfiguration.speedPercent > 100.0) ||
        (carConfiguration.steeringAngle <= 0.0) ||
        (carConfiguration.steeringAngle > 40.0) ||
        (carConfiguration.driveDurationMs == 0U))
    {
        XHAL_THROW_OUT_OF_RANGE("Voice-prompt-car configuration is outside its range");
    }
}

} /* namespace xwalk::agent */
