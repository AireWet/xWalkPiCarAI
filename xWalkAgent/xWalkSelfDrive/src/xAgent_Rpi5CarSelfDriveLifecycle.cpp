/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveLifecycle.cpp
 * @brief       Implements self-drive dependency and worker lifecycle behavior.
 *
 * @details
 * Validates the injected timing boundary and owns the optional action worker.
 *
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive
 *
 * @author      Joxy John
 * @date        2026-07-31
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
#include "xAgent_Rpi5CarSelfDrive.h"
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
 * @brief Constructs a preset-action coordinator around caller-owned objects.
 *
 * @param[in] picarx
 * PiCar-X coordinator that must outlive this object and any worker.
 *
 * @param[in] music
 * Music controller that must outlive this object and any worker.
 *
 * @param[in,out] context
 * Optional callback context that must outlive this object and any worker.
 *
 * @param[in] callback
 * Non-null delay operation. Worker use must be non-throwing.
 *
 * @param[in] continueOperation
 * Optional cancellation query. Worker use must be non-throwing when supplied.
 *
 * @param[in] soundDirectory
 * Sound-resource directory copied for later preset resolution.
 *
 * @throws std::invalid_argument
 * If `callback` is null.
 */
XWalkSelfDrive::XWalkSelfDrive(XWalkPicarx& picarx, hal::XWalkMusic& music,
    hal::contextpointer context, selfdrivedelaycallback callback,
    selfdrivecontinuecallback continueOperation, hal::stringview soundDirectory)
    : picarxObject(&picarx), musicObject(&music), soundDirectoryValue(soundDirectory),
      callbackContext(context),
      delayCallback(callback), continueCallback(continueOperation)
{
    validateDelayCallback(delayCallback);
    if (soundDirectoryValue.empty())
    {
        XHAL_THROW_INVALID_ARGUMENT("Self-drive sound directory must not be empty");
    }
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Stops and joins the worker without releasing injected objects.
 *
 * @warning
 * Operations running on the worker and its callbacks must not throw.
 */
XWalkSelfDrive::~XWalkSelfDrive()
{
    stop();
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Rejects a null delay callback before storing dependencies.
 *
 * @param[in] delayOperation
 * Delay callback that must not be null.
 *
 * @throws std::invalid_argument
 * If `delayOperation` is null.
 */
void XWalkSelfDrive::validateDelayCallback(selfdrivedelaycallback delayOperation)
{
    if (delayOperation == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Self-drive delay callback must not be null");
    }
}

/**
 * @brief Invokes the application-owned delay operation.
 *
 * @param[in] durationMs
 * Requested delay in milliseconds.
 *
 * @return
 * `true` when the complete delay finishes; otherwise `false` after cancellation
 * or callback failure has latched emergency stop.
 */
hal::boolean XWalkSelfDrive::delay(hal::uint32 durationMs)
{
    if (continueCallback == nullptr)
    {
        const hal::boolean completed = delayCallback(callbackContext, durationMs);
        if (!completed)
        {
            operationFailedValue.store(true);
            runningValue.store(false);
            static_cast<void>(picarxObject->emergencyStop());
            stateChanged.notify_all();
        }
        return completed;
    }
    constexpr hal::uint32 cancellationIntervalMs{20U};
    hal::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        if (!continueCallback(callbackContext))
        {
            static_cast<void>(picarxObject->emergencyStop());
            return false;
        }
        const hal::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        if (!delayCallback(callbackContext, sliceMs))
        {
            operationFailedValue.store(true);
            runningValue.store(false);
            static_cast<void>(picarxObject->emergencyStop());
            stateChanged.notify_all();
            return false;
        }
        remainingMs -= sliceMs;
    }
    return true;
}

/**
 * @brief Replaces the optional application cancellation query.
 * @param[in,out] context Optional non-owning context that must outlive later action execution.
 * @param[in] continueOperation Optional non-throwing query; null disables cancellation checks.
 */
void XWalkSelfDrive::setCancellation(hal::contextpointer context,
    selfdrivecontinuecallback continueOperation) noexcept
{
    callbackContext = context;
    continueCallback = continueOperation;
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Starts a fresh background worker and clears previously queued actions.
 *
 * @throws std::logic_error
 * If the worker is already running.
 */
void XWalkSelfDrive::start()
{
    hal::mutexlock lock(stateMutex);
    if (runningValue.load())
    {
        XHAL_THROW_LOGIC_ERROR("Self-drive worker is already running");
    }
    actionQueue.clear();
    statusValue = XWalkSelfDriveStatus::Standby;
    lastStatusValue = XWalkSelfDriveStatus::Standby;
    hasLastStatus = false;
    operationFailedValue.store(false);
    runningValue.store(true);
    worker = hal::threadhandle(&XWalkSelfDrive::actionLoop, this);
}

/**
 * @brief Requests worker shutdown and joins it when present.
 *
 * @post
 * `running()` returns `false` and no worker remains joinable.
 */
void XWalkSelfDrive::stop()
{
    runningValue.store(false);
    stateChanged.notify_all();
    if (worker.joinable())
    {
        worker.join();
    }
}

/**
 * @brief Returns whether the background worker has been requested to run.
 *
 * @return
 * `true` between successful `start()` and completion of `stop()`.
 */
hal::boolean XWalkSelfDrive::running() const noexcept
{
    return runningValue.load();
}

} /* namespace xwalk::agent */
