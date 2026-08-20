/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarTypes.h
 * @brief       Declares voice-active-car configuration and callbacks.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_TYPES_H
#define XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_TYPES_H

#include "xHal_Rpi5CarTypes.h"
#include "xHal_Rpi5CarWebSearchTypes.h"

namespace xwalk::agent
{

    using voiceactivecaroutputcallback = void (*)(agent::contextpointer, agent::stringview);
    using voiceactivecarcontinuecallback = agent::boolean (*)(agent::contextpointer);
    using voiceactivecardelaycallback = void (*)(agent::contextpointer, agent::uint32);
    using voiceactivecarclockcallback = agent::uint64 (*)(agent::contextpointer) noexcept;
    using voiceactivecarimagecallback = agent::string (*)(agent::contextpointer);
    using voiceactivecarsearchcallback = hal::XWalkWebSearchResponse (*)(agent::contextpointer, agent::stringview);
    /**
     * @brief Reads one typed prompt for a keyboard-input voice-car profile.
     * @param[in,out] context Non-owning application context that outlives the coordinator.
     * @param[in] prompt Prompt text valid only for the synchronous call.
     * @return Owned user input; an empty value skips the current round.
     */
    using voiceactivecarinputcallback = agent::string (*)(agent::contextpointer, agent::stringview);

    /** @brief Selects microphone recognition or application-owned keyboard input. */
    enum class XWalkVoiceActiveCarInputMode : agent::uint8
    {
        /** @brief Reads each ordinary prompt from the speech-recognition pipeline. */
        Voice,
        /** @brief Reads each ordinary prompt through the injected input callback. */
        Keyboard
    };

    /** @brief Selects the model-response syntax parsed by the coordinator. */
    enum class XWalkVoiceActiveCarResponseFormat : agent::uint8
    {
        /** @brief Parses response text followed by an `ACTIONS:` line. */
        ActionLines,
        /** @brief Parses the upstream GPT-car `actions` and `answer` JSON object. */
        Json
    };

    struct XWalkVoiceActiveCarCallbacks
    {
            voiceactivecaroutputcallback output{nullptr};               /**< Writes one status line. */
            voiceactivecarcontinuecallback shouldContinue{nullptr};     /**< Controls foreground rounds. */
            voiceactivecardelaycallback delay{nullptr};                 /**< Performs bounded LED timing. */
            voiceactivecarclockcallback monotonicMilliseconds{nullptr}; /**< Reads session idle time. */
            voiceactivecarimagecallback captureImage{nullptr};          /**< Returns an optional image path. */
            voiceactivecarinputcallback input{nullptr};                 /**< Reads one keyboard prompt when selected. */
            agent::contextpointer webSearchContext{nullptr};            /**< Optional non-owning search client. */
            voiceactivecarsearchcallback webSearch{nullptr};            /**< Retrieves bounded untrusted references. */
    };

    struct XWalkVoiceActiveCarConfiguration
    {
            agent::float64 tooCloseCm{10.0};        /**< Ultrasonic trigger threshold in centimetres. */
            agent::boolean withImage{true};         /**< Enables image capture for ordinary prompts. */
            agent::uint32 listenTimeoutMs{30'000U}; /**< Bounded recognition interval. */
            agent::boolean wakeEnabled{};           /**< Requires a wake phrase before an ordinary prompt. */
            agent::string wakeWord{};               /**< Case-insensitive phrase that activates one ordinary round. */
            agent::string answerOnWake{};           /**< Optional response spoken after wake detection. */
            XWalkVoiceActiveCarInputMode inputMode{XWalkVoiceActiveCarInputMode::Voice};
            XWalkVoiceActiveCarResponseFormat responseFormat{XWalkVoiceActiveCarResponseFormat::ActionLines};
            agent::boolean sensorEnabled{true};                  /**< Enables the ultrasonic autonomous prompt. */
            agent::boolean continuousConversationEnabled{false}; /**< Keeps bounded follow-up rounds awake. */
            agent::uint32 conversationIdleTimeoutMs{30'000U};    /**< Active-session idle deadline. */
            agent::uint32 conversationMaximumRounds{10U};        /**< Successful rounds before wake reset. */
            agent::uint32 conversationMaximumMisses{3U};         /**< Empty recognitions before wake reset. */
            agent::stringvector sleepPhrases{};                  /**< Trimmed case-insensitive exit phrases. */
            agent::string sleepAcknowledgement{};                /**< Optional speech after an explicit exit. */
            agent::boolean webSearchEnabled{false};              /**< Enables bounded current-information retrieval. */
    };

    struct XWalkVoiceActiveCarResponse
    {
            agent::string text{};          /**< Spoken response preceding the action delimiter. */
            agent::stringvector actions{}; /**< Parsed action names, or one `stop` fallback. */
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_TYPES_H */
