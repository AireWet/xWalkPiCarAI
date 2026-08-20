/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiSelfDrive.cpp
 * @brief       Composes Raspberry Pi self-drive actions.
 * @details     Binds configured ALSA output and sound resources to SelfDrive.
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

#include "xAgent_Rpi5CarSelfDrive.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusicSndFileDecoder.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured SelfDrive actions.
     * @param[in] parameters Non-owning application callback, configuration,
     * and PiCar-X dependencies valid through this synchronous composition.
     * @return Status returned by the configured callback.
     * @pre `parameters.callback`, `parameters.config`, and
     * `parameters.picarx` are non-null.
     */
    agent::int32 XWalkBootRpi::runSelfDrive(const xAgentContext& parameters)
    {
        hal::XWalkConfigStore& config = *parameters.config;
        XWalkPicarx& picarx = *parameters.picarx;
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .056, "Boot composing self-drive services");
        hal::XWalkAudioAlsa audioBackend(config.get("voice_playback_device", "default"),
                                         config.get("voice_mixer_device", "default"),
                                         config.get("voice_mixer_element", "PCM"));
        hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr, hal::XWalkMusicSndFileDecoder::operations());
        hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
        XWalkSelfDrive selfDrive(picarx,
                                 music,
                                 nullptr,
                                 &selfDriveDelayMilliseconds,
                                 nullptr,
                                 config.get("resource_sound_directory", "/usr/share/xwalk/sounds"),
                                 config.get("resource_music_directory", "/usr/share/xwalk/music"));
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.selfDrive = &selfDrive;
        services.music = &music;
        return parameters.callback(parameters.appContext, services);
    }

} /* namespace xwalk::agent */
