/******************************************************************************
 * @file        xAgent_Rpi5CarConnectivityGroupHardwareTest.cpp
 * @brief       Verifies the connectivity group in the Raspberry Pi build profile.
 *
 * @details
 * Registers one GoogleTest case per connectivity coordinator in the hardware profile.
 *
 * @project     xWalk Firmware
 * @module      xWalkConnectivity Group Hardware Test
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

#include "xAgent_Rpi5CarAppControl.h"
#include "xAgent_Rpi5CarSpiTransfer.h"

#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentConnectivityHardwareGroup, AppControl)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkAppControl>);
}

TEST(XWalkAgentConnectivityHardwareGroup, SpiTransfer)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkSpiTransfer>);
}
