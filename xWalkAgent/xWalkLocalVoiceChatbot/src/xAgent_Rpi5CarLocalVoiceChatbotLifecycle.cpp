/******************************************************************************
 * @file        xAgent_Rpi5CarLocalVoiceChatbotLifecycle.cpp
 * @brief       Implements local voice-chatbot construction and execution.
 *
 * @project     xWalk Firmware
 * @module      xWalkLocalVoiceChatbot
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xAgent_Rpi5CarLocalVoiceChatbot.h"

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarSpeechToTextTypes.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

XWalkLocalVoiceChatbot::XWalkLocalVoiceChatbot(
    hal::XWalkVoiceAssistant& assistant, hal::contextpointer context,
    const XWalkLocalVoiceChatbotCallbacks& backendCallbacks,
    const XWalkLocalVoiceChatbotConfiguration& chatbotConfiguration):
    assistantObject(&assistant), callbackContext(context),
    callbacks(backendCallbacks), configuration(chatbotConfiguration)
{
    validate(callbacks, configuration);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

hal::int32 XWalkLocalVoiceChatbot::run()
{
    assistantObject->start();
    while (callbacks.shouldContinue(callbackContext))
    {
        callbacks.output(callbackContext, "Listening... (Press Ctrl+C to stop)");
        const hal::string recognizedText =
            assistantObject->listen(configuration.listenTimeoutMs);
        if (recognizedText.empty())
        {
            callbacks.output(callbackContext, configuration.silenceMessage);
            callbacks.delay(callbackContext, 100U);
            continue;
        }

        callbacks.output(callbackContext, hal::string("[YOU] ") + recognizedText);
        const hal::string response = assistantObject->think(recognizedText);
        callbacks.output(callbackContext, response);
        const hal::string cleanResponse = stripThinking(response);
        assistantObject->say(cleanResponse.empty() ? configuration.emptyResponse : cleanResponse);
        callbacks.delay(callbackContext, 50U);
    }

    assistantObject->say(configuration.goodbye);
    assistantObject->stop();
    callbacks.output(callbackContext, "Bye.");
    return 0;
}

/** @brief Requests recognition shutdown and stops the assistant. */
void XWalkLocalVoiceChatbot::stop()
{
    assistantObject->stop();
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

void XWalkLocalVoiceChatbot::validate(
    const XWalkLocalVoiceChatbotCallbacks& backendCallbacks,
    const XWalkLocalVoiceChatbotConfiguration& chatbotConfiguration)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr) ||
        (backendCallbacks.delay == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Local voice-chatbot callbacks must be complete");
    }
    if ((chatbotConfiguration.listenTimeoutMs == 0U) ||
        (chatbotConfiguration.listenTimeoutMs >
            XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        XHAL_THROW_OUT_OF_RANGE("Local voice-chatbot listen timeout is outside its range");
    }
}

} /* namespace xwalk::agent */
