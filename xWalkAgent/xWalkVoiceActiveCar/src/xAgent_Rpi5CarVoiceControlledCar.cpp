/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceControlledCar.cpp
 * @brief       Implements wake-word voice control for PiCar-X.
 *
 * @details
 * Implements recognition-session control, transcript classification, bounded
 * movement, cancellation, and safe shutdown.
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

#include "xAgent_Rpi5CarVoiceControlledCar.h"

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"
#include "xHal_Rpi5CarExceptions.h"

#include <algorithm>
#include <cctype>

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
 * @brief Binds caller-owned vehicle, recognition, and callback services.
 * @param[in,out] picarx Vehicle coordinator that must outlive this object.
 * @param[in,out] speechToText Recognition coordinator that must outlive this object.
 * @param[in,out] context Nullable callback context that must outlive callback use.
 * @param[in] backendCallbacks Complete synchronous application callbacks.
 * @param[in] carConfiguration Owned wake, sleep, timing, speed, and steering settings.
 */
XWalkVoiceControlledCar::XWalkVoiceControlledCar(XWalkPicarx& picarx,
    hal::XWalkSpeechToText& speechToText, hal::contextpointer context,
    const XWalkVoiceActiveCarCallbacks& backendCallbacks,
    const XWalkVoiceControlledCarConfiguration& carConfiguration):
    picarxObject(&picarx), speechToTextObject(&speechToText),
    callbackContext(context), callbacks(backendCallbacks),
    configuration(carConfiguration)
{
    validate(callbacks, configuration);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/** @brief Runs wake-word recognition until cancellation. @return Zero after safe shutdown. */
hal::int32 XWalkVoiceControlledCar::run()
{
    XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
    callbacks.output(callbackContext,
        hal::string("Voice control ready; say '") + configuration.wakeWord + "'");
    while (callbacks.shouldContinue(callbackContext))
    {
        const hal::string wakeTranscript =
            speechToTextObject->listen(configuration.listenTimeoutMs);
        if (!containsWakeWord(wakeTranscript, configuration.wakeWord))
        {
            continue;
        }
        callbacks.output(callbackContext,
            "Wake word detected; listening for commands");
        while (callbacks.shouldContinue(callbackContext))
        {
            const hal::string transcript =
                speechToTextObject->listen(configuration.listenTimeoutMs);
            const XWalkVoiceControlledCarCommand command =
                classifyCommand(transcript, configuration.sleepWord);
            if (command == XWalkVoiceControlledCarCommand::Unknown)
            {
                continue;
            }
            execute(command);
            if (command == XWalkVoiceControlledCarCommand::Sleep)
            {
                callbacks.output(callbackContext,
                    "Sleeping; say the wake word to resume");
                break;
            }
        }
    }
    stop();
    callbacks.output(callbackContext, "Voice-controlled car stopped");
    return 0;
}

/** @brief Requests recognition shutdown, stops the motors, and centres steering. */
void XWalkVoiceControlledCar::stop()
{
    speechToTextObject->stop();
    picarxObject->stop();
    picarxObject->setDirectionServoAngle(0.0);
}

/**
 * @brief Classifies one transcript by case-insensitive keyword matching.
 * @param[in] transcript Recognized text to inspect.
 * @param[in] sleepWord Non-empty session-closing phrase.
 * @return Supported command or `Unknown`.
 * @throws std::invalid_argument If `sleepWord` is empty.
 */
XWalkVoiceControlledCarCommand XWalkVoiceControlledCar::classifyCommand(
    hal::stringview transcript, hal::stringview sleepWord)
{
    if (sleepWord.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-controlled-car sleep word must not be empty");
    }
    const hal::string normalized = lowercase(transcript);
    const hal::string normalizedSleepWord = lowercase(sleepWord);
    if (normalized.find(normalizedSleepWord) != hal::string::npos)
    {
        return XWalkVoiceControlledCarCommand::Sleep;
    }
    if (normalized.find("forward") != hal::string::npos)
    {
        return XWalkVoiceControlledCarCommand::Forward;
    }
    if (normalized.find("backward") != hal::string::npos)
    {
        return XWalkVoiceControlledCarCommand::Backward;
    }
    if (normalized.find("left") != hal::string::npos)
    {
        return XWalkVoiceControlledCarCommand::Left;
    }
    if (normalized.find("right") != hal::string::npos)
    {
        return XWalkVoiceControlledCarCommand::Right;
    }
    return XWalkVoiceControlledCarCommand::Unknown;
}

/**
 * @brief Tests one transcript for a case-insensitive wake phrase.
 * @param[in] transcript Recognized text to inspect.
 * @param[in] wakeWord Non-empty wake phrase to find.
 * @return `true` when found; otherwise `false`.
 * @throws std::invalid_argument If `wakeWord` is empty.
 */
hal::boolean XWalkVoiceControlledCar::containsWakeWord(
    hal::stringview transcript, hal::stringview wakeWord)
{
    if (wakeWord.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-controlled-car wake word must not be empty");
    }
    return lowercase(transcript).find(lowercase(wakeWord)) != hal::string::npos;
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/** @brief Executes one recognized command. @param[in] command Command to apply. */
void XWalkVoiceControlledCar::execute(XWalkVoiceControlledCarCommand command)
{
    if (command == XWalkVoiceControlledCarCommand::Sleep)
    {
        picarxObject->stop();
        picarxObject->setDirectionServoAngle(0.0);
        return;
    }
    if (command == XWalkVoiceControlledCarCommand::Forward)
    {
        picarxObject->setDirectionServoAngle(0.0);
        picarxObject->forward(configuration.speedPercent);
    }
    else if (command == XWalkVoiceControlledCarCommand::Backward)
    {
        picarxObject->setDirectionServoAngle(0.0);
        picarxObject->backward(configuration.speedPercent);
    }
    else if (command == XWalkVoiceControlledCarCommand::Left)
    {
        picarxObject->setDirectionServoAngle(-configuration.steeringAngle);
        picarxObject->forward(configuration.speedPercent);
    }
    else if (command == XWalkVoiceControlledCarCommand::Right)
    {
        picarxObject->setDirectionServoAngle(configuration.steeringAngle);
        picarxObject->forward(configuration.speedPercent);
    }
    else
    {
        return;
    }
    callbacks.delay(callbackContext, configuration.driveDurationMs);
    picarxObject->stop();
    if ((command == XWalkVoiceControlledCarCommand::Left) ||
        (command == XWalkVoiceControlledCarCommand::Right))
    {
        picarxObject->setDirectionServoAngle(0.0);
    }
}

/**
 * @brief Validates callbacks and runtime configuration.
 * @param[in] backendCallbacks Application callback table.
 * @param[in] carConfiguration Recognition and movement configuration.
 * @throws std::invalid_argument If a callback or phrase is missing.
 * @throws std::out_of_range If a numeric value is outside its range.
 */
void XWalkVoiceControlledCar::validate(
    const XWalkVoiceActiveCarCallbacks& backendCallbacks,
    const XWalkVoiceControlledCarConfiguration& carConfiguration)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr) ||
        (backendCallbacks.delay == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-controlled-car callbacks must be complete");
    }
    if (carConfiguration.wakeWord.empty() || carConfiguration.sleepWord.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-controlled-car phrases must not be empty");
    }
    if ((carConfiguration.speedPercent < 0.0) ||
        (carConfiguration.speedPercent > 100.0) ||
        (carConfiguration.steeringAngle <= 0.0) ||
        (carConfiguration.steeringAngle > 40.0) ||
        (carConfiguration.driveDurationMs == 0U) ||
        (carConfiguration.listenTimeoutMs == 0U) ||
        (carConfiguration.listenTimeoutMs >
            XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        XHAL_THROW_OUT_OF_RANGE("Voice-controlled-car configuration is outside its range");
    }
}

/**
 * @brief Returns a lowercase ASCII copy for phrase matching.
 * @param[in] text Transcript or phrase to normalize.
 * @return Owned lowercase copy.
 */
hal::string XWalkVoiceControlledCar::lowercase(hal::stringview text)
{
    hal::string result(text);
    std::transform(result.begin(), result.end(), result.begin(), [](char value)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    });
    return result;
}

} /* namespace xwalk::agent */
