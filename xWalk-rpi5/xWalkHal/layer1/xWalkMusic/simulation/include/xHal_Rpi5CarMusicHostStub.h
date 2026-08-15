/******************************************************************************
 * @file        xHal_Rpi5CarMusicHostStub.h
 * @brief       Declares the silent in-memory Music host stub.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MUSIC_HOST_STUB_H
#define XHAL_RPI5CAR_MUSIC_HOST_STUB_H
#include "xHal_Rpi5CarMusic.h"
namespace xwalk::hal::sim
{
    /** @brief Records Music callbacks without opening an audio device. */
    class XWalkMusicHostStub final
    {
        private:
            uint32 enableCountValue{};
            uint32 playbackCountValue{};
            uint32 controlCountValue{};
            uint32 toneCountValue{};
            size toneByteCountValue{};

        public:
            static void enableOutput(contextpointer context);
            static void playSound(contextpointer context, stringview filename, optionalfloat64 normalizedVolume);
            static void
            playSoundBackground(contextpointer context, stringview filename, optionalfloat64 normalizedVolume);
            static void playMusic(contextpointer context, stringview filename, int32 loops, float64 startSeconds);
            static void setMusicVolume(contextpointer context, float64 normalizedVolume);
            static void stopMusic(contextpointer context);
            static void pauseMusic(contextpointer context);
            static void resumeMusic(contextpointer context);
            static float64 getSoundLength(contextpointer context, stringview filename);
            static void
            playTone(contextpointer context, const bytevector& pcmData, uint32 sampleRateHz, uint8 channelCount);
            static XWalkMusicCallbacks callbacks();
            uint32 enableCount() const noexcept;
            uint32 playbackCount() const noexcept;
            uint32 controlCount() const noexcept;
            uint32 toneCount() const noexcept;
            size toneByteCount() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_MUSIC_HOST_STUB_H */
