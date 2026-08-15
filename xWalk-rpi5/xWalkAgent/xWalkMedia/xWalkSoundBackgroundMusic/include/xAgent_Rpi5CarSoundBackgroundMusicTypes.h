/******************************************************************************
 * @file        xAgent_Rpi5CarSoundBackgroundMusicTypes.h
 * @brief       Declares interactive sound-and-music state and configuration.
 * @project     xWalk Firmware
 * @module      xWalkSoundBackgroundMusic
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_SOUND_BACKGROUND_MUSIC_TYPES_H
#define XAGENT_RPI5CAR_SOUND_BACKGROUND_MUSIC_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

    using soundbackgroundmusicdelaycallback = void (*)(agent::contextpointer, agent::uint32);
    using soundbackgroundmusiccontinuecallback = agent::boolean (*)(agent::contextpointer);

    /** @brief Identifies the result of one interactive key. */
    enum class XWalkSoundBackgroundMusicEvent : agent::uint8
    {
        Ignored = 0U,
        MusicStarted,
        MusicStopped,
        SoundPlayed,
        BackgroundSoundStarted,
        Cancelled
    };

    /** @brief Stores source-compatible resource, volume, and timing settings. */
    struct XWalkSoundBackgroundMusicConfiguration
    {
            agent::string hornFilename{"car-double-horn.wav"};
            agent::string musicFilename{"slow-trail-Ahjay_Stelino.mp3"};
            agent::float64 musicVolumePercent{20.0};
            agent::uint32 postSoundDelayMs{50U};
    };

    /** @brief Reports one key outcome and retained background-music state. */
    struct XWalkSoundBackgroundMusicResult
    {
            XWalkSoundBackgroundMusicEvent event{XWalkSoundBackgroundMusicEvent::Ignored};
            agent::boolean musicPlaying{};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SOUND_BACKGROUND_MUSIC_TYPES_H */
