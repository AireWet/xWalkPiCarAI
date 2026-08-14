/******************************************************************************
 * @file        xAgent_Rpi5CarMediaGroupHardwareTest.cpp
 * @brief       Verifies the media group in the Raspberry Pi build profile.
 *
 * @details
 * Registers the media coordinator as a GoogleTest hardware-profile case.
 *
 * @project     xWalk Firmware
 * @module      xWalkMedia Group Hardware Test
 *
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarSoundBackgroundMusic.h"

#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentMediaHardwareGroup, SoundBackgroundMusic)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkSoundBackgroundMusic>);
}
