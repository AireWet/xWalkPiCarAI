/******************************************************************************
 * @file        xControllerMoveHandler.cpp
 * @brief       Implements the MoveHandler command responsibility.
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
 * @brief Executes the move command.
 * @param[in] request Validated action, speed percentage, and duration.
 * @return Zero after movement and the final stop complete.
 */
::ctrl::int32 XWalkController::XWALK_handlerMove(const XWalkMoveRequest& request)
{
    if (request.action == XWalkMoveAction::Demo)
    {
        return XWALK_handlerMoveExample();
    }
    const ::ctrl::boolean operationRequested = operationMayContinue();
    if (operationRequested == false)
    {
        return 0;
    }
    if (request.action == XWalkMoveAction::Forward)
    {
        picarxObject->forward(request.speedPercent);
    }
    else
    {
        picarxObject->backward(request.speedPercent);
    }
    static_cast<void>(delayWhileOperationRequested(request.durationMs));
    picarxObject->stop();
    return 0;
}

} /* namespace xwalk::ctrl */
