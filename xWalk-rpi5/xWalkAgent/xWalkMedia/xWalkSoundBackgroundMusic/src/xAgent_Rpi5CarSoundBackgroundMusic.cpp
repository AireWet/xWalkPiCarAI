/******************************************************************************
 * @file        xAgent_Rpi5CarSoundBackgroundMusic.cpp
 * @brief       Implements interactive sound and background-music keys.
 * @project     xWalk Firmware
 * @module      xWalkSoundBackgroundMusic
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarSoundBackgroundMusic.h"

#include "xHal_Rpi5CarTrace.h"
#include <cctype>

namespace xwalk::agent
{

    XWalkSoundBackgroundMusicResult XWalkSoundBackgroundMusic::handleKey(const agent::string& key)
    {
        if (!startedValue)
        {
            XWALK_RPIAGENT_ERROR(XWALK_LOGIC, "Sound background music must be started before handling keys");
        }
        const agent::boolean operationRequested = continueCallback(callbackContext);
        if (operationRequested == false)
        {
            if (musicPlayingValue)
            {
                musicObject->musicStop();
                musicPlayingValue = false;
            }
            return {XWalkSoundBackgroundMusicEvent::Cancelled, false};
        }

        const char command =
            (key.size() == 1U) ? static_cast<char>(std::tolower(static_cast<unsigned char>(key[0]))) : '\0';
        XWalkSoundBackgroundMusicEvent event = XWalkSoundBackgroundMusicEvent::Ignored;
        if (command == 'q')
        {
            musicPlayingValue = !musicPlayingValue;
            if (musicPlayingValue)
            {
                musicObject->musicPlay(backgroundMusicPath.string());
                event = XWalkSoundBackgroundMusicEvent::MusicStarted;
            }
            else
            {
                musicObject->musicStop();
                event = XWalkSoundBackgroundMusicEvent::MusicStopped;
            }
        }
        else if (command == ' ')
        {
            musicObject->soundPlay(hornPath.string());
            event = XWalkSoundBackgroundMusicEvent::SoundPlayed;
        }
        else if (command == 'c')
        {
            musicObject->soundPlayBackground(hornPath.string());
            event = XWalkSoundBackgroundMusicEvent::BackgroundSoundStarted;
        }

        if ((command == ' ') || (command == 'c'))
        {
            const agent::boolean delayCompleted = wait(configurationValue.postSoundDelayMs);
            if (delayCompleted == false)
            {
                if (musicPlayingValue)
                {
                    musicObject->musicStop();
                    musicPlayingValue = false;
                }
                event = XWalkSoundBackgroundMusicEvent::Cancelled;
            }
        }
        return {event, musicPlayingValue};
    }

} /* namespace xwalk::agent */
