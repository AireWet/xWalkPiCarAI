/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGpt.h
 * @brief       Declares the Gemini-backed Jarvis voice-car profile.
 *
 * @details
 * Defines the Jarvis identity, Gemini model, Piper voice, wake behavior,
 * proximity threshold, camera setting, and filtered assistant action prompt.
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
     * @brief Supplies immutable Gemini-backed Jarvis profile defaults.
     *
     * @details
     * Owns no hardware, credential, provider, or mutable conversation state. The
     * Raspberry Pi composition root consumes these values and keeps secrets in the
     * process environment.
     */
    class XWalkVoiceActiveCarGpt final
    {
        public:
            /** @brief Configured robot-assistant name. */
            static constexpr agent::cstring NAME = "Jarvis";
            /** @brief Source speech-recognition language. */
            static constexpr agent::cstring SPEECH_LANGUAGE = "en-us";
            /** @brief British male Piper model path used for spoken Jarvis replies. */
            static constexpr agent::cstring SPEECH_VOICE = "/usr/share/xwalk/models/piper/en_GB-alan-medium.onnx";
            /** @brief Stable Gemini language-model name used by the profile default. */
            static constexpr agent::cstring MODEL_NAME = "gemini-3.7-flash";
            /** @brief Gemini OpenAI-compatible endpoint used when deployment does not override it. */
            static constexpr agent::cstring MODEL_ENDPOINT =
                "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions";
            /** @brief Environment variable that exclusively supplies the Gemini credential. */
            static constexpr agent::cstring API_KEY_ENVIRONMENT = "GEMINI_API_KEY";
            /** @brief Configured case-insensitive wake phrase. */
            static constexpr agent::cstring WAKE_WORD = "hey jarvis";
            /** @brief Response spoken when the Jarvis wake phrase is detected. */
            static constexpr agent::cstring ANSWER_ON_WAKE = "Systems online. Ready when you are, Joxy.";

            /**
             * @brief Returns filtered Jarvis instructions and welcome text.
             * @return Owned assistant configuration for one caller-created coordinator.
             */
            static hal::XWalkVoiceAssistantConfiguration assistantConfiguration();

            /**
             * @brief Returns Jarvis sensing, image, recognition, and wake settings.
             * @return Ten-centimetre, image-enabled, English Jarvis configuration.
             */
            static XWalkVoiceActiveCarConfiguration carConfiguration();
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H */
