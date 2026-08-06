/******************************************************************************
 * @file        xControllerCommandSafety.cpp
 * @brief       Implements the CommandSafety command responsibility.
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
 * @brief Reports whether the active command may continue and latches emergency stop otherwise.
 * @return `true` while the application permits another bounded step; otherwise `false`.
 */
::ctrl::boolean XWalkController::operationMayContinue()
{
    const ::ctrl::boolean callbackAllowsContinuation =
        static_cast<::ctrl::boolean>(
            callbacks.continueOperation(callbackContext));
    if (callbackAllowsContinuation)
    {
        return true;
    }
    static_cast<void>(picarxObject->emergencyStop());
    return false;
}

/**
 * @brief Performs a cancellable delay using bounded application-owned slices.
 * @param[in] durationMs Total requested delay in milliseconds.
 * @return `true` after the complete delay; `false` after cancellation and emergency stop.
 */
::ctrl::boolean XWalkController::delayWhileOperationRequested(::ctrl::uint32 durationMs)
{
    constexpr ::ctrl::uint32 cancellationIntervalMs{20U};
    ::ctrl::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        const ::ctrl::boolean operationRequested = operationMayContinue();
        if (operationRequested == false)
        {
            return false;
        }
        const ::ctrl::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        delay(sliceMs);
        remainingMs -= sliceMs;
    }
    return operationMayContinue();
}

} /* namespace xwalk::ctrl */
