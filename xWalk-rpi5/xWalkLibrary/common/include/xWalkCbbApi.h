/******************************************************************************
 * @file        xWalkCbbApi.h
 * @brief       Declares native module-specific CBB scheduler APIs.
 *
 * @details
 * A send name identifies its destination: Ctrl sends to Controller, Agent sends
 * to Agent, and Hal sends to HAL. No API in this header depends on a server or
 * transport serialization library.
 *
 * @project     xWalk Firmware
 * @module      xWalk Common Library
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XWALK_CBB_API_H
#define XWALK_CBB_API_H

#include "xWalkCbbSignal.h"

namespace xwalk::ctrl
{

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
    ::ctrl::int32 cxx_xWalkGetPayload_LPP(::ctrl::uint32 expectedMailBoxId,
                                          const XWalkSignal* signal,
                                          xWalkPayloadBuffer* payload) noexcept;
    ::ctrl::int32 cxx_xWalkSetPayload_LPP(::ctrl::uint32 expectedMailBoxId,
                                          XWalkSignal* signal,
                                          const xWalkEncodedPayload* payload) noexcept;

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

} /* namespace xwalk::ctrl */

#endif /* XWALK_CBB_API_H */
