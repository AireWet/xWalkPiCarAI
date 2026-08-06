/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaLifecycle.cpp
 * @brief       Implements Ollama provider validation and lifecycle.
 *
 * @details
 * Validates bounded deployment configuration, binds real or injected HTTP
 * transport, and publishes the coordinator callback table.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelOllama.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs one Ollama provider using real libcurl transport.
 *
 * @param[in] endpoint Non-empty HTTP or HTTPS endpoint ending in `/api/chat`.
 * @param[in] model Non-empty deployment-selected Ollama model name.
 * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
 * @throws std::invalid_argument If endpoint or model is invalid.
 * @throws std::out_of_range If a text or timeout limit is exceeded.
 */
XWalkLanguageModelOllama::XWalkLanguageModelOllama(stringview endpoint,
    stringview model, uint32 timeoutMs):
    XWalkLanguageModelOllama(nullptr, systemOperations(),
        XWalkLanguageModelHttpDialect::Ollama, endpoint, model, {}, timeoutMs,
        XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_DEFAULT_MAXIMUM_OUTPUT_TOKENS)
{
}

/**
 * @brief Constructs one Ollama provider with an injected HTTP transport.
 *
 * @param[in,out] context Nullable non-owning context that must outlive this backend.
 * @param[in] backendOperations Complete HTTP operation table copied by the backend.
 * @param[in] endpoint Non-empty HTTP or HTTPS endpoint ending in `/api/chat`.
 * @param[in] model Non-empty deployment-selected Ollama model name.
 * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
 * @throws std::invalid_argument If the callback, endpoint, or model is invalid.
 * @throws std::out_of_range If a text or timeout limit is exceeded.
 */
XWalkLanguageModelOllama::XWalkLanguageModelOllama(contextpointer context,
    const XWalkLanguageModelOllamaOperations& backendOperations,
    stringview endpoint, stringview model, uint32 timeoutMs):
    XWalkLanguageModelOllama(context, backendOperations,
        XWalkLanguageModelHttpDialect::Ollama, endpoint, model, {}, timeoutMs,
        XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_DEFAULT_MAXIMUM_OUTPUT_TOKENS)
{
}

/**
 * @brief Constructs one selected HTTP provider using real libcurl transport.
 * @param[in] dialect Native Ollama or OpenAI-compatible request dialect.
 * @param[in] endpoint Complete deployment-selected chat endpoint.
 * @param[in] model Non-empty deployment-selected model identifier.
 * @param[in] apiKey Empty for Ollama or a non-empty cloud API key.
 * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
 * @param[in] maximumOutputTokens Non-zero bounded compatible completion limit.
 */
XWalkLanguageModelOllama::XWalkLanguageModelOllama(
    XWalkLanguageModelHttpDialect dialect, stringview endpoint,
    stringview model, stringview apiKey, uint32 timeoutMs,
    uint32 maximumOutputTokens):
    XWalkLanguageModelOllama(nullptr, systemOperations(), dialect, endpoint,
        model, apiKey, timeoutMs, maximumOutputTokens)
{
}

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
XWalkLanguageModelOllama::XWalkLanguageModelOllama(contextpointer context,
    const XWalkLanguageModelOllamaOperations& backendOperations,
    XWalkLanguageModelHttpDialect dialect, stringview endpoint,
    stringview model, stringview apiKey, uint32 timeoutMs,
    uint32 maximumOutputTokens):
    transportContext(context), operations(backendOperations), endpointValue(endpoint),
    modelValue(model), dialectValue(dialect), apiKeyValue(apiKey),
    timeoutMsValue(timeoutMs), maximumOutputTokensValue(maximumOutputTokens)
{
    validateConstruction(operations, endpointValue, modelValue, dialectValue,
        apiKeyValue, timeoutMsValue, maximumOutputTokensValue);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys owned history without releasing non-owning transport state.
 */
XWalkLanguageModelOllama::~XWalkLanguageModelOllama() = default;

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns the complete callback table for `XWalkLanguageModel`.
 * @return Five non-null callbacks requiring this backend as context.
 */
XWalkLanguageModelCallbacks XWalkLanguageModelOllama::callbacks() const noexcept
{
    return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &prompt};
}

/**
 * @brief Converts one deployment provider name into a supported HTTP dialect.
 * @param[in] providerName Supported deployment provider identifier.
 * @return Selected HTTP dialect.
 * @throws std::invalid_argument If the provider name is unsupported.
 */
XWalkLanguageModelHttpDialect XWalkLanguageModelOllama::dialectFromString(
    stringview providerName)
{
    if (providerName == "ollama")
    {
        return XWalkLanguageModelHttpDialect::Ollama;
    }
    if ((providerName == "openai") || (providerName == "chatgpt") ||
        (providerName == "gemini") || (providerName == "claude") ||
        (providerName == "anthropic") || (providerName == "openai_compatible"))
    {
        return XWalkLanguageModelHttpDialect::OpenAiChatCompletions;
    }
    if (providerName == "kiro")
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Kiro has no supported model-selectable HTTP inference endpoint");
    }
    XHAL_THROW_INVALID_ARGUMENT("Language-model provider is unsupported");
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates construction configuration and operations.
 *
 * @param[in] backendOperations Operation table requiring a non-null POST callback.
 * @param[in] endpoint Non-empty HTTP or HTTPS endpoint ending in `/api/chat`.
 * @param[in] model Non-empty model name within the provider text limit.
 * @param[in] timeoutMs Timeout from 1 through 300,000 milliseconds.
 * @throws std::invalid_argument If a callback, endpoint, or model is invalid.
 * @throws std::out_of_range If a text or timeout limit is exceeded.
 */
