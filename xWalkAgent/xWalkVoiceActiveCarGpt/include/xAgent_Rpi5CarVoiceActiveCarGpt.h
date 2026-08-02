/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCarGpt.h
 * @brief       Declares the English GPT voice-active-car profile.
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCarGpt
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H
#define XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H

#include "xAgent_Rpi5CarVoiceActiveCar.h"

namespace xwalk::agent
{

class XWalkVoiceActiveCarGpt final
{
    public:
        static constexpr hal::cstring NAME = "Buddy";
        static constexpr hal::cstring SPEECH_LANGUAGE = "en-us";
        static constexpr hal::cstring SPEECH_VOICE = "en_US-ryan-low";
        static constexpr hal::cstring WAKE_WORD = "hey buddy";
        static constexpr hal::cstring ANSWER_ON_WAKE = "Hi there";

        /** @brief Returns English assistant instructions and welcome text. */
        static hal::XWalkVoiceAssistantConfiguration assistantConfiguration();
        /** @brief Returns the ten-centimetre image-enabled car configuration. */
        static XWalkVoiceActiveCarConfiguration carConfiguration() noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_GPT_H */
