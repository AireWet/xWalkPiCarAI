/******************************************************************************
 * @file        xHal_Rpi5CarWebSearchSystem.cpp
 * @brief       Implements the bounded libcurl web-search transport.
 * @project     xWalk Firmware
 * @module      xWalkWebSearch
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarWebSearch.h"
#include "xHal_Rpi5CarTrace.h"
#include <curl/curl.h>
#include <limits>

namespace xwalk::hal
{

    XWalkWebSearchOperations XWalkWebSearch::systemOperations() noexcept
    {
        return {&systemGetJson};
    }

    string XWalkWebSearch::systemGetJson(contextpointer context, stringview url, uint32 timeoutMs, size maximumBytes)
    {
        static_cast<void>(context);
        static const CURLcode initialization = ::curl_global_init(CURL_GLOBAL_DEFAULT);
        if (initialization != CURLE_OK)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Web-search HTTP initialization failed");
        }
        CURL* request = ::curl_easy_init();
        if (request == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Web-search HTTP allocation failed");
        }
        XWalkWebSearchResponseState state{{}, maximumBytes};
        const string urlCopy(url);
        const long timeout = static_cast<long>(timeoutMs);
        CURLcode result = ::curl_easy_setopt(request, CURLOPT_URL, urlCopy.c_str());
        if (result == CURLE_OK)
        {
            result = ::curl_easy_setopt(request, CURLOPT_TIMEOUT_MS, timeout);
        }
        if (result == CURLE_OK)
        {
            result = ::curl_easy_setopt(request, CURLOPT_NOSIGNAL, 1L);
        }
        if (result == CURLE_OK)
        {
            result = ::curl_easy_setopt(request, CURLOPT_PROXY, "");
        }
        if (result == CURLE_OK)
        {
            result = ::curl_easy_setopt(request, CURLOPT_FOLLOWLOCATION, 0L);
        }
        if (result == CURLE_OK)
        {
            result = ::curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, &writeResponse);
        }
        if (result == CURLE_OK)
        {
            result = ::curl_easy_setopt(request, CURLOPT_WRITEDATA, &state);
        }
        if (result == CURLE_OK)
        {
            result = ::curl_easy_perform(request);
        }
        long status{};
        const CURLcode statusResult = ::curl_easy_getinfo(request, CURLINFO_RESPONSE_CODE, &status);
        ::curl_easy_cleanup(request);
        if ((result != CURLE_OK) || (statusResult != CURLE_OK) || (status < 200L) || (status >= 300L))
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Local web-search request failed");
        }
        return state.response;
    }

    size XWalkWebSearch::writeResponse(charpointer data, size itemSize, size itemCount, contextpointer userData)
    {
        const size maximumSize = std::numeric_limits<size>::max();
        if ((data == nullptr) || (userData == nullptr) || ((itemCount != 0U) && (itemSize > (maximumSize / itemCount))))
        {
            return 0U;
        }
        const size count = itemSize * itemCount;
        XWalkWebSearchResponseState& state = *static_cast<XWalkWebSearchResponseState*>(userData);
        const size responseSize = state.response.size();
        if ((responseSize > state.maximumBytes) || (count > (state.maximumBytes - responseSize)))
        {
            return 0U;
        }
        state.response.append(data, count);
        return count;
    }

} /* namespace xwalk::hal */
