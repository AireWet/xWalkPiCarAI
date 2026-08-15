/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaTypes.h
 * @brief       Declares Ollama provider messages and HTTP operations.
 *
 * @details
 * Defines owned history entries plus an injectable JSON POST transport used by
 * native Ollama and OpenAI-compatible HTTP providers in deterministic tests.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Ollama Backend
 *
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_TYPES_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Enumeration declarations
     ******************************************************************************/

    /**
     * @enum XWalkLanguageModelHttpDialect
     * @brief Selects the JSON and authentication contract used by the HTTP backend.
     */
    enum class XWalkLanguageModelHttpDialect : uint8
    {
        /**
         * @brief Uses Ollama `/api/chat` JSON without a required API key.
         */
        Ollama = 0U,

        /**
         * @brief Uses authenticated OpenAI-compatible `/chat/completions` JSON.
         */
        OpenAiChatCompletions = 1U
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkLanguageModelOllamaMessage
     * @brief Contains one owned language-model conversation message.
     */
    struct XWalkLanguageModelOllamaMessage
    {
            /** @brief Valid system, user, or assistant participant role. */
            XWalkLanguageModelRole role{XWalkLanguageModelRole::System};

            /** @brief Owned message content bounded by the provider text limit. */
            string content{};

            /** @brief Optional owned base64 image data without a URI prefix. */
            string imageBase64{};
    };

    /**
     * @struct XWalkLanguageModelOllamaResponseState
     * @brief Retains one libcurl response and its request-specific byte bound.
     */
    struct XWalkLanguageModelOllamaResponseState
    {
            /** @brief Owned response bytes appended only during one synchronous request. */
            string response{};

            /** @brief Maximum response bytes accepted by the active request. */
            size maximumBytes{};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Dynamically sized bounded sequence of owned model messages. */
    using languagemodelollamamessagevector = std::vector<XWalkLanguageModelOllamaMessage>;

    /**
     * @brief Sends one bounded JSON request and returns a bounded JSON response.
     *
     * @param[in,out] context Nullable non-owning transport context that outlives the backend.
     * @param[in] endpoint Non-empty selected chat endpoint retained only for this call.
     * @param[in] requestJson Complete JSON request within the documented request limit.
     * @param[in] authorizationHeader Empty text or one complete authorization header.
     * @param[in] timeoutMs Request timeout from 1 through 300,000 milliseconds.
     * @param[in] maximumResponseBytes Maximum response bytes accepted by the transport.
     * @return Owned complete JSON response.
     * @warning Request content and response content must not be logged by normal diagnostics.
     */
    using languagemodelollamapostcallback = string (*)(contextpointer context,
                                                       stringview endpoint,
                                                       stringview requestJson,
                                                       stringview authorizationHeader,
                                                       uint32 timeoutMs,
                                                       size maximumResponseBytes);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkLanguageModelOllamaOperations
     * @brief Contains the complete injectable language-model HTTP transport table.
     */
    struct XWalkLanguageModelOllamaOperations
    {
            /** @brief Sends one synchronous application/json POST request. */
            languagemodelollamapostcallback postJson{nullptr};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_TYPES_H */
