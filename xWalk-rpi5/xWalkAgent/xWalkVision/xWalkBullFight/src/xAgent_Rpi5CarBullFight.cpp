/******************************************************************************
 * @file        xAgent_Rpi5CarBullFight.cpp
 * @brief       Implements one red-target pursuit step.
 * @project     xWalk Firmware
 * @module      xWalkBullFight
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarBullFight.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::agent
{

    agent::float64 XWalkBullFight::constrainCameraAngle(agent::float64 angleDegrees) const noexcept
    {
        if (angleDegrees > configurationValue.maximumCameraAngleDegrees)
        {
            return configurationValue.maximumCameraAngleDegrees;
        }
        if (angleDegrees < -configurationValue.maximumCameraAngleDegrees)
        {
            return -configurationValue.maximumCameraAngleDegrees;
        }
        return angleDegrees;
    }

    XWalkBullFightResult XWalkBullFight::step()
    {
        if (!startedValue)
        {
            XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Bull fight must be started before stepping");
        }
        XWalkBullFightResult result{};
        const agent::boolean operationRequested = callbacks.continueOperation(callbackContext);
        if (operationRequested == false)
        {
            picarxObject->stop();
            result.state = XWalkBullFightState::Cancelled;
            return result;
        }

        result.target = callbacks.observe(callbackContext).color;
        if (result.target.count > 0U)
        {
            const agent::float64 halfSpan = configurationValue.correctionSpanDegrees / 2.0;
            panAngleDegreesValue = constrainCameraAngle(
                panAngleDegreesValue +
                (static_cast<agent::float64>(result.target.centerX) * configurationValue.correctionSpanDegrees /
                 static_cast<agent::float64>(configurationValue.frameWidthPixels)) -
                halfSpan);
            tiltAngleDegreesValue = constrainCameraAngle(
                tiltAngleDegreesValue + halfSpan -
                (static_cast<agent::float64>(result.target.centerY) * configurationValue.correctionSpanDegrees /
                 static_cast<agent::float64>(configurationValue.frameHeightPixels)));
            if (directionAngleDegreesValue > panAngleDegreesValue)
            {
                directionAngleDegreesValue -= 1.0;
            }
            else if (directionAngleDegreesValue < panAngleDegreesValue)
            {
                directionAngleDegreesValue += 1.0;
            }
            picarxObject->setCameraPanAngle(panAngleDegreesValue);
            picarxObject->setCameraTiltAngle(tiltAngleDegreesValue);
            picarxObject->setDirectionServoAngle(panAngleDegreesValue);
            picarxObject->forward(configurationValue.speedPercent);
            result.state = XWalkBullFightState::Pursuing;
        }
        else
        {
            picarxObject->forward(0.0);
        }
        result.panAngleDegrees = panAngleDegreesValue;
        result.tiltAngleDegrees = tiltAngleDegreesValue;
        result.directionAngleDegrees = directionAngleDegreesValue;
        const agent::boolean delayCompleted = wait(configurationValue.sampleDelayMs);
        if (delayCompleted == false)
        {
            picarxObject->stop();
            result.state = XWalkBullFightState::Cancelled;
        }
        return result;
    }

} /* namespace xwalk::agent */
