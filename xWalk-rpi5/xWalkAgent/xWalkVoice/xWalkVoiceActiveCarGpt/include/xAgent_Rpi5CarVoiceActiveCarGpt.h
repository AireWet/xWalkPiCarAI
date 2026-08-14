/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGpt.h
 * @brief       Declares the example-21 English GPT voice-car profile.
 *
 * @details
 * Defines the source-compatible Buddy identity, OpenAI model, Piper voice,
 * wake behavior, proximity threshold, camera setting, and assistant prompt.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCarGpt
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

#ifndef XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H
#define XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCar.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkVoiceActiveCarGpt
 * @brief Supplies immutable source defaults for example 21.
 *
 * @details
 * Owns no hardware, credential, provider, or mutable conversation state. The
 * Raspberry Pi composition root consumes these values and keeps secrets in the
 * process environment.
 */
class XWalkVoiceActiveCarGpt final
{
    public:
        /** @brief Source robot name. */
        static constexpr agent::cstring NAME = "Buddy";
        /** @brief Source speech-recognition language. */
        static constexpr agent::cstring SPEECH_LANGUAGE = "en-us";
        /** @brief Source Piper voice model. */
        static constexpr agent::cstring SPEECH_VOICE = "en_US-ryan-low";
        /** @brief Source OpenAI language-model name. */
        static constexpr agent::cstring MODEL_NAME = "gpt-4o-mini";
        /** @brief OpenAI-compatible endpoint used when deployment does not override it. */
        static constexpr agent::cstring MODEL_ENDPOINT =
            "https://api.openai.com/v1/chat/completions";
        /** @brief Environment variable that exclusively supplies the credential. */
        static constexpr agent::cstring API_KEY_ENVIRONMENT = "OPENAI_API_KEY";
        /** @brief Source case-insensitive wake phrase. */
        static constexpr agent::cstring WAKE_WORD = "hey buddy";
        /** @brief Source response spoken when the wake phrase is detected. */
        static constexpr agent::cstring ANSWER_ON_WAKE = "Hi there";

        /**
         * @brief Returns the complete source-compatible instructions and welcome text.
         * @return Owned assistant configuration for one caller-created coordinator.
         */
        static hal::XWalkVoiceAssistantConfiguration assistantConfiguration();

        /**
         * @brief Returns source-compatible sensing, image, recognition, and wake settings.
         * @return Ten-centimetre, image-enabled, English Buddy configuration.
         */
        static XWalkVoiceActiveCarConfiguration carConfiguration();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H */
