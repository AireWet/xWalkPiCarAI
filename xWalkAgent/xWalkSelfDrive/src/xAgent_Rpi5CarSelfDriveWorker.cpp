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
constexpr xwalk::hal::uint32 ACTION_COMPLETION_DELAY_MS = 500U;
/** @brief Delay between worker state checks, in milliseconds. */
constexpr xwalk::hal::uint32 WORKER_POLL_DELAY_MS = 10U;

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
    while (runningValue.load())
    {
        hal::string action;
        hal::boolean runThinkingPose = false;

        {
            hal::mutexlock lock(stateMutex);
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
                if (actionQueue.empty())
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
        if (!action.empty())
        {
            static_cast<void>(doAction(action));
            if (operationFailedValue.load())
            {
                hal::mutexlock lock(stateMutex);
                statusValue = XWalkSelfDriveStatus::Standby;
                stateChanged.notify_all();
                return;
            }
            static_cast<void>(delay(ACTION_COMPLETION_DELAY_MS));
            hal::mutexlock lock(stateMutex);
            if (actionQueue.empty())
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
hal::boolean XWalkSelfDrive::addAction(hal::stringview action)
{
    if (!isActionSupported(action))
    {
        return false;
    }

    hal::mutexlock lock(stateMutex);
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
    hal::mutexlock lock(stateMutex);
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
hal::boolean XWalkSelfDrive::waitActionsDone()
{
    hal::uniquemutexlock lock(stateMutex);
    stateChanged.wait(lock, [this]()
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
    hal::mutexlock lock(stateMutex);
    return statusValue;
}

} /* namespace xwalk::agent */
