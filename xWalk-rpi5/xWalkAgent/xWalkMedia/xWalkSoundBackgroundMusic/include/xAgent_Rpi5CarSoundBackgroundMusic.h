/******************************************************************************
 * @file        xAgent_Rpi5CarSoundBackgroundMusic.h
 * @brief       Declares interactive sound effects and background music.
 * @project     xWalk Firmware
 * @module      xWalkSoundBackgroundMusic
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_SOUND_BACKGROUND_MUSIC_H
#define XAGENT_RPI5CAR_SOUND_BACKGROUND_MUSIC_H

#include "xAgent_Rpi5CarSoundBackgroundMusicTypes.h"
#include "xHal_Rpi5CarMusic.h"

namespace xwalk::agent
{

    /** @brief Ports `13.sound_background_music.py` through caller-owned audio. */
    class XWalkSoundBackgroundMusic final
    {
        private:
            hal::XWalkMusic* musicObject{nullptr};
            agent::contextpointer callbackContext{nullptr};
            soundbackgroundmusicdelaycallback delayCallback{nullptr};
            soundbackgroundmusiccontinuecallback continueCallback{nullptr};
            XWalkSoundBackgroundMusicConfiguration configurationValue{};
            agent::filesystempath hornPath{};
            agent::filesystempath backgroundMusicPath{};
            agent::boolean musicPlayingValue{};
            agent::boolean startedValue{};

        protected:
            static void validate(const XWalkSoundBackgroundMusicConfiguration& configuration,
                                 soundbackgroundmusicdelaycallback delayOperation,
                                 soundbackgroundmusiccontinuecallback continueOperation);
            agent::boolean wait(agent::uint32 durationMs) const;

        public:
            XWalkSoundBackgroundMusic(hal::XWalkMusic& music,
                                      agent::contextpointer context,
                                      soundbackgroundmusicdelaycallback delayOperation,
                                      soundbackgroundmusiccontinuecallback continueOperation,
                                      const agent::filesystempath& soundDirectory,
                                      const agent::filesystempath& musicDirectory,
                                      const XWalkSoundBackgroundMusicConfiguration& configuration = {});
            ~XWalkSoundBackgroundMusic() noexcept;

            XWalkSoundBackgroundMusic(const XWalkSoundBackgroundMusic&) = delete;
            XWalkSoundBackgroundMusic(XWalkSoundBackgroundMusic&&) = delete;
            XWalkSoundBackgroundMusic& operator=(const XWalkSoundBackgroundMusic&) = delete;
            XWalkSoundBackgroundMusic& operator=(XWalkSoundBackgroundMusic&&) = delete;

            agent::boolean start();
            XWalkSoundBackgroundMusicResult handleKey(const agent::string& key);
            void finish();
            agent::boolean started() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SOUND_BACKGROUND_MUSIC_H */
