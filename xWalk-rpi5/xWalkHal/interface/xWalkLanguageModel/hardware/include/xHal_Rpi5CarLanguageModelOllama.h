/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllama.h
 * @brief       Declares a bounded Ollama and OpenAI-compatible HTTP backend.
 *
 * @details
 * Owns provider configuration and bounded conversation history, converts
 * Ollama or OpenAI-compatible requests and responses, optionally encodes
 * images, and uses libcurl or an injected synchronous HTTP transport.
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

#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelOllamaTypes.h"

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
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkLanguageModelOllama
     * @brief Implements one bounded local or authenticated HTTP chat provider.
     *
     * @details The backend owns dialect, endpoint, model, optional credential,
     * instructions, welcome text, and retained messages. Calls require external
     * serialization and never emit request, response, image, or credential diagnostics.
     */
    class XWalkLanguageModelOllama final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Nullable non-owning HTTP context that must outlive this backend. */
            contextpointer transportContext{nullptr};

            /** @brief Complete HTTP operation table copied during construction. */
            XWalkLanguageModelOllamaOperations operations{};

            /** @brief Owned non-empty Ollama `/api/chat` endpoint. */
            string endpointValue{};

            /** @brief Owned non-empty deployment-selected Ollama model name. */
            string modelValue{};

            /** @brief Selected native Ollama or OpenAI-compatible HTTP dialect. */
            XWalkLanguageModelHttpDialect dialectValue{XWalkLanguageModelHttpDialect::Ollama};

            /** @brief Owned API key used only to construct one authorization header. */
            string apiKeyValue{};

            /** @brief Owned optional system instructions. */
            string instructionsValue{};

            /** @brief Owned optional assistant welcome text. */
            string welcomeValue{};

            /** @brief Bounded owned conversation history excluding current prompts. */
            languagemodelollamamessagevector history{};

            /** @brief Retained-message limit from one through two hundred. */
            uint32 maximumMessagesValue{XHAL_RPI5CAR_LANGUAGE_MODEL_DEFAULT_MAXIMUM_MESSAGES};

            /** @brief Synchronous HTTP timeout in milliseconds. */
            uint32 timeoutMsValue{XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS};

            /** @brief Requested completion output-token bound for compatible APIs. */
            uint32 maximumOutputTokensValue{XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_DEFAULT_MAXIMUM_OUTPUT_TOKENS};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Returns the complete real libcurl transport operation table. */
            static XWalkLanguageModelOllamaOperations systemOperations() noexcept;

            /**
             * @brief Sends one real bounded JSON POST request through libcurl.
             *
             * @param[in,out] context Unused nullable transport context.
             * @param[in] endpoint Non-empty Ollama chat endpoint.
             * @param[in] requestJson Bounded complete JSON request.
             * @param[in] authorizationHeader Empty text or a complete authorization header.
             * @param[in] timeoutMs Request timeout in milliseconds.
             * @param[in] maximumResponseBytes Maximum accepted response bytes.
             * @return Owned successful two-hundred-range JSON response.
             * @throws std::runtime_error If libcurl setup, transfer, or HTTP status fails.
             */
            static string systemPostJson(contextpointer context,
                                         stringview endpoint,
                                         stringview requestJson,
                                         stringview authorizationHeader,
                                         uint32 timeoutMs,
                                         size maximumResponseBytes);

            /**
             * @brief Appends one bounded libcurl response block.
             *
             * @param[in] data Received byte block.
             * @param[in] itemSize Byte width of one received item.
             * @param[in] itemCount Number of received items.
             * @param[in,out] userData Non-null owned response string for this request.
             * @return Accepted byte count, or zero when the response limit would be exceeded.
             */
            static size systemWriteResponse(charpointer data, size itemSize, size itemCount, contextpointer userData);

            /**
             * @brief Validates construction configuration and operations.
             *
             * @param[in] backendOperations Operation table requiring a non-null POST callback.
             * @param[in] endpoint Non-empty endpoint within the provider text limit.
             * @param[in] model Non-empty model name within the provider text limit.
             * @param[in] dialect Selected supported HTTP dialect.
             * @param[in] apiKey Empty for Ollama or a required cloud credential.
             * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
             * @param[in] maximumOutputTokens Non-zero bounded compatible completion limit.
             * @throws std::invalid_argument If a callback, endpoint, or model is invalid.
             * @throws std::out_of_range If a text or timeout limit is exceeded.
             */
            static void validateConstruction(const XWalkLanguageModelOllamaOperations& backendOperations,
                                             stringview endpoint,
                                             stringview model,
                                             XWalkLanguageModelHttpDialect dialect,
                                             stringview apiKey,
                                             uint32 timeoutMs,
                                             uint32 maximumOutputTokens);

            /**
             * @brief Validates one role enumerator.
             * @param[in] role Role to inspect.
             * @throws std::invalid_argument If the role is unsupported.
             */
            static void validateRole(XWalkLanguageModelRole role);

            /**
             * @brief Validates one bounded provider text value.
             * @param[in] text Text whose encoded byte length is inspected.
             * @throws std::out_of_range If the provider text limit is exceeded.
             */
            static void validateText(stringview text);

            /**
             * @brief Converts one callback context into its required provider.
             * @param[in,out] context Non-null pointer to a live provider backend.
             * @return Referenced provider backend.
             * @throws std::invalid_argument If `context` is null.
             */
            static XWalkLanguageModelOllama& provider(contextpointer context);

            /**
             * @brief Replaces provider system instructions after bounded validation.
             * @param[in,out] context Non-null provider callback context.
             * @param[in] instructions Instruction text retained as an owned copy.
             * @throws std::out_of_range If the provider text limit is exceeded.
             */
            static void setInstructions(contextpointer context, stringview instructions);

            /**
             * @brief Replaces provider welcome text after bounded validation.
             * @param[in,out] context Non-null provider callback context.
             * @param[in] welcome Welcome text retained as an owned copy.
             * @throws std::out_of_range If the provider text limit is exceeded.
             */
            static void setWelcome(contextpointer context, stringview welcome);

            /**
             * @brief Applies and enforces a retained-message limit.
             * @param[in,out] context Non-null provider callback context.
             * @param[in] maximumMessages Limit from one through two hundred.
             * @throws std::out_of_range If the limit is outside its range.
             */
            static void setMaximumMessages(contextpointer context, uint32 maximumMessages);

            /**
             * @brief Adds one validated bounded message and optional encoded image.
             * @param[in,out] context Non-null provider callback context.
             * @param[in] role Valid conversation participant role.
             * @param[in] content Message content retained as an owned copy.
             * @param[in] imagePath Empty path or image encoded immediately by this call.
             * @throws std::invalid_argument If `role` is unsupported.
             * @throws std::out_of_range If text or image data exceeds a bound.
             * @throws std::runtime_error If a selected image cannot be read.
             */
            static void
            addMessage(contextpointer context, XWalkLanguageModelRole role, stringview content, stringview imagePath);

            /**
             * @brief Sends one bounded chat request and retains its successful exchange.
             * @param[in,out] context Non-null provider callback context.
             * @param[in] promptText Prompt content retained only after success.
             * @param[in] imagePath Empty path or image encoded for the current request.
             * @return Owned final assistant content, which may be empty.
             * @throws std::out_of_range If request or response data exceeds a bound.
             * @throws std::runtime_error If image, transport, HTTP, or parsing fails.
             */
            static string prompt(contextpointer context, stringview promptText, stringview imagePath);

            /**
             * @brief Stores one owned message and applies history truncation.
             * @param[in] message Validated owned message to retain.
             */
            void storeMessage(const XWalkLanguageModelOllamaMessage& message);

            /**
             * @brief Reads and base64-encodes one bounded image.
             * @param[in] imagePath Empty path or existing image file path.
             * @return Empty text for no image, otherwise base64 data without a URI prefix.
             * @throws std::out_of_range If the raw image limit is exceeded.
             * @throws std::runtime_error If the image cannot be read.
             */
            static string encodeImage(stringview imagePath);

            /**
             * @brief Serializes one complete non-streaming Ollama chat request.
             * @param[in] currentPrompt Validated current user message.
             * @return Complete JSON request within the configured byte limit.
             * @throws std::out_of_range If serialization exceeds the request limit.
             */
            string buildRequest(const XWalkLanguageModelOllamaMessage& currentPrompt) const;

            /**
             * @brief Appends one JSON string with escaping and request-bound checks.
             * @param[in,out] output Request JSON receiving escaped bytes.
             * @param[in] value UTF-8 bytes retained only for this call.
             * @throws std::out_of_range If serialization exceeds the request limit.
             */
            static void appendJsonString(string& output, stringview value);

            /**
             * @brief Appends one complete Ollama message object to a JSON request.
             * @param[in,out] output Request JSON receiving one message.
             * @param[in] message Validated owned message.
             * @throws std::out_of_range If serialization exceeds the request limit.
             */
            void appendMessageJson(string& output, const XWalkLanguageModelOllamaMessage& message) const;

            /**
             * @brief Returns the Ollama JSON role name for one validated role.
             * @param[in] role Valid system, user, or assistant role.
             * @return Static lowercase role name.
             * @throws std::invalid_argument If the role is unsupported.
             */
            static stringview roleName(XWalkLanguageModelRole role);

            /**
             * @brief Extracts and decodes `message.content` from an Ollama response.
             * @param[in] responseJson Complete bounded provider response.
             * @return Owned decoded response content, which may be empty.
             * @throws std::runtime_error If required JSON structure is malformed or absent.
             */
            static string extractResponseContent(stringview responseJson);

            /**
             * @brief Decodes one JSON string beginning at its opening quote.
             * @param[in] json Complete JSON response.
             * @param[in] quoteOffset Offset of the opening quote.
             * @return Owned decoded Unicode text.
             * @throws std::runtime_error If escaping or Unicode data is malformed.
             */
            static string decodeJsonString(stringview json, size quoteOffset);

            /**
             * @brief Decodes one hexadecimal JSON Unicode digit.
             * @param[in] value ASCII hexadecimal digit.
             * @return Numeric digit from zero through fifteen.
             * @throws std::runtime_error If `value` is not hexadecimal.
             */
            static uint8 hexadecimalDigit(char value);

            /**
             * @brief Appends one validated Unicode code point as UTF-8.
             * @param[in,out] output Decoded response receiving UTF-8 bytes.
             * @param[in] codePoint Unicode scalar value from zero through `0x10FFFF`.
             * @throws std::runtime_error If `codePoint` is not a scalar value.
             */
            static void appendUtf8(string& output, uint32 codePoint);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs one Ollama provider using real libcurl transport.
             * @param[in] endpoint Non-empty deployment-selected `/api/chat` endpoint.
             * @param[in] model Non-empty deployment-selected Ollama model name.
             * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
             * @throws std::invalid_argument If endpoint or model is empty.
             * @throws std::out_of_range If a text or timeout limit is exceeded.
             */
            XWalkLanguageModelOllama(stringview endpoint,
                                     stringview model,
                                     uint32 timeoutMs = XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS);

            /**
             * @brief Constructs one Ollama provider with an injected HTTP transport.
             * @param[in,out] context Nullable non-owning context that must outlive this backend.
             * @param[in] backendOperations Complete HTTP operation table copied by the backend.
             * @param[in] endpoint Non-empty deployment-selected `/api/chat` endpoint.
             * @param[in] model Non-empty deployment-selected Ollama model name.
             * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
             * @throws std::invalid_argument If the callback, endpoint, or model is invalid.
             * @throws std::out_of_range If a text or timeout limit is exceeded.
             */
            XWalkLanguageModelOllama(contextpointer context,
                                     const XWalkLanguageModelOllamaOperations& backendOperations,
                                     stringview endpoint,
                                     stringview model,
                                     uint32 timeoutMs = XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS);

            /**
             * @brief Constructs one selected HTTP provider using real libcurl transport.
             * @param[in] dialect Native Ollama or OpenAI-compatible request dialect.
             * @param[in] endpoint Complete deployment-selected chat endpoint.
             * @param[in] model Non-empty deployment-selected model identifier.
             * @param[in] apiKey Empty for Ollama or a non-empty cloud API key.
             * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
             * @param[in] maximumOutputTokens Non-zero bounded compatible completion limit.
             */
            XWalkLanguageModelOllama(
                XWalkLanguageModelHttpDialect dialect,
                stringview endpoint,
                stringview model,
                stringview apiKey,
                uint32 timeoutMs = XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS,
                uint32 maximumOutputTokens = XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_DEFAULT_MAXIMUM_OUTPUT_TOKENS);

            /**
             * @brief Constructs one selected HTTP provider with an injected transport.
             * @param[in,out] context Nullable transport context that must outlive this backend.
             * @param[in] backendOperations Complete synchronous transport table.
             * @param[in] dialect Native Ollama or OpenAI-compatible request dialect.
             * @param[in] endpoint Complete deployment-selected chat endpoint.
             * @param[in] model Non-empty deployment-selected model identifier.
             * @param[in] apiKey Empty for Ollama or a non-empty cloud API key.
             * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
             * @param[in] maximumOutputTokens Non-zero bounded compatible completion limit.
             */
            XWalkLanguageModelOllama(
                contextpointer context,
                const XWalkLanguageModelOllamaOperations& backendOperations,
                XWalkLanguageModelHttpDialect dialect,
                stringview endpoint,
                stringview model,
                stringview apiKey,
                uint32 timeoutMs = XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS,
                uint32 maximumOutputTokens = XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_DEFAULT_MAXIMUM_OUTPUT_TOKENS);

            /** @brief Destroys owned history without releasing non-owning transport state. */
            ~XWalkLanguageModelOllama();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkLanguageModelOllama(const XWalkLanguageModelOllama&) = delete;
            XWalkLanguageModelOllama& operator=(const XWalkLanguageModelOllama&) = delete;
            XWalkLanguageModelOllama(XWalkLanguageModelOllama&&) = delete;
            XWalkLanguageModelOllama& operator=(XWalkLanguageModelOllama&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Returns the complete callback table for `XWalkLanguageModel`.
             * @return Five non-null callbacks requiring this backend as context.
             */
            XWalkLanguageModelCallbacks callbacks() const noexcept;

            /**
             * @brief Converts one deployment provider name into a supported HTTP dialect.
             * @param[in] providerName `ollama`, `openai`, `chatgpt`, `gemini`, `claude`,
             * `anthropic`, or `openai_compatible`.
             * @return Selected HTTP dialect.
             * @throws std::invalid_argument If the provider name is unsupported.
             */
            static XWalkLanguageModelHttpDialect dialectFromString(stringview providerName);
    };

    /** @brief Provider-neutral name for the backward-compatible HTTP backend class. */
    using XWalkLanguageModelHttp = XWalkLanguageModelOllama;

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_H */
