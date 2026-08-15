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

#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarTextToSpeechEspeak.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured spoken movement prompts.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] boardControl Caller-owned board controller.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runVoicePromptCar(agent::contextpointer context,
                                                 bootapplicationcallback callback,
                                                 hal::XWalkConfigStore& config,
                                                 hal::XWalkBoardControl& boardControl,
                                                 XWalkPicarx& picarx)
    {
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
        return callback(context, services);
    }

} /* namespace xwalk::agent */
