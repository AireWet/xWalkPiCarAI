/******************************************************************************
 * @file        xHal_Rpi5CarMusicHostStub.cpp
 * @brief       Implements the silent in-memory Music host stub.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarMusicHostStub.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::sim
{
    void XWalkMusicHostStub::enableOutput(contextpointer context)
    {
        ++static_cast<XWalkMusicHostStub*>(context)->enableCountValue;
    }
    void XWalkMusicHostStub::playSound(contextpointer context, stringview filename, optionalfloat64 normalizedVolume)
    {
        static_cast<void>(filename);
        static_cast<void>(normalizedVolume);
        ++static_cast<XWalkMusicHostStub*>(context)->playbackCountValue;
    }
    void XWalkMusicHostStub::playSoundBackground(contextpointer context,
                                                 stringview filename,
                                                 optionalfloat64 normalizedVolume)
    {
        playSound(context, filename, normalizedVolume);
    }
    void XWalkMusicHostStub::playMusic(contextpointer context, stringview filename, int32 loops, float64 startSeconds)
    {
        static_cast<void>(filename);
        static_cast<void>(loops);
        static_cast<void>(startSeconds);
        ++static_cast<XWalkMusicHostStub*>(context)->playbackCountValue;
    }
    void XWalkMusicHostStub::setMusicVolume(contextpointer context, float64 normalizedVolume)
    {
        static_cast<void>(normalizedVolume);
        ++static_cast<XWalkMusicHostStub*>(context)->controlCountValue;
    }
    void XWalkMusicHostStub::stopMusic(contextpointer context)
    {
        ++static_cast<XWalkMusicHostStub*>(context)->controlCountValue;
    }
    void XWalkMusicHostStub::pauseMusic(contextpointer context)
    {
        ++static_cast<XWalkMusicHostStub*>(context)->controlCountValue;
    }
    void XWalkMusicHostStub::resumeMusic(contextpointer context)
    {
        ++static_cast<XWalkMusicHostStub*>(context)->controlCountValue;
    }
    float64 XWalkMusicHostStub::getSoundLength(contextpointer context, stringview filename)
    {
        static_cast<void>(context);
        static_cast<void>(filename);
        return 1.25;
    }
    void XWalkMusicHostStub::playTone(contextpointer context,
                                      const bytevector& pcmData,
                                      uint32 sampleRateHz,
                                      uint8 channelCount)
    {
        static_cast<void>(sampleRateHz);
        static_cast<void>(channelCount);
        XWalkMusicHostStub& backend = *static_cast<XWalkMusicHostStub*>(context);
        ++backend.toneCountValue;
        backend.toneByteCountValue = pcmData.size();
        XWALK_HAL_TRACE_UID1(RPI .302, "Music host stub accepted %zu PCM tone bytes", pcmData.size());
    }
    XWalkMusicCallbacks XWalkMusicHostStub::callbacks()
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
    uint32 XWalkMusicHostStub::enableCount() const noexcept
    {
        return enableCountValue;
    }
    uint32 XWalkMusicHostStub::playbackCount() const noexcept
    {
        return playbackCountValue;
    }
    uint32 XWalkMusicHostStub::controlCount() const noexcept
    {
        return controlCountValue;
    }
    uint32 XWalkMusicHostStub::toneCount() const noexcept
    {
        return toneCountValue;
    }
    size XWalkMusicHostStub::toneByteCount() const noexcept
    {
        return toneByteCountValue;
    }
} /* namespace xwalk::hal::sim */
