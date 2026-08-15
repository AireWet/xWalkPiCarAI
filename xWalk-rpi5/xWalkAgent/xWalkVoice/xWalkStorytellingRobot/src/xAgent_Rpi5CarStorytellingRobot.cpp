/******************************************************************************
 * @file        xAgent_Rpi5CarStorytellingRobot.cpp
 * @brief       Implements the storytelling movement and speech sequence.
 * @project     xWalk Firmware
 * @module      xWalkStorytellingRobot
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarStorytellingRobot.h"

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

namespace xwalk::agent
{

    agent::int32 XWalkStorytellingRobot::run()
    {
        XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
        textToSpeechObject->speak(configuration.greeting);
        const agent::boolean firstLegRequested = callbacks.shouldContinue(callbackContext);
        if (firstLegRequested == false)
        {
            stop();
            return 0;
        }
        const agent::boolean firstLegCompleted = driveForwardAndNarrate(configuration.firstJoke);
        if (firstLegCompleted == false)
        {
            stop();
            return 0;
        }
        const agent::boolean secondLegRequested = callbacks.shouldContinue(callbackContext);
        if (secondLegRequested == false)
        {
            stop();
            return 0;
        }
        const agent::boolean secondLegCompleted = driveForwardAndNarrate(configuration.secondJoke);
        if (secondLegCompleted == false)
        {
            stop();
            return 0;
        }
        const agent::boolean operationRequested = callbacks.shouldContinue(callbackContext);
        if (operationRequested == false)
        {
            stop();
            return 0;
        }

        textToSpeechObject->speak(configuration.farewell);
        picarxObject->backward(configuration.speedPercent);
        static_cast<void>(wait(configuration.homeLegDurationMs));
        stop();
        return 0;
    }

    void XWalkStorytellingRobot::stop()
    {
        picarxObject->stop();
        picarxObject->setDirectionServoAngle(0.0);
    }

    agent::boolean XWalkStorytellingRobot::driveForwardAndNarrate(agent::stringview narration)
    {
        picarxObject->forward(configuration.speedPercent);
        const agent::boolean completed = wait(configuration.outwardLegDurationMs);
        picarxObject->stop();
        if (completed)
        {
            textToSpeechObject->speak(narration);
        }
        return completed;
    }

} /* namespace xwalk::agent */
