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
     * @param[in] parameters Non-owning application callback, configuration,
     * board-control, and PiCar-X dependencies valid through this synchronous
     * composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback`, `parameters.config`,
     * `parameters.board`, and `parameters.picarx` are non-null.
     */
    agent::int32 XWalkBootRpi::runStorytellingRobot(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        hal::XWalkBoardControl& boardControl = *parameters.board;
        XWalkPicarx& picarx = *parameters.picarx;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .061, "Boot composing storytelling-robot services");
        hal::XWalkTextToSpeechPiper piper(config.get("voice_piper_executable", "piper"),
                                          config.get("voice_piper_playback_executable", "aplay"),
                                          config.get("voice_piper_model", "en_US-amy-low"));
        hal::XWalkTextToSpeech textToSpeech(boardControl, &piper, piper.callback());
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.textToSpeech = &textToSpeech;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
