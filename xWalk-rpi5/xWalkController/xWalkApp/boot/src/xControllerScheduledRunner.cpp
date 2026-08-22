/******************************************************************************
 * @file        xControllerScheduledRunner.cpp
 * @brief       Implements scheduler-only Controller command orchestration.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerScheduledRunner.h"

#include "xControllerSchedulerMacros.h"
#include "xControllerSchedulerSignal.h"

#include <cstring>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

    /**************************************************************************
     * Public function definitions
     **************************************************************************/

    ::ctrl::int32 XWALK_reply(const XWalkSignal* request, ::ctrl::int32 result) noexcept
    {
        xClientAddress clientInfo{};
        ::ctrl::uint8 requestData[XWALK_SCHEDULER_COMMAND_DATA_SIZE]{};
        xWalkPayloadBuffer requestPayload{requestData, sizeof(requestData), 0U};
        const ::ctrl::uint32 requestSignal = CXX_getCtrlSignal_LPP(request);
        const ::ctrl::int32 requestClientInfoStatus = CXX_getCtrlClientInfo_LPP(request, &clientInfo);
        const ::ctrl::int32 requestPayloadStatus = CXX_getCtrlPayload_LPP(request, &requestPayload);
        if ((requestSignal == CXX_XWALK_SIGNAL_UNSPECIFIED) || (requestClientInfoStatus != XWALK_SCHEDULER_OK) ||
            (requestPayloadStatus != XWALK_SCHEDULER_OK))
        {
            return 2;
        }

        const ::ctrl::uint32 responseSignal = result == 0 ? cxx_xWalkGetConfirmationSignal_LPP(requestSignal)
                                                          : cxx_xWalkGetRejectionSignal_LPP(requestSignal);
        XWalkSignal response{};
        const ::ctrl::int32 clientInfoStatus = CXX_setCtrlClientInfo_LPP(&response, &clientInfo);
        const ::ctrl::int32 signalStatus = CXX_setCtrlSignal_LPP(&response, responseSignal);
        const ::ctrl::boolean responseInvalid = (responseSignal == CXX_XWALK_SIGNAL_UNSPECIFIED) ||
                                                (clientInfoStatus != XWALK_SCHEDULER_OK) ||
                                                (signalStatus != XWALK_SCHEDULER_OK);
        if (responseInvalid)
        {
            return 2;
        }

        xWalkEncodedPayload responsePayload{nullptr, 0U};
        xWalkRejectPayload rejection{};
        if (result != 0)
        {
            const std::int64_t signedResult = result;
            const ::ctrl::uint32 reason = signedResult < 0 ? static_cast<::ctrl::uint32>(-signedResult)
                                                           : static_cast<::ctrl::uint32>(signedResult);
            rejection.reason = reason;
            rejection.errorSignal = 6U;
            constexpr char detail[]{"Scheduled Controller handler returned failure"};
            static_cast<void>(std::memcpy(rejection.detail, detail, sizeof(detail)));
            responsePayload.data = reinterpret_cast<const ::ctrl::uint8*>(&rejection);
            responsePayload.size = sizeof(rejection);
        }
        const ::ctrl::int32 payloadStatus = CXX_setCtrlPayload_LPP(&response, &responsePayload);
        const ::ctrl::int32 responseStatus = CXX_sendCtrlSignal_LPP(&response, nullptr);
        const ::ctrl::boolean responseFailed =
            (payloadStatus != XWALK_SCHEDULER_OK) || (responseStatus != XWALK_SCHEDULER_OK);
        if (responseFailed)
        {
            return 2;
        }
        return result;
    }

} /* namespace xwalk::ctrl */
