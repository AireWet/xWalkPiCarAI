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
     * @details Refreshes a bounded forward or backward motor request on a 250-millisecond monotonic schedule,
     * observes cancellation through the existing sliced delay, and stops both motors on every bounded-move exit.
     * Absolute deadlines prevent backend and scheduling overhead from extending the requested duration.
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
        constexpr ::ctrl::uint64 refreshIntervalMs{250U};
        const ::ctrl::uint64 startMs = callbacks.monotonicMilliseconds(callbackContext);
        const ::ctrl::uint64 durationMs = static_cast<::ctrl::uint64>(request.durationMs);
        const ::ctrl::uint64 endMs = startMs + durationMs;
        ::ctrl::uint64 currentMs = startMs;
        ::ctrl::uint64 nextRefreshMs = startMs;
        ::ctrl::boolean operationRequested{true};
        while ((currentMs < endMs) && operationRequested)
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

            currentMs = callbacks.monotonicMilliseconds(callbackContext);
            if (currentMs >= endMs)
            {
                break;
            }

            nextRefreshMs += refreshIntervalMs;
            if (nextRefreshMs <= currentMs)
            {
                const ::ctrl::uint64 elapsedPastRefreshMs = currentMs - nextRefreshMs;
                const ::ctrl::uint64 skippedRefreshCount = (elapsedPastRefreshMs / refreshIntervalMs) + 1U;
                nextRefreshMs += skippedRefreshCount * refreshIntervalMs;
            }

            const ::ctrl::uint64 wakeMs = (nextRefreshMs < endMs) ? nextRefreshMs : endMs;
            const ::ctrl::uint64 delayDurationMs = wakeMs - currentMs;
            const ::ctrl::boolean delayCompleted =
                delayWhileOperationRequested(static_cast<::ctrl::uint32>(delayDurationMs));
            if (delayCompleted == false)
            {
                break;
            }

            currentMs = callbacks.monotonicMilliseconds(callbackContext);
        }
        picarxObject->stop();
        return 0;
    }

} /* namespace xwalk::ctrl */
