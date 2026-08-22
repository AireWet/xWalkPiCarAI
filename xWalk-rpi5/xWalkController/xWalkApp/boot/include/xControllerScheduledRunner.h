/******************************************************************************
 * @file        xControllerScheduledRunner.h
 * @brief       Declares scheduler-only Controller command orchestration.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULED_RUNNER_H
#define XCONTROLLER_SCHEDULED_RUNNER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSchedulerTypes.h"

#include "xWalkControllerConfigTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    struct XWalkRunArgs;

    /**
     * @brief Parses and executes the device-free application through the scheduler.
     * @param[in] argumentCount Number of process arguments including the executable name.
     * @param[in] arguments Non-owning process argument array.
     * @return Application status after scheduler-owned command execution.
     */
    ::ctrl::int32 xWalkRunHostApplication(::ctrl::int32 argumentCount, ::ctrl::charpointer arguments[]);

    /**
     * @brief Builds and submits the correlated Controller response for one handler result.
     * @param[in] request Scheduler-assigned request received by the child handler.
     * @param[in] result Handler result used to select CFM or REJ.
     * @return The original handler result, or two when signal validation or response submission fails.
     */
    ::ctrl::int32 XWALK_reply(const XWalkSignal* request, ::ctrl::int32 result) noexcept;

    /** @brief Executes the host-stub boot and Controller handler in a scheduler child. */
    ::ctrl::int32 XWALK_hostCmd(XWalkRunArgs* runArgs) noexcept;

    /** @brief Executes the Raspberry Pi boot and Controller handler in a scheduler child. */
    ::ctrl::int32 XWALK_runRpi(XWalkRunArgs* runArgs) noexcept;

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_SCHEDULED_RUNNER_H */
