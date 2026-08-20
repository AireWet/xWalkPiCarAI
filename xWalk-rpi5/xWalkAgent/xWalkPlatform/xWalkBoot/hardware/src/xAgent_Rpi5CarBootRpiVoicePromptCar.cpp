/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVoicePromptCar.cpp
 * @brief       Composes the Raspberry Pi spoken movement demonstration.
 * @details     Binds configured Espeak and ALSA playback providers to PiCar-X.
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
#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarTextToSpeechEspeak.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured spoken movement prompts.
     * @param[in] parameters Non-owning application callback, configuration,
     * board-control, and PiCar-X dependencies valid through this synchronous
     * composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback`, `parameters.config`,
     * `parameters.board`, and `parameters.picarx` are non-null.
     */
    agent::int32 XWalkBootRpi::runVoicePromptCar(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        hal::XWalkBoardControl& boardControl = *parameters.board;
        XWalkPicarx& picarx = *parameters.picarx;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .071, "Boot composing voice-prompt-car services");
        hal::XWalkAudioAlsa audioBackend(config.get("voice_playback_device", "default"),
                                         config.get("voice_mixer_device", "default"),
                                         config.get("voice_mixer_element", "PCM"));
        hal::XWalkTextToSpeechEspeak espeak(config.get("voice_espeak_executable", "espeak-ng"),
                                            config.get("voice_espeak_voice", "en"));
        hal::XWalkTextToSpeechAlsa speechBackend(audioBackend, &espeak, espeak.operations());
        hal::XWalkTextToSpeech textToSpeech(boardControl, &speechBackend, speechBackend.callback());
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.textToSpeech = &textToSpeech;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
