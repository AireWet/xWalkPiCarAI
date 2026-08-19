/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiSound.cpp
 * @brief       Composes Raspberry Pi standalone sound control.
 * @details     Publishes configured ALSA Music together with PiCar-X.
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
#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusicSndFileDecoder.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured standalone sound control.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runSound(agent::contextpointer context,
                                        bootapplicationcallback callback,
                                        hal::XWalkConfigStore& config,
                                        XWalkPicarx& picarx)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .059, "Boot composing sound services");
        hal::XWalkAudioAlsa audioBackend(config.get("voice_playback_device", "default"),
                                         config.get("voice_mixer_device", "default"),
                                         config.get("voice_mixer_element", "PCM"));
        hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr, hal::XWalkMusicSndFileDecoder::operations());
        hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.music = &music;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
