/******************************************************************************
 * @file        xControllerObstacleAvoidanceHandler.cpp
 * @brief       Implements the ObstacleAvoidanceHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xControllerParsing.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

/**
 * @brief Runs foreground obstacle avoidance ported from `4.avoiding_obstacles.py`.
 * @param[in] request Validated lifecycle action.
 * @return Zero after stop or cancellation, two for sensor failure, or three without an Agent.
 * @warning Start may continuously move the physical vehicle until cancellation.
 */
::ctrl::int32 XWalkController::XWALK_handlerObstacleAvoidance(
    const XWalkLifecycleRequest& request)
{
    if (obstacleAvoidanceObject == nullptr)
    {
        output("Obstacle-avoidance backend unavailable.");
        return 3;
    }
    if (request.action == XWalkLifecycleAction::Stop)
    {
        obstacleAvoidanceObject->stop();
        output("Obstacle avoidance stopped.");
        return 0;
    }

    while (true)
    {
        const ::ctrl::float64 distanceCm = picarxObject->distance();
        output("distance: " + XWALK_formatOneDecimal(distanceCm));
        const agent::XWalkObstacleAvoidanceResult result =
            obstacleAvoidanceObject->step(distanceCm);
        if (result == agent::XWalkObstacleAvoidanceResult::SensorInvalid)
        {
            output("Obstacle avoidance stopped: invalid ultrasonic sample.");
            return 2;
        }
        if (result == agent::XWalkObstacleAvoidanceResult::Cancelled)
        {
            obstacleAvoidanceObject->stop();
            output("Obstacle avoidance stopped.");
            return 0;
        }
    }
}

} /* namespace xwalk::ctrl */
