/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModel.cpp
 * @brief       Implements provider-neutral language-model operations.
 *
 * @details
 * Dispatches conversation configuration, history insertion, and synchronous
 * prompting through a validated application backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarLanguageModel.h"

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
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Replaces the system instructions used for later prompts.
 *
 * @param[in] instructions
 * Instruction text forwarded synchronously without encoding changes.
 *
 * @note
 * Empty instructions are preserved. Backend exceptions are propagated.
 */
void XWalkLanguageModel::setInstructions(stringview instructions)
{
    callbacks.setInstructions(backendContextPointer, instructions);
}

/**
 * @brief Replaces the welcome text associated with the conversation.
 *
 * @param[in] welcome
 * Welcome text forwarded synchronously without encoding changes.
 *
 * @note
 * Empty welcome text is preserved. Backend exceptions are propagated.
 */
void XWalkLanguageModel::setWelcome(stringview welcome)
{
    callbacks.setWelcome(backendContextPointer, welcome);
}

/**
 * @brief Configures the maximum retained conversation-message count.
 *
 * @param[in] maximumMessages
 * Non-zero number of messages retained according to backend policy.
 *
 * @throws std::out_of_range
 * If `maximumMessages` is zero.
 */
void XWalkLanguageModel::setMaximumMessages(uint32 maximumMessages)
{
    validateMaximumMessages(maximumMessages);
    callbacks.setMaximumMessages(backendContextPointer, maximumMessages);
}

/**
 * @brief Adds one message to the backend conversation history.
 *
 * @param[in] role
 * System, user, or assistant participant responsible for the message.
 *
 * @param[in] content
 * Message content forwarded synchronously without encoding changes.
 *
 * @param[in] imagePath
 * Optional image path. An empty view indicates no attached image.
 *
 * @note
 * The backend owns history truncation and image-processing policy.
 */
void XWalkLanguageModel::addMessage(XWalkLanguageModelRole role, stringview content, stringview imagePath)
{
    callbacks.addMessage(backendContextPointer, role, content, imagePath);
}

/**
 * @brief Submits one prompt and returns the final language-model response.
 *
 * @param[in] promptText
 * Prompt text forwarded synchronously without encoding changes.
 *
 * @param[in] imagePath
 * Optional image path. An empty view indicates a text-only prompt.
 *
 * @return
 * Owned final response text, including an empty response when returned by the backend.
 *
 * @warning
 * The injected callback may block on model inference, a process, or a network request.
 */
string XWalkLanguageModel::prompt(stringview promptText, stringview imagePath)
{
    return callbacks.prompt(backendContextPointer, promptText, imagePath);
}

} /* namespace xwalk::hal */
