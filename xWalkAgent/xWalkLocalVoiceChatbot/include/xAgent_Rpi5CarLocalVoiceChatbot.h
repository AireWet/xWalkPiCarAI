/******************************************************************************
 * @file        xAgent_Rpi5CarLocalVoiceChatbot.h
 * @brief       Declares the foreground local voice-chatbot coordinator.
 *
 * @details
 * Ports the continuous behavior of `19.local_voice_chatbot.py` onto the
 * caller-owned xWalk voice-assistant pipeline.
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

#ifndef XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_H
#define XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarLocalVoiceChatbotTypes.h"
#include "xHal_Rpi5CarVoiceAssistant.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Coordinates repeated local voice-assistant rounds in the foreground. */
class XWalkLocalVoiceChatbot final
{
    private:
        hal::XWalkVoiceAssistant* assistantObject{nullptr};
        hal::contextpointer callbackContext{nullptr};
        XWalkLocalVoiceChatbotCallbacks callbacks{};
        XWalkLocalVoiceChatbotConfiguration configuration{};

    protected:
        /** @brief Validates callbacks and bounded runtime configuration. */
        static void validate(const XWalkLocalVoiceChatbotCallbacks& backendCallbacks,
            const XWalkLocalVoiceChatbotConfiguration& chatbotConfiguration);

    public:
        /**
         * @brief Binds one caller-owned voice-assistant pipeline.
         * @param[in] assistant Voice assistant that must outlive this object.
         * @param[in,out] context Nullable context that outlives callback use.
         * @param[in] backendCallbacks Complete synchronous application callbacks.
         * @param[in] chatbotConfiguration Owned runtime messages and listen timeout.
         */
        XWalkLocalVoiceChatbot(hal::XWalkVoiceAssistant& assistant,
            hal::contextpointer context,
            const XWalkLocalVoiceChatbotCallbacks& backendCallbacks,
            const XWalkLocalVoiceChatbotConfiguration& chatbotConfiguration = {});

        /** @brief Releases no caller-owned voice or callback resources. */
        ~XWalkLocalVoiceChatbot() = default;

        XWalkLocalVoiceChatbot(XWalkLocalVoiceChatbot&&) = delete;
        XWalkLocalVoiceChatbot(const XWalkLocalVoiceChatbot&) = delete;
        XWalkLocalVoiceChatbot& operator=(XWalkLocalVoiceChatbot&&) = delete;
        XWalkLocalVoiceChatbot& operator=(const XWalkLocalVoiceChatbot&) = delete;

        /**
         * @brief Runs microphone, model, and speech rounds until cancellation.
         * @return Zero after the normal goodbye sequence completes.
         */
        hal::int32 run();

        /** @brief Requests recognition shutdown and stops the assistant. */
        void stop();

        /**
         * @brief Removes hidden-thinking sections and markers from one response.
         * @param[in] response Raw final model response.
         * @return Trimmed response suitable for speech output.
         */
        static hal::string stripThinking(hal::stringview response);
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_H */
