/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiSoundBackgroundMusic.cpp
 * @brief       Composes Raspberry Pi sound and background music.
 * @details     Binds configured ALSA output and packaged media directories.
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

#include "xAgent_Rpi5CarSoundBackgroundMusic.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarMusicAlsa.h"
#include "xHal_Rpi5CarMusicSndFileDecoder.h"

namespace xwalk::agent
{

    /**
     * @brief Runs configured sound and background music.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator exposed to the app.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runSoundBackgroundMusic(agent::contextpointer context,
                                                       bootapplicationcallback callback,
                                                       hal::XWalkConfigStore& config,
                                                       XWalkPicarx& picarx)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .058, "Boot composing sound-background-music services");
        hal::XWalkAudioAlsa audioBackend(config.get("voice_playback_device", "default"),
                                         config.get("voice_mixer_device", "default"),
                                         config.get("voice_mixer_element", "PCM"));
        hal::XWalkMusicAlsa musicBackend(audioBackend, nullptr, hal::XWalkMusicSndFileDecoder::operations());
        hal::XWalkMusic music(&musicBackend, musicBackend.callbacks());
        XWalkSoundBackgroundMusic soundBackgroundMusic(
            music,
            nullptr,
            &delayMilliseconds,
            &continueComputerVision,
            config.get("resource_sound_directory", "/usr/share/xwalk/sounds"),
            config.get("resource_music_directory", "/usr/share/xwalk/music"));
        XWalkBootServices services{};
        services.picarx = &picarx;
        services.music = &music;
        services.soundBackgroundMusic = &soundBackgroundMusic;
        return callback(context, services);
    }

} /* namespace xwalk::agent */
