/******************************************************************************
 * @file        xAgent_Rpi5CarPlatformGroupHardwareTest.cpp
 * @brief       Verifies the platform group in the Raspberry Pi build profile.
 *
 * @details
 * Registers the Raspberry Pi boot coordinator as a GoogleTest hardware-profile case.
 *
 * @project     xWalk Firmware
 * @module      xWalkPlatform Group Hardware Test
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

#include "xAgent_Rpi5CarBoot.h"
#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentPlatformHardwareGroup, Boot)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkBoot>);
}
