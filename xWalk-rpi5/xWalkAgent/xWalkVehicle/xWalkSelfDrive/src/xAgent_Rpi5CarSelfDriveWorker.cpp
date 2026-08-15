/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveWorker.cpp
 * @brief       Implements the self-drive background action-flow queue.
 *
 * @details
 * Implements explicit standby, thinking, and queued-action states while
 * reporting callback failures through explicit status.
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

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains worker timing values private to this translation unit.
 */
namespace
{

    /** @brief Delay after one queued action, in milliseconds. */
    constexpr xwalk::agent::uint32 ACTION_COMPLETION_DELAY_MS = 500U;
    /** @brief Delay between worker state checks, in milliseconds. */
    constexpr xwalk::agent::uint32 WORKER_POLL_DELAY_MS = 10U;

} /* namespace */

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
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Runs the background status and action-queue loop.
     *
     * @details
     * Delay failures use explicit callback status and latch emergency shutdown.
     */
    void XWalkSelfDrive::actionLoop() noexcept
    {
        const agent::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const agent::boolean workerRunning = static_cast<agent::boolean>(runningValue.load());
            if (workerRunning == false)
            {
                break;
            }
            agent::string action;
            agent::boolean runThinkingPose = false;

            {
                agent::mutexlock lock(stateMutex);
                if (statusValue == XWalkSelfDriveStatus::Standby)
                {
                    lastStatusValue = XWalkSelfDriveStatus::Standby;
                    hasLastStatus = true;
                }
                else if (statusValue == XWalkSelfDriveStatus::Think)
                {
                    if ((!hasLastStatus) || (lastStatusValue != XWalkSelfDriveStatus::Think))
                    {
                        lastStatusValue = XWalkSelfDriveStatus::Think;
                        hasLastStatus = true;
                        runThinkingPose = true;
                    }
                }
                else if (statusValue == XWalkSelfDriveStatus::Actions)
                {
                    lastStatusValue = XWalkSelfDriveStatus::Actions;
                    hasLastStatus = true;
                    const agent::boolean actionQueueEmpty = static_cast<agent::boolean>(actionQueue.empty());
                    if (actionQueueEmpty)
                    {
                        statusValue = XWalkSelfDriveStatus::Standby;
                        stateChanged.notify_all();
                    }
                    else
                    {
                        action = actionQueue.front();
                        actionQueue.erase(actionQueue.begin());
                    }
                }
            }

            if (runThinkingPose)
            {
                keepThink();
            }
            const agent::boolean actionAvailable = static_cast<agent::boolean>(!action.empty());
            if (actionAvailable)
            {
                static_cast<void>(doAction(action));
                const agent::boolean actionFailed = static_cast<agent::boolean>(operationFailedValue.load());
                if (actionFailed)
                {
                    agent::mutexlock lock(stateMutex);
                    statusValue = XWalkSelfDriveStatus::Standby;
                    stateChanged.notify_all();
                    return;
                }
                static_cast<void>(delay(ACTION_COMPLETION_DELAY_MS));
                agent::mutexlock lock(stateMutex);
                const agent::boolean actionQueueEmpty = static_cast<agent::boolean>(actionQueue.empty());
                if (actionQueueEmpty)
                {
                    statusValue = XWalkSelfDriveStatus::Standby;
                    stateChanged.notify_all();
                }
            }
            static_cast<void>(delay(WORKER_POLL_DELAY_MS));
        }
        stateChanged.notify_all();
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Adds one supported action to the background first-in, first-out queue.
     *
     * @param[in] action
     * Exact lowercase action name to copy into the queue.
     *
     * @return
     * `true` when the action was queued; otherwise `false` for an unknown name.
     */
    agent::boolean XWalkSelfDrive::addAction(agent::stringview action)
    {
        const agent::boolean actionSupportedNotMatched = static_cast<agent::boolean>(!isActionSupported(action));
        if (actionSupportedNotMatched)
        {
            return false;
        }

        agent::mutexlock lock(stateMutex);
        actionQueue.emplace_back(action);
        statusValue = XWalkSelfDriveStatus::Actions;
        stateChanged.notify_all();
        return true;
    }

    /**
     * @brief Selects the worker status used on its next iteration.
     *
     * @param[in] status
     * New worker state.
     */
    void XWalkSelfDrive::setStatus(XWalkSelfDriveStatus status)
    {
        agent::mutexlock lock(stateMutex);
        statusValue = status;
        stateChanged.notify_all();
    }

    /**
     * @brief Waits until queued actions reach standby or the worker stops.
     *
     * @post
     * The status is standby or `running()` returns `false`.
     *
     * @return
     * `true` when queued work completed; otherwise `false` after emergency shutdown.
     */
    agent::boolean XWalkSelfDrive::waitActionsDone()
    {
        agent::uniquemutexlock lock(stateMutex);
        stateChanged.wait(lock,
                          [this]()
                          {
                              return (statusValue == XWalkSelfDriveStatus::Standby) || (!runningValue.load());
                          });
        return !operationFailedValue.load();
    }

    /**
     * @brief Returns the current worker status under synchronization.
     *
     * @return
     * Current action-flow state.
     */
    XWalkSelfDriveStatus XWalkSelfDrive::status() const
    {
        agent::mutexlock lock(stateMutex);
        return statusValue;
    }

} /* namespace xwalk::agent */
