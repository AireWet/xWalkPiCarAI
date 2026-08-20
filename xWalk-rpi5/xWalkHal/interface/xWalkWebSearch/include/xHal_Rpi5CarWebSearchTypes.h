/******************************************************************************
 * @file        xHal_Rpi5CarWebSearchTypes.h
 * @brief       Declares bounded local web-search configuration and results.
 * @project     xWalk Firmware
 * @module      xWalkWebSearch
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_WEB_SEARCH_TYPES_H
#define XHAL_RPI5CAR_WEB_SEARCH_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::hal
{

    /** @brief Contains validated limits for one loopback SearXNG client. */
    struct XWalkWebSearchConfiguration
    {
            string endpoint{"http://127.0.0.1:8080/search"}; /**< Loopback SearXNG search endpoint. */
            uint32 maximumResults{3U};                       /**< Result limit from one through ten. */
            uint32 timeoutMs{5'000U};                        /**< HTTP timeout from one through 30,000 ms. */
            size maximumResponseBytes{262'144U};             /**< JSON byte limit from 1,024 through 1,048,576. */
    };

    /** @brief Contains bounded untrusted references and printable source attribution. */
    struct XWalkWebSearchResponse
    {
            string referenceText{};     /**< Delimited untrusted reference text for the model. */
            stringvector sourceNames{}; /**< Sanitized source names in response order. */
            stringvector sourceUrls{};  /**< Validated public HTTP(S) result URLs. */
    };

    /** @brief Retains one bounded system-transport response. */
    struct XWalkWebSearchResponseState
    {
            string response{};   /**< Owned response bytes received so far. */
            size maximumBytes{}; /**< Hard response byte limit. */
    };

    /**
     * @brief Retrieves one bounded JSON document from the configured local endpoint.
     * @param[in,out] context Nullable non-owning transport context.
     * @param[in] requestUrl Complete loopback URL with an encoded query.
     * @param[in] timeoutMs Timeout from one through 30,000 milliseconds.
     * @param[in] maximumBytes Maximum accepted response bytes.
     * @return Owned JSON response no larger than `maximumBytes`.
     */
    using websearchgetcallback = string (*)(contextpointer context,
                                            stringview requestUrl,
                                            uint32 timeoutMs,
                                            size maximumBytes);

    /** @brief Groups the complete injectable web-search transport table. */
    struct XWalkWebSearchOperations
    {
            websearchgetcallback getJson{nullptr}; /**< Performs one synchronous bounded JSON GET. */
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_WEB_SEARCH_TYPES_H */
