/******************************************************************************
 * @file        xHal_Rpi5CarWebSearch.h
 * @brief       Declares the bounded loopback SearXNG search client.
 * @project     xWalk Firmware
 * @module      xWalkWebSearch
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_WEB_SEARCH_H
#define XHAL_RPI5CAR_WEB_SEARCH_H

#include "xHal_Rpi5CarWebSearchTypes.h"

namespace xwalk::hal
{

    /**
     * @class XWalkWebSearch
     * @brief Selects current-information queries and retrieves bounded untrusted references.
     * @details Owns configuration but not an injected transport context. It contacts only the configured
     * loopback SearXNG endpoint and never follows result URLs.
     */
    class XWalkWebSearch final
    {
        private:
            contextpointer transportContext{nullptr};    /**< Nullable non-owning transport context. */
            XWalkWebSearchOperations operations{};       /**< Complete copied transport table. */
            XWalkWebSearchConfiguration configuration{}; /**< Validated owned request limits. */

        protected:
            static XWalkWebSearchOperations systemOperations() noexcept;
            static string systemGetJson(contextpointer context, stringview url, uint32 timeoutMs, size maximumBytes);
            static size writeResponse(charpointer data, size itemSize, size itemCount, contextpointer userData);
            static void validate(const XWalkWebSearchOperations& operations,
                                 const XWalkWebSearchConfiguration& configuration);
            static string encodeQuery(stringview query);
            static string sanitizeText(stringview text);
            static boolean safeResultUrl(stringview url) noexcept;
            static string jsonString(stringview json, stringview key, size start, size& next);

        public:
            explicit XWalkWebSearch(const XWalkWebSearchConfiguration& configuration);
            XWalkWebSearch(contextpointer context,
                           const XWalkWebSearchOperations& operations,
                           const XWalkWebSearchConfiguration& configuration);
            ~XWalkWebSearch();
            XWalkWebSearch(const XWalkWebSearch&) = delete;
            XWalkWebSearch& operator=(const XWalkWebSearch&) = delete;
            XWalkWebSearch(XWalkWebSearch&&) = delete;
            XWalkWebSearch& operator=(XWalkWebSearch&&) = delete;

            /** @brief Returns whether a prompt explicitly or implicitly requires current information. */
            static boolean shouldSearch(stringview prompt);

            /** @brief Retrieves bounded sanitized references without following result URLs. */
            XWalkWebSearchResponse search(stringview query) const;

            /** @brief Adapts a non-null client context to the Agent callback boundary. */
            static XWalkWebSearchResponse searchCallback(contextpointer context, stringview query);
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_WEB_SEARCH_H */
