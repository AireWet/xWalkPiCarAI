/******************************************************************************
 * @file        xHal_Rpi5CarMusicTestSupport.h
 * @brief       Declares reusable xWalkMusic host-test support.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MUSIC_TEST_SUPPORT_H
#define XHAL_RPI5CAR_MUSIC_TEST_SUPPORT_H
#include "xHal_Rpi5CarMusic.h"
namespace xwalk::hal::test::music
{
/** @brief Records operations received by the simulated audio backend. */
struct TestBackend
{
    uint32 enableCount{};
    uint32 soundCount{};
    uint32 backgroundSoundCount{};
    uint32 musicCount{};
    uint32 volumeCount{};
    uint32 stopCount{};
    uint32 pauseCount{};
    uint32 resumeCount{};
    uint32 toneCount{};
    string filename{};
    optionalfloat64 soundVolume{};
    float64 musicVolume{};
    int32 loops{};
    float64 startSeconds{};
    float64 soundDurationSeconds{1.234};
    bytevector pcmData{};
    uint32 sampleRateHz{};
    uint8 channelCount{};
};
void enableOutput(contextpointer context);
void playSound(contextpointer context, stringview filename,
    optionalfloat64 normalizedVolume);
void playSoundBackground(contextpointer context, stringview filename,
    optionalfloat64 normalizedVolume);
void playMusic(contextpointer context, stringview filename, int32 loops,
    float64 startSeconds);
void setMusicVolume(contextpointer context, float64 normalizedVolume);
void stopMusic(contextpointer context);
void pauseMusic(contextpointer context);
void resumeMusic(contextpointer context);
float64 getSoundLength(contextpointer context, stringview filename);
void playTone(contextpointer context, const bytevector& pcmData,
    uint32 sampleRateHz, uint8 channelCount);
XWalkMusicCallbacks musicCallbacks();
} /* namespace xwalk::hal::test::music */
#endif /* XHAL_RPI5CAR_MUSIC_TEST_SUPPORT_H */
