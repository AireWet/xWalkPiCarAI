/******************************************************************************
 * @file        xHal_Rpi5CarMusicTestSupport.cpp
 * @brief       Implements reusable xWalkMusic host-test support.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMusicTestSupport.h"
namespace xwalk::hal::test::music
{
    void enableOutput(contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->enableCount;
    }
    void playSound(contextpointer context, stringview filename, optionalfloat64 normalizedVolume)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.soundCount;
        backend.filename = string(filename);
        backend.soundVolume = normalizedVolume;
    }
    void playSoundBackground(contextpointer context, stringview filename, optionalfloat64 normalizedVolume)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.backgroundSoundCount;
        backend.filename = string(filename);
        backend.soundVolume = normalizedVolume;
    }
    void playMusic(contextpointer context, stringview filename, int32 loops, float64 startSeconds)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.musicCount;
        backend.filename = string(filename);
        backend.loops = loops;
        backend.startSeconds = startSeconds;
    }
    void setMusicVolume(contextpointer context, float64 normalizedVolume)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.volumeCount;
        backend.musicVolume = normalizedVolume;
    }
    void stopMusic(contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->stopCount;
    }
    void pauseMusic(contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->pauseCount;
    }
    void resumeMusic(contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->resumeCount;
    }
    float64 getSoundLength(contextpointer context, stringview filename)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.filename = string(filename);
        return backend.soundDurationSeconds;
    }
    void playTone(contextpointer context, const bytevector& pcmData, uint32 sampleRateHz, uint8 channelCount)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.toneCount;
        backend.pcmData = pcmData;
        backend.sampleRateHz = sampleRateHz;
        backend.channelCount = channelCount;
    }
    XWalkMusicCallbacks musicCallbacks()
    {
        return {&enableOutput,
                &playSound,
                &playSoundBackground,
                &playMusic,
                &setMusicVolume,
                &stopMusic,
                &pauseMusic,
                &resumeMusic,
                &getSoundLength,
                &playTone};
    }
} /* namespace xwalk::hal::test::music */
