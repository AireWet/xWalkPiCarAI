/******************************************************************************
 * @file        xControllerSchedulerRuntimeTypes.h
 * @brief       Defines the private module-facing scheduler runtime record.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULER_RUNTIME_TYPES_H
#define XCONTROLLER_SCHEDULER_RUNTIME_TYPES_H

#include "xControllerScheduler.h"

namespace xwalk::ctrl
{

    /** @brief Stores one module callback behind its public CBB handler. */
    typedef struct xCbbModuleRecord
    {
            ::ctrl::boolean registered;
            xWalkModuleId moduleId;
            ::ctrl::contextpointer context;
            xWalkSignalHandler_LPP handler;
    } xCbbModuleRecord;

    /** @brief Owns one hidden scheduler façade instance for an application command. */
    typedef struct xSchedulerRuntime
    {
            XWalkScheduler scheduler;
            ::ctrl::uint32 mailBoxId;
            xSchedulerContinueCallback continueCallback;
            ::ctrl::contextpointer continueContext;
            xCbbModuleRecord modules[3U];
    } xSchedulerRuntime;

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_SCHEDULER_RUNTIME_TYPES_H */
