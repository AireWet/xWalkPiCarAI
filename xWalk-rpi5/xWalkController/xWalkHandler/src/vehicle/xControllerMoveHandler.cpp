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
     *
     * @details Refreshes a bounded forward or backward motor request every 250 milliseconds, observes
     * cancellation through the existing sliced delay, and stops both motors on every bounded-move exit.
     * Zero-duration requests proceed directly to the stopped state. Demo requests retain their dedicated flow.
     *
     * @param[in] request Validated action, speed percentage, and duration.
     * @return Zero after movement and the final stop complete.
     */
    ::ctrl::int32 XWalkController::XWALK_handlerMove(const XWalkMoveRequest& request)
    {
        if (request.action == XWalkMoveAction::Demo)
        {
            return XWALK_handlerMoveExample();
        }
        constexpr ::ctrl::uint32 refreshIntervalMs{250U};
        ::ctrl::uint32 remainingMs = request.durationMs;
        ::ctrl::boolean operationRequested{true};
        while ((remainingMs > 0U) && operationRequested)
        {
            operationRequested = operationMayContinue();
            if (operationRequested == false)
            {
                break;
            }

            if (request.action == XWalkMoveAction::Forward)
            {
                picarxObject->forward(request.speedPercent);
            }
            else
            {
                picarxObject->backward(request.speedPercent);
            }

            const ::ctrl::uint32 sliceMs = (remainingMs < refreshIntervalMs) ? remainingMs : refreshIntervalMs;
            const ::ctrl::boolean delayCompleted = delayWhileOperationRequested(sliceMs);
            if (delayCompleted == false)
            {
                break;
            }

            remainingMs -= sliceMs;
        }
        picarxObject->stop();
        return 0;
    }

} /* namespace xwalk::ctrl */
