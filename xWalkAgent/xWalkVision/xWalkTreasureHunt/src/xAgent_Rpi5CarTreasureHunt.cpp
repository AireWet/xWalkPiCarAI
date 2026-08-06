/******************************************************************************
 * @file        xAgent_Rpi5CarTreasureHunt.cpp
 * @brief       Implements target detection and interactive treasure-hunt movement.
 *
 * @project     xWalk Firmware
 * @module      xWalkTreasureHunt
 *
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 *
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

#include "xAgent_Rpi5CarTreasureHunt.h"

#include "xHal_Rpi5CarExceptions.h"

#include <cctype>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/** @namespace xwalk::agent @brief Contains xWalk application coordinators. */
namespace xwalk::agent
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Checks the target and applies one source-compatible keyboard command.
 * @param[in] key One operator key, `space`, or `quit`.
 * @return Observation and action completed during this bounded step.
 * @warning Movement keys drive the physical car at configured requested power.
 */
XWalkTreasureHuntResult XWalkTreasureHunt::step(const agent::string& key)
{
    if (!startedValue)
    {
        XHAL_THROW_LOGIC_ERROR("Treasure hunt must be started before stepping");
    }
    XWalkTreasureHuntResult result{};
    result.targetColor = targetColorValue;
    const agent::boolean operationRequested =
        callbacks.vision.continueOperation(callbackContext);
    if (operationRequested == false)
    {
        picarxObject->stop();
        result.action = XWalkTreasureHuntAction::Cancelled;
        return result;
    }

    const XWalkComputerVisionDetection detection =
        callbacks.vision.observe(callbackContext).color;
    if ((detection.count != 0U) &&
        (detection.width > configurationValue.detectionWidthThresholdPixels))
    {
        result.targetFound = true;
        textToSpeechObject->speak("Well done!");
        const agent::boolean delayCompleted = wait(configurationValue.promptDelayMs);
        if (delayCompleted == false)
        {
            picarxObject->stop();
            result.action = XWalkTreasureHuntAction::Cancelled;
            return result;
        }
        renewTarget();
        result.targetColor = targetColorValue;
    }

    agent::string normalizedKey = key;
    for (char& value : normalizedKey)
    {
        value = static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    }
    if ((normalizedKey == "quit") || (normalizedKey == "exit"))
    {
        result.action = XWalkTreasureHuntAction::Quit;
        return result;
    }
    if ((normalizedKey == "space") || (normalizedKey == " "))
    {
        textToSpeechObject->speak("Look for " + colorName(targetColorValue) + "!");
        result.action = XWalkTreasureHuntAction::TargetRepeated;
    }
    else {
        const agent::boolean normalizedKeyWAInvalid =
            static_cast<agent::boolean>(
                (normalizedKey.size() == 1U) &&
            ((normalizedKey[0U] == 'w') || (normalizedKey[0U] == 'a') ||
                (normalizedKey[0U] == 's') || (normalizedKey[0U] == 'd')));
            if (normalizedKeyWAInvalid)
    {
        result.action = move(normalizedKey[0U]) ?
            XWalkTreasureHuntAction::Moved :
            XWalkTreasureHuntAction::Cancelled;
    }
    }
    if (result.action != XWalkTreasureHuntAction::Cancelled)
    {
        const agent::boolean delayCompleted = wait(configurationValue.loopDelayMs);
        if (delayCompleted == false)
        {
            picarxObject->stop();
            result.action = XWalkTreasureHuntAction::Cancelled;
        }
    }
    return result;
}

/**
 * @brief Returns a lowercase display name for one selectable color.
 * @param[in] color Source-compatible color selection.
 * @return Lowercase color name.
 * @throws std::invalid_argument If `color` is closed or invalid.
 */
agent::string XWalkTreasureHunt::colorName(XWalkComputerVisionColor color)
{
    switch (color)
    {
        case XWalkComputerVisionColor::Red:
            return "red";
        case XWalkComputerVisionColor::Orange:
            return "orange";
        case XWalkComputerVisionColor::Yellow:
            return "yellow";
        case XWalkComputerVisionColor::Green:
            return "green";
        case XWalkComputerVisionColor::Blue:
            return "blue";
        case XWalkComputerVisionColor::Purple:
            return "purple";
        case XWalkComputerVisionColor::Close:
        default:
            XHAL_THROW_INVALID_ARGUMENT("Treasure-hunt target color is invalid");
    }
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Applies one source-compatible movement key and bounded stop.
 * @param[in] key Lowercase keyboard command.
 * @return `true` for a recognized movement key; otherwise `false`.
 */
agent::boolean XWalkTreasureHunt::move(char key)
{
    if ((key != 'w') && (key != 'a') && (key != 's') && (key != 'd'))
    {
        return false;
    }
    if (key == 'a')
    {
        picarxObject->setDirectionServoAngle(-configurationValue.turnAngleDegrees);
        picarxObject->forward(configurationValue.driveSpeedPercent);
    }
    else if (key == 'd')
    {
        picarxObject->setDirectionServoAngle(configurationValue.turnAngleDegrees);
        picarxObject->forward(configurationValue.driveSpeedPercent);
    }
    else
    {
        picarxObject->setDirectionServoAngle(0.0);
        if (key == 'w')
        {
            picarxObject->forward(configurationValue.driveSpeedPercent);
        }
        else
        {
            picarxObject->backward(configurationValue.driveSpeedPercent);
        }
    }
    const agent::boolean completed = wait(configurationValue.movementDelayMs);
    picarxObject->stop();
    return completed;
}

} /* namespace xwalk::agent */
