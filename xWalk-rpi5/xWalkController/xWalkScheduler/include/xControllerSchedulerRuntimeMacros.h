/******************************************************************************
 * @file        xControllerSchedulerRuntimeMacros.h
 * @brief       Defines Controller scheduler lifecycle convenience wrappers.
 *
 * @project     xWalk Firmware
 * @module      xWalk Controller Scheduler
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULER_RUNTIME_MACROS_H
#define XCONTROLLER_SCHEDULER_RUNTIME_MACROS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSchedulerSignal.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

#define CXX_openCtrl_LPP(MODULE_TYPE, CALLBACKS, CONTINUE_CALLBACK, CONTEXT)                                           \
    (::xwalk::ctrl::cxx_xWalkOpen_LPP(                                                                                 \
        XWALK_CTRL_MAILBOX_ID, "xwalk-controller", (MODULE_TYPE), (CALLBACKS), (CONTINUE_CALLBACK), (CONTEXT)))
#define CXX_waitCtrl_LPP(SIG_NO, RESULT) (::xwalk::ctrl::cxx_xWalkWait_LPP(XWALK_CTRL_MAILBOX_ID, (SIG_NO), (RESULT)))
#define CXX_closeCtrl_LPP() (::xwalk::ctrl::cxx_xWalkClose_LPP())
#define CXX_getSchedulerStatus_LPP(STATUS_MASKS) (::xwalk::ctrl::cxx_xWalkGetSchedulerStatus_LPP((STATUS_MASKS)))

#endif /* XCONTROLLER_SCHEDULER_RUNTIME_MACROS_H */
