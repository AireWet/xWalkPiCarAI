/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVoiceActiveCarGpt.cpp
 * @brief       Selects the Raspberry Pi provider-neutral Jarvis voice-car profile.
 * @details     Delegates to the shared configured voice-active composition.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"
#include "xHal_Rpi5CarTrace.h"

namespace xwalk::agent
{

    /**
     * @brief Runs the configured provider-neutral Jarvis voice-active-car profile.
     * @param[in] parameters Non-owning application, vehicle, configuration, and
     * GPIO dependencies valid through the synchronous dispatch.
     * @return Status returned by the configured application callback.
     * @pre Every required pointer in `parameters` is non-null.
     */
    agent::int32 XWalkBootRpi::runVoiceActiveCarGpt(const xAgentContext& parameters)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .068, "Boot selecting Jarvis voice-active-car composition");
        return runVoiceActiveMode(XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ, parameters);
    }

} /* namespace xwalk::agent */
