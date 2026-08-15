/******************************************************************************
 * @file        xAgent_Rpi5CarObstacleAvoidance.cpp
 * @brief       Implements distance-band obstacle decisions.
 *
 * @details
 * Preserves the upstream thresholds, steering angles, power, and turn delays,
 * while stopping instead of reversing when ultrasonic acquisition fails.
 *
 * @project     xWalk Firmware
 * @module      xWalkObstacleAvoidance
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xAgent_Rpi5CarObstacleAvoidance.h"

#include "xHal_Rpi5CarCommonFunctions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Applies one source-compatible decision for a measured distance.
     * @param[in] distanceCm Ultrasonic distance in centimeters; non-positive values are invalid.
     * @return The applied movement band, sensor failure, or cancellation result.
     * @warning Successful decisions may move the physical vehicle at 50-percent requested speed.
     */
    XWalkObstacleAvoidanceResult XWalkObstacleAvoidance::step(agent::float64 distanceCm)
    {
        constexpr agent::float64 safeDistanceCm{40.0};
        constexpr agent::float64 dangerDistanceCm{20.0};
        constexpr agent::float64 powerPercent{50.0};

        const agent::boolean distanceCmInvalid =
            static_cast<agent::boolean>(!XHAL_IS_FINITE(distanceCm) || (distanceCm <= 0.0));
        if (distanceCmInvalid)
        {
            picarxObject->stop();
            return XWalkObstacleAvoidanceResult::SensorInvalid;
        }
        const agent::boolean operationRequested = continueCallback(callbackContext);
        if (operationRequested == false)
        {
            picarxObject->stop();
            return XWalkObstacleAvoidanceResult::Cancelled;
        }
        if (distanceCm >= safeDistanceCm)
        {
            picarxObject->setDirectionServoAngle(0.0);
            picarxObject->forward(powerPercent);
            return XWalkObstacleAvoidanceResult::Forward;
        }
        if (distanceCm >= dangerDistanceCm)
        {
            picarxObject->setDirectionServoAngle(30.0);
            picarxObject->forward(powerPercent);
            const agent::boolean reverseDelayCompleted = wait(100U);
            if (reverseDelayCompleted == false)
            {
                picarxObject->stop();
                return XWalkObstacleAvoidanceResult::Cancelled;
            }
            return XWalkObstacleAvoidanceResult::TurnRight;
        }

        picarxObject->setDirectionServoAngle(-30.0);
        picarxObject->backward(powerPercent);
        const agent::boolean turnDelayCompleted = wait(500U);
        if (turnDelayCompleted == false)
        {
            picarxObject->stop();
            return XWalkObstacleAvoidanceResult::Cancelled;
        }
        return XWalkObstacleAvoidanceResult::ReverseLeft;
    }

    /**
     * @brief Stops both drive motors without changing steering.
     * @post Both motor commands are zero after a successful backend operation.
     */
    void XWalkObstacleAvoidance::stop()
    {
        picarxObject->stop();
    }

} /* namespace xwalk::agent */
