/******************************************************************************
 * @file        xAgent_Rpi5CarLocalVoiceChatbotTypes.h
 * @brief       Declares local voice-chatbot configuration and callback types.
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

#ifndef XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_TYPES_H
#define XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

/******************************************************************************
 * Constant declarations
 ******************************************************************************/

/** @brief Default system instructions preserved from the Python example. */
inline constexpr agent::cstring XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_INSTRUCTIONS =
    "You are a helpful assistant. Answer directly in plain English. "
    "Do NOT include any hidden thinking, analysis, or tags like <think>.";

/** @brief Default spoken welcome preserved from the Python example. */
inline constexpr agent::cstring XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME =
    "Hello! I'm your voice chatbot. Speak when you're ready.";

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Writes one chatbot status or conversation line. */
using localvoicechatbotoutputcallback = void (*)(agent::contextpointer context,
    agent::stringview line);

/** @brief Reports whether another microphone round may begin. */
using localvoicechatbotcontinuecallback = agent::boolean (*)(agent::contextpointer context);

/** @brief Suspends the chatbot between bounded operations. */
using localvoicechatbotdelaycallback = void (*)(agent::contextpointer context,
    agent::uint32 durationMs);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Groups the application operations required by the chatbot loop. */
struct XWalkLocalVoiceChatbotCallbacks
{
    localvoicechatbotoutputcallback output{nullptr}; /**< Writes one complete output line. */
    localvoicechatbotcontinuecallback shouldContinue{nullptr}; /**< Controls foreground looping. */
    localvoicechatbotdelaycallback delay{nullptr}; /**< Performs one bounded delay. */
};

/** @brief Stores owned runtime text and timing configuration. */
struct XWalkLocalVoiceChatbotConfiguration
{
    agent::string welcome{XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME}; /**< Printed welcome. */
    agent::string listeningMessage{"🎤 Listening... (Press Ctrl+C to stop)"}; /**< Listening status. */
    agent::string silenceMessage{"[INFO] Nothing recognized. Try again."}; /**< Empty-input status. */
    agent::string emptyResponse{"Sorry, I didn't catch that."}; /**< Empty-model fallback. */
    agent::string stoppingMessage{"[INFO] Stopping..."}; /**< Cancellation status. */
    agent::string goodbye{"Goodbye!"}; /**< Spoken farewell. */
    agent::string bye{"Bye."}; /**< Final output status. */
    agent::uint32 listenTimeoutMs{10'000U}; /**< Per-round recognition timeout. */
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_TYPES_H */
