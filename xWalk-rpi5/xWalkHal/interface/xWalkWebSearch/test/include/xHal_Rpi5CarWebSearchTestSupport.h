/******************************************************************************
 * @file        xHal_Rpi5CarWebSearchTestSupport.h
 * @brief       Declares reusable web-search host-test transport state.
 * @project     xWalk Firmware
 * @module      xWalkWebSearch Host Test
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_WEB_SEARCH_TEST_SUPPORT_H
#define XHAL_RPI5CAR_WEB_SEARCH_TEST_SUPPORT_H

#include "xHal_Rpi5CarWebSearch.h"

namespace xwalk::hal::test::websearch
{
    /** @brief Records one injected bounded retrieval request. */
    struct TestTransport
    {
            string response{};   /**< JSON returned by the fake transport. */
            string url{};        /**< Last complete encoded request URL. */
            uint32 timeoutMs{};  /**< Last timeout in milliseconds. */
            size maximumBytes{}; /**< Last response byte bound. */
    };

    string getJson(contextpointer context, stringview url, uint32 timeoutMs, size maximumBytes);
    XWalkWebSearchOperations operations() noexcept;
} /* namespace xwalk::hal::test::websearch */

#endif /* XHAL_RPI5CAR_WEB_SEARCH_TEST_SUPPORT_H */
