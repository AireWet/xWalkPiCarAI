/******************************************************************************
 * @file        xWalk_handlerAppCli.h
 * @brief       Declares scheduler-backed CLI signal handling.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XWALK_HANDLER_APP_CLI_H
#define XWALK_HANDLER_APP_CLI_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSchedulerTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    /**
     * @brief Executes one validated CLI command through the Controller scheduler.
     * @param[in] signalNumber Stable command signal submitted to the Controller mailbox.
     * @param[in] moduleType Stable module classification copied into status events.
     * @param[in] signalCallback Child callback that owns boot and handler execution.
     * @param[in,out] context Callback context inherited by the child at `fork()`.
     * @return Handler result, or two when scheduler setup or supervision fails.
     */
    ::ctrl::int32 xWalk_handlerAppCli(::ctrl::uint32 signalNumber,
                                      ::ctrl::uint32 moduleType,
                                      xWalkSignalHandler_LPP signalCallback,
                                      ::ctrl::contextpointer context) noexcept;

} /* namespace xwalk::ctrl */

#endif /* XWALK_HANDLER_APP_CLI_H */
