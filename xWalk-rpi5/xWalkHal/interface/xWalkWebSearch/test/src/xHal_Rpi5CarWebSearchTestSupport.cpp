/******************************************************************************
 * @file        xHal_Rpi5CarWebSearchTestSupport.cpp
 * @brief       Implements the injected web-search host-test transport.
 * @project     xWalk Firmware
 * @module      xWalkWebSearch Host Test
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarWebSearchTestSupport.h"
#include "xHal_Rpi5CarTrace.h"

namespace xwalk::hal::test::websearch
{
    string getJson(contextpointer context, stringview url, uint32 timeoutMs, size maximumBytes)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Web-search test transport requires state");
        }
        TestTransport& state = *static_cast<TestTransport*>(context);
        state.url = url;
        state.timeoutMs = timeoutMs;
        state.maximumBytes = maximumBytes;
        return state.response;
    }

    XWalkWebSearchOperations operations() noexcept
    {
        return {&getJson};
    }
} /* namespace xwalk::hal::test::websearch */
