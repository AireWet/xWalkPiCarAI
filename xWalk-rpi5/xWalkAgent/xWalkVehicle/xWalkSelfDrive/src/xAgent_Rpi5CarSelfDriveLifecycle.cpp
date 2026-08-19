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

#include "xHal_Rpi5CarTrace.h"
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
    XWalkSelfDrive::XWalkSelfDrive(XWalkPicarx& picarx,
                                   hal::XWalkMusic& music,
                                   agent::contextpointer context,
                                   selfdrivedelaycallback callback,
                                   selfdrivecontinuecallback continueOperation,
                                   agent::stringview soundDirectory)
        : picarxObject(&picarx), musicObject(&music), soundDirectoryValue(soundDirectory), callbackContext(context),
          delayCallback(callback), continueCallback(continueOperation)
    {
        validateDelayCallback(delayCallback);
        const agent::boolean soundDirectoryEmpty = static_cast<agent::boolean>(soundDirectoryValue.empty());
        if (soundDirectoryEmpty)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Self-drive sound directory must not be empty");
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
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Self-drive delay callback must not be null");
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
    agent::boolean XWalkSelfDrive::delay(agent::uint32 durationMs)
    {
        if (continueCallback == nullptr)
        {
            const agent::boolean completed = delayCallback(callbackContext, durationMs);
            if (!completed)
            {
                operationFailedValue.store(true);
                runningValue.store(false);
                static_cast<void>(picarxObject->emergencyStop());
                stateChanged.notify_all();
            }
            return completed;
        }
        constexpr agent::uint32 cancellationIntervalMs{20U};
        agent::uint32 remainingMs = durationMs;
        while (remainingMs > 0U)
        {
            const agent::boolean operationRequested = continueCallback(callbackContext);
            if (operationRequested == false)
            {
                static_cast<void>(picarxObject->emergencyStop());
                return false;
            }
            const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ? remainingMs : cancellationIntervalMs;
            const agent::boolean delayCompleted = delayCallback(callbackContext, sliceMs);
            if (delayCompleted == false)
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
     * @param[in,out] context Optional non-owning context that must outlive later
     * action execution.
     * @param[in] continueOperation Optional non-throwing query; null disables
     * cancellation checks.
     */
    void XWalkSelfDrive::setCancellation(agent::contextpointer context,
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
        const agent::boolean workerAlreadyRunning = static_cast<agent::boolean>(runningValue.load());
        if (workerAlreadyRunning)
        {
            XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Self-drive worker is already running");
        }
        const agent::boolean completedWorkerJoinable = static_cast<agent::boolean>(worker.joinable());
        if (completedWorkerJoinable)
        {
            worker.join();
        }
        agent::mutexlock lock(stateMutex);
        actionQueue.clear();
        statusValue = XWalkSelfDriveStatus::Standby;
        lastStatusValue = XWalkSelfDriveStatus::Standby;
        hasLastStatus = false;
        operationFailedValue.store(false);
        runningValue.store(true);
        worker = agent::threadhandle(&XWalkSelfDrive::actionLoop, this);
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
        const agent::boolean workerJoinable = static_cast<agent::boolean>(worker.joinable());
        if (workerJoinable)
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
    agent::boolean XWalkSelfDrive::running() const noexcept
    {
        return runningValue.load();
    }

} /* namespace xwalk::agent */
