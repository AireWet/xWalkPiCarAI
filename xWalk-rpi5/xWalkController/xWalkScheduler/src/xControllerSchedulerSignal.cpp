/******************************************************************************
 * @file        xControllerSchedulerSignal.cpp
 * @brief       Implements native CBB signal helpers.
 *
 * @details
 * Validates and copies fixed-capacity native signals without transport code.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler
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

#include "xControllerSchedulerSignal.h"

#include "xControllerScheduler.h"
#include "xHal_Rpi5CarTrace.h"

#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

namespace xwalk::ctrl
{

    static XWalkScheduler* boundScheduler{nullptr};
    static ::ctrl::boolean childSignalActive{false};
    static ::ctrl::boolean childResponseReady{false};
    static XWalkSignal childRequestSignal{};
    static XWalkSignal childResponseSignal{};
    static ::ctrl::int32 childSocket{-1};

    ::ctrl::uint32 cxx_xWalkGetConfirmationSignal_LPP(::ctrl::uint32 requestSigNo) noexcept
    {
        if (requestSigNo == CXX_XWALK_I2C_REQ)
        {
            return CXX_XWALK_I2C_CFM;
        }
        if (((requestSigNo >= CXX_XWALK_CMD_UNKNOWN_REQ) && (requestSigNo <= CXX_XWALK_CMD_VIDEO_STREAM_REQ)) ||
            ((requestSigNo >= CXX_XWALK_CTRL_CMD_REQ) && (requestSigNo <= CXX_XWALK_SOUND_REQ)))
        {
            return requestSigNo + UINT32_C(0x0100);
        }
        return CXX_XWALK_SIGNAL_UNSPECIFIED;
    }

    ::ctrl::uint32 cxx_xWalkGetRejectionSignal_LPP(::ctrl::uint32 requestSigNo) noexcept
    {
        if (requestSigNo == CXX_XWALK_I2C_REQ)
        {
            return CXX_XWALK_I2C_REJ;
        }
        if (((requestSigNo >= CXX_XWALK_CMD_UNKNOWN_REQ) && (requestSigNo <= CXX_XWALK_CMD_VIDEO_STREAM_REQ)) ||
            ((requestSigNo >= CXX_XWALK_CTRL_CMD_REQ) && (requestSigNo <= CXX_XWALK_SOUND_REQ)))
        {
            return requestSigNo + UINT32_C(0x0200);
        }
        return CXX_XWALK_SIGNAL_UNSPECIFIED;
    }

    ::ctrl::int32 cxx_xWalkIsRequestSignal_LPP(::ctrl::uint32 sigNo) noexcept
    {
        return ((cxx_xWalkGetConfirmationSignal_LPP(sigNo) != CXX_XWALK_SIGNAL_UNSPECIFIED) ||
                (sigNo == CXX_XWALK_APP_CFG_REQ) || (sigNo == CXX_XWALK_APP_ARGS_REQ) ||
                (sigNo == CXX_XWALK_SERVO_CAL_CFG_REQ))
                   ? 1
                   : 0;
    }

    ::ctrl::int32 cxx_xWalkIsConfirmationSignal_LPP(::ctrl::uint32 sigNo) noexcept
    {
        return ((sigNo == CXX_XWALK_I2C_CFM) ||
                ((sigNo >= CXX_XWALK_CMD_UNKNOWN_CFM) && (sigNo <= CXX_XWALK_CMD_VIDEO_STREAM_CFM)) ||
                ((sigNo >= CXX_XWALK_CTRL_CMD_CFM) && (sigNo <= CXX_XWALK_SOUND_CFM)))
                   ? 1
                   : 0;
    }

    ::ctrl::int32 cxx_xWalkIsRejectionSignal_LPP(::ctrl::uint32 sigNo) noexcept
    {
        return ((sigNo == CXX_XWALK_I2C_REJ) ||
                ((sigNo >= CXX_XWALK_CMD_UNKNOWN_REJ) && (sigNo <= CXX_XWALK_CMD_VIDEO_STREAM_REJ)) ||
                ((sigNo >= CXX_XWALK_CTRL_CMD_REJ) && (sigNo <= CXX_XWALK_SOUND_REJ)))
                   ? 1
                   : 0;
    }

    ::ctrl::int32 cxx_xWalkSignalMatchesMailbox_LPP(::ctrl::uint32 mailBoxId, ::ctrl::uint32 sigNo) noexcept
    {
        if (mailBoxId == XWALK_HAL_MAILBOX_ID)
        {
            return ((sigNo == CXX_XWALK_I2C_REQ) || (sigNo == CXX_XWALK_I2C_CFM) || (sigNo == CXX_XWALK_I2C_REJ)) ? 1
                                                                                                                  : 0;
        }
        if (mailBoxId == XWALK_CTRL_MAILBOX_ID)
        {
            return (((sigNo >= CXX_XWALK_CMD_UNKNOWN_REQ) && (sigNo <= CXX_XWALK_CMD_VIDEO_STREAM_REQ)) ||
                    ((sigNo >= CXX_XWALK_CMD_UNKNOWN_CFM) && (sigNo <= CXX_XWALK_CMD_VIDEO_STREAM_CFM)) ||
                    ((sigNo >= CXX_XWALK_CMD_UNKNOWN_REJ) && (sigNo <= CXX_XWALK_CMD_VIDEO_STREAM_REJ)) ||
                    (sigNo == CXX_XWALK_APP_CFG_REQ) || (sigNo == CXX_XWALK_APP_ARGS_REQ) ||
                    (sigNo == CXX_XWALK_CTRL_CMD_REQ) || (sigNo == CXX_XWALK_CTRL_CMD_CFM) ||
                    (sigNo == CXX_XWALK_CTRL_CMD_REJ))
                       ? 1
                       : 0;
        }
        if (mailBoxId == XWALK_AGENT_MAILBOX_ID)
        {
            return (((sigNo >= CXX_XWALK_NO_ARG_REQ) && (sigNo <= CXX_XWALK_SERVO_CAL_CFG_REQ)) ||
                    ((sigNo >= CXX_XWALK_NO_ARG_CFM) && (sigNo <= CXX_XWALK_SOUND_CFM)) ||
                    ((sigNo >= CXX_XWALK_NO_ARG_REJ) && (sigNo <= CXX_XWALK_SOUND_REJ)))
                       ? 1
                       : 0;
        }
        return 0;
    }

    ::ctrl::int32 cxx_xWalkBindScheduler_LPP(XWalkScheduler* scheduler) noexcept
    {
        if ((scheduler == nullptr) || ((boundScheduler != nullptr) && (boundScheduler != scheduler)))
        {
            return scheduler == nullptr ? XWALK_SCHEDULER_INVALID_ARGUMENT : XWALK_SCHEDULER_INVALID_STATE;
        }
        boundScheduler = scheduler;
        return XWALK_SCHEDULER_OK;
    }

    void cxx_xWalkUnbindScheduler_LPP(const XWalkScheduler* scheduler) noexcept
    {
        if (boundScheduler == scheduler)
        {
            boundScheduler = nullptr;
        }
    }

    ::ctrl::int32 cxx_xWalkGetSchedulerStatus_LPP(xProcessStatusMasks* statusMasks) noexcept
    {
        return boundScheduler == nullptr ? XWALK_SCHEDULER_INVALID_STATE
                                         : boundScheduler->getProcessStatusMasks(statusMasks);
    }

    void xWalkBeginChildSignal(const XWalkSignal* requestSignal) noexcept
    {
        childSignalActive = requestSignal != nullptr;
        childResponseReady = false;
        std::memset(&childRequestSignal, 0, sizeof(childRequestSignal));
        std::memset(&childResponseSignal, 0, sizeof(childResponseSignal));
        if (requestSignal != nullptr)
        {
            childRequestSignal = *requestSignal;
        }
    }

    ::ctrl::boolean xWalkTakeChildSignal(XWalkSignal* responseSignal) noexcept
    {
        if ((responseSignal == nullptr) || !childResponseReady)
        {
            return false;
        }
        *responseSignal = childResponseSignal;
        return true;
    }

    void xWalkEndChildSignal() noexcept
    {
        childSignalActive = false;
        childResponseReady = false;
        std::memset(&childRequestSignal, 0, sizeof(childRequestSignal));
        std::memset(&childResponseSignal, 0, sizeof(childResponseSignal));
    }

    void xWalkSetChildSocket(::ctrl::int32 socketDescriptor) noexcept
    {
        childSocket = socketDescriptor;
    }

    ::ctrl::int32 cxx_xWalkSend_LPP(::ctrl::uint32 mailBoxId,
                                    const xWalkEncodedPayload* payload,
                                    ::ctrl::uint32 sigNo,
                                    xClientAddress* assignedRequestAddress) noexcept
    {
        if ((boundScheduler == nullptr) && childSignalActive && (childSocket >= 0) &&
            (mailBoxId != childRequestSignal.clientInfo.mailBoxId))
        {
            const ::ctrl::boolean payloadInvalid =
                (payload == nullptr) || ((payload->data == nullptr) && (payload->size != 0U)) ||
                (payload->size > XWALK_CBB_PAYLOAD_SIZE) || (cxx_xWalkIsRequestSignal_LPP(sigNo) == 0) ||
                (cxx_xWalkSignalMatchesMailbox_LPP(mailBoxId, sigNo) == 0);
            if (payloadInvalid)
            {
                return XWALK_SCHEDULER_INVALID_ARGUMENT;
            }
            xSchedulerPacket packet{};
            packet.protocolVersion = XWALK_SCHEDULER_PROTOCOL_VERSION;
            packet.messageSize = sizeof(packet);
            packet.packetType = XWALK_SCHEDULER_PACKET_NESTED_SIGNAL;
            packet.commandType = XWALK_COMMAND_CUSTOM;
            packet.address = childRequestSignal.clientInfo;
            packet.commandId = sigNo;
            packet.dataType = XWALK_SCHEDULER_DATA_SIGNAL;
            XWalkSignal nested{};
            nested.sigNo = sigNo;
            nested.source = childRequestSignal.destination;
            nested.destination = mailBoxId == XWALK_AGENT_MAILBOX_ID ? XWALK_MODULE_AGENT : XWALK_MODULE_HAL;
            nested.clientInfo.mailBoxId = mailBoxId;
            constexpr char nestedAddress[]{"xwalk-nested"};
            static_cast<void>(std::memcpy(nested.clientInfo.clientAddress, nestedAddress, sizeof(nestedAddress)));
            nested.clientInfo.moduleType = childRequestSignal.clientInfo.moduleType;
            nested.payloadSize = payload->size;
            if (payload->size != 0U)
            {
                static_cast<void>(std::memcpy(nested.payload, payload->data, payload->size));
            }
            packet.dataSize = sizeof(nested);
            static_cast<void>(std::memcpy(packet.commandData, &nested, sizeof(nested)));
            ssize_t sent{-1};
            do
            {
                sent = ::send(childSocket, &packet, sizeof(packet), MSG_NOSIGNAL);
            } while ((sent < 0) && (errno == EINTR));
            if ((sent < 0) && (errno == EPERM))
            {
                do
                {
                    sent = ::write(childSocket, &packet, sizeof(packet));
                } while ((sent < 0) && (errno == EINTR));
            }
            return sent == static_cast<ssize_t>(sizeof(packet)) ? XWALK_SCHEDULER_OK : XWALK_SCHEDULER_IPC_FAILURE;
        }
        if (boundScheduler == nullptr)
        {
            return XWALK_SCHEDULER_INVALID_STATE;
        }
        const ::ctrl::boolean payloadInvalid =
            (payload == nullptr) || ((payload->data == nullptr) && (payload->size != 0U)) ||
            (payload->size > XWALK_CBB_PAYLOAD_SIZE) || (cxx_xWalkIsRequestSignal_LPP(sigNo) == 0) ||
            (cxx_xWalkSignalMatchesMailbox_LPP(mailBoxId, sigNo) == 0);
        if (payloadInvalid)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        XWALK_CTRL_TRACE_UID3(
            CTRL .104, "Native signal mailbox %u signal %u bytes %zu", mailBoxId, sigNo, payload->size);
        return boundScheduler->submitSignal(
            mailBoxId, sigNo, payload->data, payload->size, nullptr, assignedRequestAddress);
    }

    ::ctrl::int32 cxx_xWalkSendSignal_LPP(::ctrl::uint32 expectedMailBoxId,
                                          const XWalkSignal* signal,
                                          xClientAddress* assignedRequestAddress) noexcept
    {
        if ((boundScheduler == nullptr) && childSignalActive)
        {
            const ::ctrl::uint32 cfm = cxx_xWalkGetConfirmationSignal_LPP(childRequestSignal.sigNo);
            const ::ctrl::uint32 rej = cxx_xWalkGetRejectionSignal_LPP(childRequestSignal.sigNo);
            if ((signal == nullptr) || childResponseReady ||
                (signal->clientInfo.mailBoxId != childRequestSignal.clientInfo.mailBoxId) ||
                (signal->clientInfo.xWalkLocalIndex != childRequestSignal.clientInfo.xWalkLocalIndex) ||
                ((signal->sigNo != cfm) && (signal->sigNo != rej)) || (signal->payloadSize > XWALK_CBB_PAYLOAD_SIZE))
            {
                return XWALK_SCHEDULER_INVALID_ARGUMENT;
            }
            childResponseSignal = *signal;
            childResponseReady = true;
            if (assignedRequestAddress != nullptr)
            {
                *assignedRequestAddress = signal->clientInfo;
            }
            return XWALK_SCHEDULER_OK;
        }
        const ::ctrl::boolean signalInvalid =
            (boundScheduler == nullptr) || (signal == nullptr) || (signal->clientInfo.mailBoxId != expectedMailBoxId) ||
            (signal->clientInfo.xWalkLocalIndex != 0U) || (signal->clientInfo.clientAddress[0] == '\0') ||
            (cxx_xWalkIsRequestSignal_LPP(signal->sigNo) == 0) ||
            (cxx_xWalkSignalMatchesMailbox_LPP(expectedMailBoxId, signal->sigNo) == 0) ||
            (signal->payloadSize > XWALK_CBB_PAYLOAD_SIZE);
        if (signalInvalid)
        {
            return boundScheduler == nullptr ? XWALK_SCHEDULER_INVALID_STATE : XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        return boundScheduler->submitSignal(expectedMailBoxId,
                                            signal->sigNo,
                                            signal->payload,
                                            signal->payloadSize,
                                            &signal->clientInfo,
                                            assignedRequestAddress);
    }

    ::ctrl::uint32 cxx_xWalkGetSignalNumber_LPP(::ctrl::uint32 expectedMailBoxId, const XWalkSignal* signal) noexcept
    {
        return ((signal == nullptr) || (signal->clientInfo.mailBoxId != expectedMailBoxId) ||
                (cxx_xWalkSignalMatchesMailbox_LPP(expectedMailBoxId, signal->sigNo) == 0))
                   ? static_cast<::ctrl::uint32>(CXX_XWALK_SIGNAL_UNSPECIFIED)
                   : signal->sigNo;
    }

    ::ctrl::int32
    cxx_xWalkSetSignalNumber_LPP(::ctrl::uint32 expectedMailBoxId, XWalkSignal* signal, ::ctrl::uint32 sigNo) noexcept
    {
        const ::ctrl::boolean signalMatchesMailbox = cxx_xWalkSignalMatchesMailbox_LPP(expectedMailBoxId, sigNo) != 0;
        const ::ctrl::boolean signalInvalid =
            (signal == nullptr) || !signalMatchesMailbox ||
            ((signal->clientInfo.mailBoxId != 0U) && (signal->clientInfo.mailBoxId != expectedMailBoxId));
        if (signalInvalid)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        signal->sigNo = sigNo;
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 cxx_xWalkGetClientInfo_LPP(::ctrl::uint32 expectedMailBoxId,
                                             const XWalkSignal* signal,
                                             xClientAddress* clientInfo) noexcept
    {
        if ((signal == nullptr) || (clientInfo == nullptr) || (signal->clientInfo.mailBoxId != expectedMailBoxId) ||
            (signal->clientInfo.xWalkLocalIndex == 0U) || (signal->clientInfo.clientAddress[0] == '\0'))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        *clientInfo = signal->clientInfo;
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 cxx_xWalkSetClientInfo_LPP(::ctrl::uint32 expectedMailBoxId,
                                             XWalkSignal* signal,
                                             const xClientAddress* clientInfo) noexcept
    {
        const ::ctrl::boolean clientInfoInvalid =
            (signal == nullptr) || (clientInfo == nullptr) || (clientInfo->mailBoxId != expectedMailBoxId) ||
            (clientInfo->xWalkLocalIndex == 0U) || (clientInfo->clientAddress[0] == '\0') ||
            (::strnlen(clientInfo->clientAddress, XWALK_CLIENT_ADDRESS_SIZE) >= XWALK_CLIENT_ADDRESS_SIZE);
        if (clientInfoInvalid)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        signal->clientInfo = *clientInfo;
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 cxx_xWalkSetPayload_LPP(::ctrl::uint32 expectedMailBoxId,
                                          XWalkSignal* signal,
                                          const xWalkEncodedPayload* payload) noexcept
    {
        if ((signal == nullptr) || (payload == nullptr) || ((payload->data == nullptr) && (payload->size != 0U)) ||
            (payload->size > XWALK_CBB_PAYLOAD_SIZE) || (signal->clientInfo.mailBoxId != expectedMailBoxId))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        std::memset(signal->payload, 0, sizeof(signal->payload));
        if (payload->size != 0U)
        {
            static_cast<void>(std::memcpy(signal->payload, payload->data, payload->size));
        }
        signal->payloadSize = payload->size;
        return XWALK_SCHEDULER_OK;
    }

    ::ctrl::int32 cxx_xWalkGetPayload_LPP(::ctrl::uint32 expectedMailBoxId,
                                          const XWalkSignal* signal,
                                          xWalkPayloadBuffer* payload) noexcept
    {
        if ((signal == nullptr) || (payload == nullptr) || (signal->clientInfo.mailBoxId != expectedMailBoxId) ||
            (signal->payloadSize > XWALK_CBB_PAYLOAD_SIZE))
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        payload->size = signal->payloadSize;
        if ((signal->payloadSize > payload->capacity) || ((payload->data == nullptr) && (signal->payloadSize != 0U)))
        {
            return XWALK_SCHEDULER_BUFFER_TOO_SMALL;
        }
        if (signal->payloadSize != 0U)
        {
            static_cast<void>(std::memcpy(payload->data, signal->payload, signal->payloadSize));
        }
        return XWALK_SCHEDULER_OK;
    }

} /* namespace xwalk::ctrl */
