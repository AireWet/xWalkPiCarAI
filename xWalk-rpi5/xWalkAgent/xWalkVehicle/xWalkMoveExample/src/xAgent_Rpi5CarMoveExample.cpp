/******************************************************************************
 * @file        xAgent_Rpi5CarMoveExample.cpp
 * @brief       Implements the source-compatible bounded movement sequence.
 * @project     xWalk Firmware
 * @module      xWalkMoveExample
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarMoveExample.h"

namespace xwalk::agent
{

void XWalkMoveExample::setServoAngle(agent::uint8 servoId, agent::float64 angleDegrees)
{
    if (servoId == 0U)
    {
        picarxObject->setDirectionServoAngle(angleDegrees);
    }
    else if (servoId == 1U)
    {
        picarxObject->setCameraPanAngle(angleDegrees);
    }
    else
    {
        picarxObject->setCameraTiltAngle(angleDegrees);
    }
}

agent::boolean XWalkMoveExample::sweepServo(agent::uint8 servoId)
{
    for (agent::int32 angle = 0; angle < 35; ++angle)
    {
        setServoAngle(servoId, static_cast<agent::float64>(angle));
        const agent::boolean delayCompleted = wait(10U);
        if (delayCompleted == false)
        {
            return false;
        }
    }
    for (agent::int32 angle = 35; angle > -35; --angle)
    {
        setServoAngle(servoId, static_cast<agent::float64>(angle));
        const agent::boolean delayCompleted = wait(10U);
        if (delayCompleted == false)
        {
            return false;
        }
    }
    for (agent::int32 angle = -35; angle < 0; ++angle)
    {
        setServoAngle(servoId, static_cast<agent::float64>(angle));
        const agent::boolean delayCompleted = wait(10U);
        if (delayCompleted == false)
        {
            return false;
        }
    }
    return true;
}

agent::boolean XWalkMoveExample::run()
{
    const agent::boolean operationRequested = continueCallback(callbackContext);
    if (operationRequested == false)
    {
        stop();
        return false;
    }

    picarxObject->forward(30.0);
    const agent::boolean movementDelayCompleted = wait(500U);
    if (movementDelayCompleted == false)
    {
        stop();
        return false;
    }
    const agent::boolean steeringSweepCompleted = sweepServo(0U);
    if (steeringSweepCompleted == false)
    {
        stop();
        return false;
    }

    picarxObject->stop();
    const agent::boolean pauseCompleted = wait(1'000U);
    if (pauseCompleted == false)
    {
        stop();
        return false;
    }
    const agent::boolean panSweepCompleted = sweepServo(1U);
    if (panSweepCompleted == false)
    {
        stop();
        return false;
    }
    const agent::boolean tiltSweepCompleted = sweepServo(2U);
    if (tiltSweepCompleted == false)
    {
        stop();
        return false;
    }

    picarxObject->stop();
    return wait(200U);
}

} /* namespace xwalk::agent */
