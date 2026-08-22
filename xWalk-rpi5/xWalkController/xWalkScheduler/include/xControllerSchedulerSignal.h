/******************************************************************************
 * @file        xControllerSchedulerSignal.h
 * @brief       Declares common Protobuf signal and scheduler helpers.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULER_SIGNAL_H
#define XCONTROLLER_SCHEDULER_SIGNAL_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSchedulerTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    class XWalkScheduler;

    ::ctrl::int32 cxx_xWalkIsRequestSignal_LPP(::ctrl::uint32 sigNo) noexcept;
    ::ctrl::int32 cxx_xWalkIsConfirmationSignal_LPP(::ctrl::uint32 sigNo) noexcept;
    ::ctrl::int32 cxx_xWalkIsRejectionSignal_LPP(::ctrl::uint32 sigNo) noexcept;
    ::ctrl::uint32 cxx_xWalkGetConfirmationSignal_LPP(::ctrl::uint32 requestSigNo) noexcept;
    ::ctrl::uint32 cxx_xWalkGetRejectionSignal_LPP(::ctrl::uint32 requestSigNo) noexcept;
    ::ctrl::int32 cxx_xWalkSignalMatchesMailbox_LPP(::ctrl::uint32 mailBoxId, ::ctrl::uint32 sigNo) noexcept;

    ::ctrl::int32 cxx_xWalkBindScheduler_LPP(XWalkScheduler* scheduler) noexcept;
    void cxx_xWalkUnbindScheduler_LPP(const XWalkScheduler* scheduler) noexcept;
    ::ctrl::int32 cxx_xWalkGetSchedulerStatus_LPP(xProcessStatusMasks* statusMasks) noexcept;
    ::ctrl::int32 cxx_xWalkOpen_LPP(::ctrl::uint32 mailBoxId,
                                    ::ctrl::cstring clientAddress,
                                    ::ctrl::uint32 moduleType,
                                    const xModuleCallbacks* callbacks,
                                    xSchedulerContinueCallback continueCallback,
                                    ::ctrl::contextpointer continueContext) noexcept;
    ::ctrl::int32
    cxx_xWalkWait_LPP(::ctrl::uint32 mailBoxId, ::ctrl::uint32 signalNumber, ::ctrl::int32* requestResult) noexcept;
    void cxx_xWalkClose_LPP() noexcept;

    ::ctrl::int32 cxx_xWalkSchedulerRegister_LPP(xWalkModuleId moduleId,
                                                 ::ctrl::contextpointer context,
                                                 xWalkSignalHandler_LPP handler,
                                                 ::ctrl::uint32 moduleType) noexcept;
    ::ctrl::int32 cxx_xWalkSchedulerUnregister_LPP(xWalkModuleId moduleId) noexcept;
    ::ctrl::int32 cxx_xWalkSchedulerSend_LPP(const XWalkSignal* signal, xClientAddress* requestAddress) noexcept;

    ::ctrl::int32 cxx_xWalkCtrlInit_LPP(::ctrl::contextpointer context,
                                        xWalkSignalHandler_LPP handler,
                                        ::ctrl::uint32 moduleType) noexcept;
    ::ctrl::int32 cxx_xWalkAgentInit_LPP(::ctrl::contextpointer context,
                                         xWalkSignalHandler_LPP handler,
                                         ::ctrl::uint32 moduleType) noexcept;
    ::ctrl::int32 cxx_xWalkHalInit_LPP(::ctrl::contextpointer context,
                                       xWalkSignalHandler_LPP handler,
                                       ::ctrl::uint32 moduleType) noexcept;
    ::ctrl::int32 cxx_xWalkCtrlShutdown_LPP() noexcept;
    ::ctrl::int32 cxx_xWalkAgentShutdown_LPP() noexcept;
    ::ctrl::int32 cxx_xWalkHalShutdown_LPP() noexcept;
    ::ctrl::int32 cxx_xWalkCtrlHandler_LPP(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept;
    ::ctrl::int32 cxx_xWalkAgentHandler_LPP(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept;
    ::ctrl::int32 cxx_xWalkHalHandler_LPP(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept;

    /** @brief Starts one child-handler response context for the supplied request. */
    void xWalkBeginChildSignal(const XWalkSignal* requestSignal) noexcept;

    /** @brief Copies the response submitted by the active child handler. */
    ::ctrl::boolean xWalkTakeChildSignal(XWalkSignal* responseSignal) noexcept;

    /** @brief Clears the active child-handler response context. */
    void xWalkEndChildSignal() noexcept;
    void xWalkSetChildSocket(::ctrl::int32 socketDescriptor) noexcept;

    ::ctrl::int32 cxx_xWalkSend_LPP(::ctrl::uint32 mailBoxId,
                                    const xWalkEncodedPayload* payload,
                                    ::ctrl::uint32 sigNo,
                                    xClientAddress* assignedRequestAddress) noexcept;
    ::ctrl::int32 cxx_xWalkSendSignal_LPP(::ctrl::uint32 expectedMailBoxId,
                                          const XWalkSignal* signal,
                                          xClientAddress* assignedRequestAddress) noexcept;

    ::ctrl::uint32 cxx_xWalkGetSignalNumber_LPP(::ctrl::uint32 expectedMailBoxId, const XWalkSignal* signal) noexcept;
    ::ctrl::int32
    cxx_xWalkSetSignalNumber_LPP(::ctrl::uint32 expectedMailBoxId, XWalkSignal* signal, ::ctrl::uint32 sigNo) noexcept;
    ::ctrl::int32 cxx_xWalkGetClientInfo_LPP(::ctrl::uint32 expectedMailBoxId,
                                             const XWalkSignal* signal,
                                             xClientAddress* clientInfo) noexcept;
    ::ctrl::int32 cxx_xWalkSetClientInfo_LPP(::ctrl::uint32 expectedMailBoxId,
                                             XWalkSignal* signal,
                                             const xClientAddress* clientInfo) noexcept;
    ::ctrl::int32 cxx_xWalkSetPayload_LPP(::ctrl::uint32 expectedMailBoxId,
                                          XWalkSignal* signal,
                                          const xWalkEncodedPayload* payload) noexcept;
    ::ctrl::int32 cxx_xWalkGetPayload_LPP(::ctrl::uint32 expectedMailBoxId,
                                          const XWalkSignal* signal,
                                          xWalkPayloadBuffer* payload) noexcept;

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_SCHEDULER_SIGNAL_H */
