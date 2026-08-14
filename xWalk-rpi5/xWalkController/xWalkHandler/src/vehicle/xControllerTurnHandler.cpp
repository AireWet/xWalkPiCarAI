/******************************************************************************
 * @file        xControllerTurnHandler.cpp
 * @brief       Implements the TurnHandler command responsibility.
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
 * @brief Executes the turn command.
 * @param[in] request Validated direction and steering-angle magnitude.
 * @return Zero after the fixed turn sequence and final stop complete.
 */
::ctrl::int32 XWalkController::XWALK_handlerTurn(const XWalkTurnRequest& request)
{
    const ::ctrl::float64 signedAngle = (request.direction == XWalkTurnDirection::Right) ?
        request.angleDegrees : -request.angleDegrees;
    const ::ctrl::boolean operationRequested = operationMayContinue();
    if (operationRequested == false)
    {
        return 0;
    }
    picarxObject->setDirectionServoAngle(signedAngle);
    const ::ctrl::boolean steeringDelayCompleted =
        delayWhileOperationRequested(300U);
    if (steeringDelayCompleted == false)
    {
        return 0;
    }
    picarxObject->forward(30.0);
    const ::ctrl::boolean movementDelayCompleted =
        delayWhileOperationRequested(800U);
    if (movementDelayCompleted == false)
    {
        return 0;
    }
    picarxObject->setDirectionServoAngle(0.0);
    static_cast<void>(delayWhileOperationRequested(300U));
    picarxObject->stop();
    return 0;
}

} /* namespace xwalk::ctrl */
