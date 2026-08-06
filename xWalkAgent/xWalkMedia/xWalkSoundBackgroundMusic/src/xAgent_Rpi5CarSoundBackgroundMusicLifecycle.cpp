/******************************************************************************
 * @file        xAgent_Rpi5CarSoundBackgroundMusicLifecycle.cpp
 * @brief       Implements audio-example lifecycle and resource validation.
 * @project     xWalk Firmware
 * @module      xWalkSoundBackgroundMusic
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarSoundBackgroundMusic.h"

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarFileFunctions.h"

#include <cmath>

namespace xwalk::agent
{

XWalkSoundBackgroundMusic::XWalkSoundBackgroundMusic(
    hal::XWalkMusic& music, agent::contextpointer context,
    soundbackgroundmusicdelaycallback delayOperation,
    soundbackgroundmusiccontinuecallback continueOperation,
    const agent::filesystempath& soundDirectory,
    const agent::filesystempath& musicDirectory,
    const XWalkSoundBackgroundMusicConfiguration& configuration):
    musicObject(&music), callbackContext(context), delayCallback(delayOperation),
    continueCallback(continueOperation), configurationValue(configuration),
    hornPath(hal::resolveResourcePath(soundDirectory,
        configurationValue.hornFilename)),
    backgroundMusicPath(hal::resolveResourcePath(musicDirectory,
        configurationValue.musicFilename))
{
    validate(configurationValue, delayCallback, continueCallback);
}

XWalkSoundBackgroundMusic::~XWalkSoundBackgroundMusic() noexcept
{
    if (musicPlayingValue)
    {
        try
        {
            musicObject->musicStop();
        }
        catch (...)
        {
            /* Destruction cannot report an audio-backend cleanup failure. */
        }
    }
}

void XWalkSoundBackgroundMusic::validate(
    const XWalkSoundBackgroundMusicConfiguration& configuration,
    soundbackgroundmusicdelaycallback delayOperation,
    soundbackgroundmusiccontinuecallback continueOperation)
{
    const agent::boolean configurationInvalid =
        static_cast<agent::boolean>(
            (delayOperation == nullptr) || (continueOperation == nullptr) ||
        configuration.hornFilename.empty() ||
        configuration.musicFilename.empty() ||
        !std::isfinite(configuration.musicVolumePercent));
    if (configurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Sound-background-music configuration is invalid");
    }
    if ((configuration.musicVolumePercent < 0.0) ||
        (configuration.musicVolumePercent > 100.0) ||
        (configuration.postSoundDelayMs > 1'000U))
    {
        XHAL_THROW_OUT_OF_RANGE(
            "Sound-background-music configuration is out of range");
    }
}

agent::boolean XWalkSoundBackgroundMusic::wait(agent::uint32 durationMs) const
{
    constexpr agent::uint32 cancellationIntervalMs{20U};
    agent::uint32 remainingMs = durationMs;
    while (remainingMs > 0U)
    {
        const agent::boolean operationRequested = continueCallback(callbackContext);
        if (operationRequested == false)
        {
            return false;
        }
        const agent::uint32 sliceMs = (remainingMs < cancellationIntervalMs) ?
            remainingMs : cancellationIntervalMs;
        delayCallback(callbackContext, sliceMs);
        remainingMs -= sliceMs;
    }
    return continueCallback(callbackContext);
}

agent::boolean XWalkSoundBackgroundMusic::start()
{
    if (startedValue)
    {
        return true;
    }
    const agent::boolean hornPathBackgroundMusicPathInvalid =
        static_cast<agent::boolean>(
            !hal::isReadableRegularFile(hornPath) ||
        !hal::isReadableRegularFile(backgroundMusicPath));
    if (hornPathBackgroundMusicPathInvalid)
    {
        return false;
    }
    musicObject->musicSetVolume(configurationValue.musicVolumePercent);
    musicPlayingValue = false;
    startedValue = true;
    return true;
}

void XWalkSoundBackgroundMusic::finish()
{
    if (musicPlayingValue)
    {
        musicObject->musicStop();
    }
    musicPlayingValue = false;
    startedValue = false;
}

agent::boolean XWalkSoundBackgroundMusic::started() const noexcept
{
    return startedValue;
}

} /* namespace xwalk::agent */
