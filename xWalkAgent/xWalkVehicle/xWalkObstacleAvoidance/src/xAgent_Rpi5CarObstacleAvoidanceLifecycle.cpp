/******************************************************************************
 * @file        xAgent_Rpi5CarObstacleAvoidanceLifecycle.cpp
 * @brief       Implements obstacle-avoidance lifecycle and scheduling.
 *
 * @details
 * Validates callbacks, provides bounded cancellation polling, and performs
 * non-throwing emergency motor cleanup during destruction.
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

#include "xHal_Rpi5CarExceptions.h"

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
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds caller-owned vehicle and synchronous scheduling operations.
 * @param[in] picarx PiCar-X coordinator that must outlive this Agent.
 * @param[in,out] context Optional callback context that must outlive this Agent.
 * @param[in] delayOperation Non-null synchronous delay operation.
 * @param[in] continueOperation Non-null synchronous cancellation query.
 * @throws std::invalid_argument If either callback is null.
 */
XWalkObstacleAvoidance::XWalkObstacleAvoidance(XWalkPicarx& picarx,
    agent::contextpointer context, obstacleavoidancedelaycallback delayOperation,
    obstacleavoidancecontinuecallback continueOperation):
    picarxObject(&picarx), callbackContext(context), delayCallback(delayOperation),
    continueCallback(continueOperation)
{
    if ((delayCallback == nullptr) || (continueCallback == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Obstacle avoidance requires complete callbacks");
    }
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Latches a non-throwing emergency motor stop without releasing dependencies.
 * @post The PiCar-X emergency latch is set and both motors receive independent stop attempts.
 */
XWalkObstacleAvoidance::~XWalkObstacleAvoidance() noexcept
{
    static_cast<void>(picarxObject->emergencyStop());
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Waits in cancellable slices no longer than 20 milliseconds.
 * @param[in] durationMs Requested delay in milliseconds.
 * @return `true` after the complete delay or `false` after cancellation.
 */
agent::boolean XWalkObstacleAvoidance::wait(agent::uint32 durationMs) const
{
    constexpr agent::uint32 cancellationIntervalMs{20U};
    agent::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        const agent::boolean operationRequested = continueCallback(callbackContext);
        if (operationRequested == false)
        {
            return false;
        }
        const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        delayCallback(callbackContext, sliceMs);
        remainingMs -= sliceMs;
    }
    return continueCallback(callbackContext);
}

} /* namespace xwalk::agent */
