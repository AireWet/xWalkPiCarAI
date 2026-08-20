/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGpt.h
 * @brief       Declares the Gemini-backed Jarvis voice-car profile.
 *
 * @details
 * Defines the Jarvis identity, Gemini model, Piper voice, wake behavior,
 * proximity threshold, text-only policy, session limits, and filtered prompt.
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
            static constexpr agent::cstring MODEL_NAME = "gemini-3.6-flash";
            /** @brief Gemini OpenAI-compatible endpoint used when deployment does not override it. */
            static constexpr agent::cstring MODEL_ENDPOINT =
                "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions";
            /** @brief Environment variable that exclusively supplies the Gemini credential. */
            static constexpr agent::cstring API_KEY_ENVIRONMENT = "GEMINI_API_KEY";
            /** @brief Maximum Gemini tokens retained for concise spoken output. */
            static constexpr agent::uint32 MAXIMUM_OUTPUT_TOKENS = 256U;
            /** @brief Permanent text-only image policy for Jarvis. */
            static constexpr agent::boolean WITH_IMAGE = false;
            /** @brief Enables bounded wake-free follow-up requests by default. */
            static constexpr agent::boolean CONTINUOUS_CONVERSATION = true;
            /** @brief Active conversation idle timeout in milliseconds. */
            static constexpr agent::uint32 CONVERSATION_IDLE_TIMEOUT_MS = 30'000U;
            /** @brief Maximum successful requests in one active conversation. */
            static constexpr agent::uint32 CONVERSATION_MAXIMUM_ROUNDS = 10U;
            /** @brief Maximum consecutive recognition misses in one conversation. */
            static constexpr agent::uint32 CONVERSATION_MAXIMUM_MISSES = 3U;
            /** @brief Comma-separated default phrases that return Jarvis to wake mode. */
            static constexpr agent::cstring SLEEP_PHRASES = "goodbye jarvis,go to sleep,stop listening";
            /** @brief Default acknowledgement spoken after an explicit sleep phrase. */
            static constexpr agent::cstring SLEEP_ACKNOWLEDGEMENT =
                "Going to sleep. Say hey Jarvis when you need me, Joxy.";
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
             * @brief Returns Jarvis sensing, text-only recognition, and wake settings.
             * @return Text-only, continuous, bounded English Jarvis configuration.
             */
            static XWalkVoiceActiveCarConfiguration carConfiguration();
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H */
