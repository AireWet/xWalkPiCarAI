/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaCallbacks.cpp
 * @brief       Implements Ollama conversation and prompt callbacks.
 *
 * @details
 * Retains bounded conversation state, performs one synchronous chat request,
 * and stores only successful user and assistant exchanges.
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
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Replaces provider system instructions after bounded validation.
 *
 * @param[in,out] context Non-null provider callback context.
 * @param[in] instructions Instruction text retained as an owned copy.
 * @throws std::out_of_range If the provider text limit is exceeded.
 */
void XWalkLanguageModelOllama::setInstructions(contextpointer context,
    stringview instructions)
{
    validateText(instructions);
    provider(context).instructionsValue = instructions;
}

/**
 * @brief Replaces provider welcome text after bounded validation.
 *
 * @param[in,out] context Non-null provider callback context.
 * @param[in] welcome Welcome text retained as an owned copy.
 * @throws std::out_of_range If the provider text limit is exceeded.
 */
void XWalkLanguageModelOllama::setWelcome(contextpointer context, stringview welcome)
{
    validateText(welcome);
    provider(context).welcomeValue = welcome;
}

/**
 * @brief Applies and enforces a retained-message limit.
 *
 * @param[in,out] context Non-null provider callback context.
 * @param[in] maximumMessages Limit from one through two hundred.
 * @throws std::out_of_range If the limit is zero or exceeds two hundred.
 */
void XWalkLanguageModelOllama::setMaximumMessages(contextpointer context,
    uint32 maximumMessages)
{
    if ((maximumMessages == 0U) ||
        (maximumMessages > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_MESSAGES))
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama history limit is outside its supported range");
    }
    XWalkLanguageModelOllama& self = provider(context);
    self.maximumMessagesValue = maximumMessages;
    const size maximumSize = static_cast<size>(maximumMessages);
    const hal::boolean historyTooLarge =
        static_cast<hal::boolean>(
            self.history.size() > maximumSize);
    if (historyTooLarge)
    {
        const size eraseCount = self.history.size() - maximumSize;
        self.history.erase(self.history.begin(), self.history.begin() +
            static_cast<languagemodelollamamessagevector::difference_type>(eraseCount));
    }
}

/**
 * @brief Adds one validated bounded message and optional encoded image.
 *
 * @param[in,out] context Non-null provider callback context.
 * @param[in] role Valid conversation participant role.
 * @param[in] content Message content retained as an owned copy.
 * @param[in] imagePath Empty path or image encoded immediately by this call.
 * @throws std::invalid_argument If `role` is unsupported.
 * @throws std::out_of_range If text or image data exceeds a bound.
 * @throws std::runtime_error If a selected image cannot be read.
 */
void XWalkLanguageModelOllama::addMessage(contextpointer context,
    XWalkLanguageModelRole role, stringview content, stringview imagePath)
{
    validateRole(role);
    validateText(content);
    const XWalkLanguageModelOllamaMessage message{
        role, string(content), encodeImage(imagePath)};
    provider(context).storeMessage(message);
}

/**
 * @brief Sends one bounded chat request and retains its successful exchange.
 *
 * @param[in,out] context Non-null provider callback context.
 * @param[in] promptText Prompt content retained only after a successful request.
 * @param[in] imagePath Empty path or image encoded for the current request.
 * @return Owned final assistant content, which may be empty.
 * @throws std::out_of_range If text, image, request, or response content exceeds a bound.
 * @throws std::runtime_error If image, transport, HTTP, or response parsing fails.
 */
string XWalkLanguageModelOllama::prompt(contextpointer context,
    stringview promptText, stringview imagePath)
{
    validateText(promptText);
    XWalkLanguageModelOllama& self = provider(context);
    const XWalkLanguageModelOllamaMessage currentPrompt{
        XWalkLanguageModelRole::User, string(promptText), encodeImage(imagePath)};
    const string requestJson = self.buildRequest(currentPrompt);
    string authorizationHeader{};
    const hal::boolean apiKeyAvailable =
        static_cast<hal::boolean>(
            !self.apiKeyValue.empty());
    if (apiKeyAvailable)
    {
        authorizationHeader = "Authorization: Bearer ";
        authorizationHeader += self.apiKeyValue;
    }
    const string responseJson = self.operations.postJson(self.transportContext,
        self.endpointValue, requestJson, authorizationHeader, self.timeoutMsValue,
        XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_RESPONSE_BYTES);
    const hal::boolean responseJsonTooLarge =
        static_cast<hal::boolean>(
            responseJson.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_RESPONSE_BYTES);
    if (responseJsonTooLarge)
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama response exceeds its bounded byte count");
    }
    const string responseContent = extractResponseContent(responseJson);
    validateText(responseContent);
    self.storeMessage(currentPrompt);
    self.storeMessage({XWalkLanguageModelRole::Assistant, responseContent, {}});
    return responseContent;
}

/**
 * @brief Stores one owned message and applies history truncation.
 *
 * @param[in] message Validated owned message to retain.
 * @post History contains no more than `maximumMessagesValue` entries.
 */
void XWalkLanguageModelOllama::storeMessage(
    const XWalkLanguageModelOllamaMessage& message)
{
    history.push_back(message);
    const size maximumSize = static_cast<size>(maximumMessagesValue);
    const hal::boolean historyLimitExceeded =
        static_cast<hal::boolean>(
            history.size() > maximumSize);
    if (historyLimitExceeded)
    {
        history.erase(history.begin());
    }
}

} /* namespace xwalk::hal */
