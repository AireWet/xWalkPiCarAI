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

#include "xHal_Rpi5CarSpeechToTextTypes.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    XWalkLocalVoiceChatbot::XWalkLocalVoiceChatbot(hal::XWalkVoiceAssistant& assistant,
                                                   agent::contextpointer context,
                                                   const XWalkLocalVoiceChatbotCallbacks& backendCallbacks,
                                                   const XWalkLocalVoiceChatbotConfiguration& chatbotConfiguration)
        : assistantObject(&assistant), callbackContext(context), callbacks(backendCallbacks),
          configuration(chatbotConfiguration)
    {
        validate(callbacks, configuration);
    }

    XWalkLocalVoiceChatbot::~XWalkLocalVoiceChatbot()
    {
        assistantObject->stop();
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Runs source-compatible microphone, model, and speech rounds.
     * @return Zero after cancellation and the complete farewell sequence.
     * @throws std::exception Propagates callback or voice-pipeline failures. The
     * scope-bound destructor stops the assistant while the stack unwinds.
     */
    agent::int32 XWalkLocalVoiceChatbot::run()
    {
        assistantObject->start();
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .038, "Local voice-chatbot assistant started");
        callbacks.output(callbackContext, configuration.welcome);
        const agent::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const agent::boolean operationMayContinue =
                static_cast<agent::boolean>(callbacks.shouldContinue(callbackContext));
            if (operationMayContinue == false)
            {
                break;
            }
            callbacks.output(callbackContext, configuration.listeningMessage);
            const agent::string recognizedText = assistantObject->listen(configuration.listenTimeoutMs);
            const agent::boolean recognizedTextEmpty = static_cast<agent::boolean>(recognizedText.empty());
            if (recognizedTextEmpty)
            {
                callbacks.output(callbackContext, configuration.silenceMessage);
                callbacks.delay(callbackContext, 100U);
                continue;
            }

            callbacks.output(callbackContext, agent::string("[YOU] ") + recognizedText);
            const agent::string response = assistantObject->think(recognizedText);
            callbacks.output(callbackContext, response);
            const agent::string cleanResponse = stripThinking(response);
            assistantObject->say(cleanResponse.empty() ? configuration.emptyResponse : cleanResponse);
            callbacks.delay(callbackContext, 50U);
        }

        callbacks.output(callbackContext, configuration.stoppingMessage);
        finish();
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

    /**
     * @brief Validates callbacks, messages, and the recognition timeout.
     * @param[in] backendCallbacks Complete synchronous callback table.
     * @param[in] chatbotConfiguration Owned source-compatible runtime settings.
     * @throws std::invalid_argument If a callback or message is missing.
     * @throws std::out_of_range If the recognition timeout is invalid.
     */
    void XWalkLocalVoiceChatbot::validate(const XWalkLocalVoiceChatbotCallbacks& backendCallbacks,
                                          const XWalkLocalVoiceChatbotConfiguration& chatbotConfiguration)
    {
        if ((backendCallbacks.output == nullptr) || (backendCallbacks.shouldContinue == nullptr) ||
            (backendCallbacks.delay == nullptr))
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Local voice-chatbot callbacks must be complete");
        }
        if ((chatbotConfiguration.listenTimeoutMs == 0U) ||
            (chatbotConfiguration.listenTimeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
        {
            XWALK_RPIAGENT_ERROR(XWALK_RANGE, "Local voice-chatbot listen timeout is outside its range");
        }
        const agent::boolean chatbotConfigurationInvalid = static_cast<agent::boolean>(
            chatbotConfiguration.welcome.empty() || chatbotConfiguration.listeningMessage.empty() ||
            chatbotConfiguration.silenceMessage.empty() || chatbotConfiguration.emptyResponse.empty() ||
            chatbotConfiguration.stoppingMessage.empty() || chatbotConfiguration.goodbye.empty() ||
            chatbotConfiguration.bye.empty());
        if (chatbotConfigurationInvalid)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Local voice-chatbot messages must not be empty");
        }
    }

    /**
     * @brief Speaks the farewell, stops recognition, and reports completion.
     * @throws std::exception Propagates a speech or output backend failure. The
     * scope-bound destructor stops the assistant if farewell speech fails.
     */
    void XWalkLocalVoiceChatbot::finish()
    {
        assistantObject->say(configuration.goodbye);
        assistantObject->stop();
        callbacks.output(callbackContext, configuration.bye);
    }

} /* namespace xwalk::agent */
