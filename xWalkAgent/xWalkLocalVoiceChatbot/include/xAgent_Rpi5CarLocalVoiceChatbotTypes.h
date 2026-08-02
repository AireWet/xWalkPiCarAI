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
inline constexpr hal::cstring XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_INSTRUCTIONS =
    "You are a helpful assistant. Answer directly in plain English. "
    "Do NOT include any hidden thinking, analysis, or tags like <think>.";

/** @brief Default spoken welcome preserved from the Python example. */
inline constexpr hal::cstring XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_WELCOME =
    "Hello! I'm your voice chatbot. Speak when you're ready.";

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Writes one chatbot status or conversation line. */
using localvoicechatbotoutputcallback = void (*)(hal::contextpointer context,
    hal::stringview line);

/** @brief Reports whether another microphone round may begin. */
using localvoicechatbotcontinuecallback = hal::boolean (*)(hal::contextpointer context);

/** @brief Suspends the chatbot between bounded operations. */
using localvoicechatbotdelaycallback = void (*)(hal::contextpointer context,
    hal::uint32 durationMs);

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
    hal::string silenceMessage{"[INFO] Nothing recognized. Try again."};
    hal::string emptyResponse{"Sorry, I didn't catch that."};
    hal::string goodbye{"Goodbye!"};
    hal::uint32 listenTimeoutMs{10'000U};
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_LOCAL_VOICE_CHATBOT_TYPES_H */
