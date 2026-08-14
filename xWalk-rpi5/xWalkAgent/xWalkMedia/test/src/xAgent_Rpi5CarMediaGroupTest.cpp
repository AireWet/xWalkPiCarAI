/******************************************************************************
 * @file        xAgent_Rpi5CarMediaGroupTest.cpp
 * @brief       Tests every media Agent module through GoogleTest.
 * @project     xWalk Firmware
 * @module      xWalkMedia Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarSoundBackgroundMusic.h"

#include <gtest/gtest.h>

TEST(XWalkAgentMediaGroup, SoundBackgroundMusic)
{
    const xwalk::agent::XWalkSoundBackgroundMusicConfiguration configuration;
    EXPECT_EQ(configuration.hornFilename, "car-double-horn.wav");
    EXPECT_EQ(configuration.musicFilename, "slow-trail-Ahjay_Stelino.mp3");
    EXPECT_DOUBLE_EQ(configuration.musicVolumePercent, 20.0);
    EXPECT_EQ(configuration.postSoundDelayMs, 50U);
}
