/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiStorytellingRobot.cpp
 * @brief       Composes Raspberry Pi storytelling-robot speech.
 * @details     Binds the configured Piper process provider to PiCar-X.
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

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarTextToSpeechPiper.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured storytelling speech.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] boardControl Caller-owned board controller.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runStorytellingRobot(agent::contextpointer context,
                                                    bootapplicationcallback callback,
                                                    hal::XWalkConfigStore& config,
                                                    hal::XWalkBoardControl& boardControl,
                                                    XWalkPicarx& picarx)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .061, "Boot composing storytelling-robot services");
        hal::XWalkTextToSpeechPiper piper(config.get("voice_piper_executable", "piper"),
                                          config.get("voice_piper_playback_executable", "aplay"),
                                          config.get("voice_piper_model", "en_US-amy-low"));
        hal::XWalkTextToSpeech textToSpeech(boardControl, &piper, piper.callback());
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.textToSpeech = &textToSpeech;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