void XWalkLanguageModelOllama::validateConstruction(
    const XWalkLanguageModelOllamaOperations& backendOperations,
    stringview endpoint, stringview model, XWalkLanguageModelHttpDialect dialect,
    stringview apiKey, uint32 timeoutMs, uint32 maximumOutputTokens)
{
    if (backendOperations.postJson == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Ollama backend requires an HTTP transport");
    }
    validateText(endpoint);
    validateText(model);
    const boolean httpEndpoint = endpoint.substr(0U, 7U) == "http://";
    const boolean httpsEndpoint = endpoint.substr(0U, 8U) == "https://";
    const stringview chatSuffix = (dialect == XWalkLanguageModelHttpDialect::Ollama) ?
        stringview{"/api/chat"} : stringview{"/chat/completions"};
    const boolean hasChatSuffix = (endpoint.size() >= chatSuffix.size()) &&
        (endpoint.substr(endpoint.size() - chatSuffix.size()) == chatSuffix);
    if ((dialect != XWalkLanguageModelHttpDialect::Ollama) &&
        (dialect != XWalkLanguageModelHttpDialect::OpenAiChatCompletions))
    {
        XHAL_THROW_INVALID_ARGUMENT("Language-model HTTP dialect is invalid");
    }
    const hal::boolean endpointModelHttpEndpointInvalid =
        static_cast<hal::boolean>(
            endpoint.empty() || model.empty() || (!httpEndpoint && !httpsEndpoint) || !hasChatSuffix ||
        (endpoint.find('\0') != stringview::npos) || (endpoint.find('\n') != stringview::npos) ||
        (endpoint.find('\r') != stringview::npos));
    if (endpointModelHttpEndpointInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Ollama endpoint or model is invalid");
    }
    const hal::boolean secureEndpointConfigurationInvalid =
        static_cast<hal::boolean>(
            (dialect == XWalkLanguageModelHttpDialect::OpenAiChatCompletions) &&
        (!httpsEndpoint || apiKey.empty()));
    if (secureEndpointConfigurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "OpenAI-compatible providers require HTTPS and a non-empty API key");
    }
    const hal::boolean ollamaCredentialConfigured =
        static_cast<hal::boolean>(
            (dialect == XWalkLanguageModelHttpDialect::Ollama) && !apiKey.empty());
    if (ollamaCredentialConfigured)
    {
        XHAL_THROW_INVALID_ARGUMENT("Ollama does not accept a configured API key");
    }
    const hal::boolean apiKeyNRInvalid =
        static_cast<hal::boolean>(
            (apiKey.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_API_KEY_BYTES) ||
        (apiKey.find('\0') != stringview::npos) ||
        (apiKey.find('\n') != stringview::npos) ||
        (apiKey.find('\r') != stringview::npos));
    if (apiKeyNRInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Language-model API key is invalid");
    }
    if ((timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TIMEOUT_MS))
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama timeout is outside its supported range");
    }
    if ((maximumOutputTokens == 0U) ||
        (maximumOutputTokens > XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_OUTPUT_TOKENS))
    {
        XHAL_THROW_OUT_OF_RANGE("Language-model output-token limit is outside its range");
    }
}

/**
 * @brief Validates one role enumerator.
 *
 * @param[in] role Role to inspect.
 * @throws std::invalid_argument If the role is unsupported.
 */
void XWalkLanguageModelOllama::validateRole(XWalkLanguageModelRole role)
{
    if ((role != XWalkLanguageModelRole::System) &&
        (role != XWalkLanguageModelRole::User) &&
        (role != XWalkLanguageModelRole::Assistant))
    {
        XHAL_THROW_INVALID_ARGUMENT("Ollama message role is invalid");
    }
}

/**
 * @brief Validates one bounded provider text value.
 *
 * @param[in] text Text whose encoded byte length is inspected.
 * @throws std::out_of_range If the provider text limit is exceeded.
 */
void XWalkLanguageModelOllama::validateText(stringview text)
{
    const hal::boolean textTooLarge =
        static_cast<hal::boolean>(
            text.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TEXT_BYTES);
    if (textTooLarge)
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama text exceeds its bounded byte count");
    }
}

/**
 * @brief Converts one callback context into its required provider.
 *
 * @param[in,out] context Non-null pointer to a live provider backend.
 * @return Referenced provider backend.
 * @throws std::invalid_argument If `context` is null.
 */
XWalkLanguageModelOllama& XWalkLanguageModelOllama::provider(contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Ollama callback context must not be null");
    }
    return *static_cast<XWalkLanguageModelOllama*>(context);
}

} /* namespace xwalk::hal */
