/******************************************************************************
 * @file        xAgent_Rpi5CarVoicePromptCar.cpp
 * @brief       Implements the spoken PiCar-X movement demonstration.
 * @project     xWalk Firmware
 * @module      xWalkVoicePromptCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVoicePromptCar.h"

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

namespace xwalk::agent
{

agent::int32 XWalkVoicePromptCar::run()
{
    XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
    textToSpeechObject->speak("Hello! I'm PiCar-X.");
    const agent::boolean forwardMovementAllowed =
        static_cast<agent::boolean>(
            callbacks.shouldContinue(callbackContext));
    if (forwardMovementAllowed)
    {
        drive("Moving forward", true);
    }
    const agent::boolean backwardMovementAllowed =
        static_cast<agent::boolean>(
            callbacks.shouldContinue(callbackContext));
    if (backwardMovementAllowed)
    {
        drive("Moving backward", false);
    }
    const agent::boolean leftTurnAllowed =
        static_cast<agent::boolean>(
            callbacks.shouldContinue(callbackContext));
    if (leftTurnAllowed)
    {
        turn("Turning left", -configuration.steeringAngle);
    }
    const agent::boolean rightTurnAllowed =
        static_cast<agent::boolean>(
            callbacks.shouldContinue(callbackContext));
    if (rightTurnAllowed)
    {
        turn("Turning right", configuration.steeringAngle);
    }
    stop();
    callbacks.output(callbackContext, "Voice-prompt car demonstration stopped");
    return 0;
}

void XWalkVoicePromptCar::stop()
{
    picarxObject->stop();
    picarxObject->setDirectionServoAngle(0.0);
}

void XWalkVoicePromptCar::drive(agent::stringview prompt, agent::boolean forward)
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

void XWalkVoicePromptCar::turn(agent::stringview prompt, agent::float64 angle)
{
    textToSpeechObject->speak(prompt);
    picarxObject->setDirectionServoAngle(angle);
    picarxObject->forward(configuration.speedPercent);
    callbacks.delay(callbackContext, configuration.driveDurationMs);
    picarxObject->stop();
    picarxObject->setDirectionServoAngle(0.0);
}

} /* namespace xwalk::agent */
