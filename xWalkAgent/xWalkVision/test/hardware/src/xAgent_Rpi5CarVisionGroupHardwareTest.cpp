/******************************************************************************
 * @file        xAgent_Rpi5CarVisionGroupHardwareTest.cpp
 * @brief       Verifies the vision group in the Raspberry Pi build profile.
 *
 * @details
 * Registers one GoogleTest case per vision coordinator in the hardware profile.
 *
 * @project     xWalk Firmware
 * @module      xWalkVision Group Hardware Test
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

#include "xAgent_Rpi5CarBullFight.h"
#include "xAgent_Rpi5CarCameraCapture.h"
#include "xAgent_Rpi5CarComputerVision.h"
#include "xAgent_Rpi5CarFaceTracking.h"
#include "xAgent_Rpi5CarTreasureHunt.h"
#include "xAgent_Rpi5CarVideoCar.h"
#include "xAgent_Rpi5CarVideoRecording.h"

#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentVisionHardwareGroup, ComputerVision)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkComputerVision>);
}

TEST(XWalkAgentVisionHardwareGroup, FaceTracking)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkFaceTracking>);
}

TEST(XWalkAgentVisionHardwareGroup, BullFight)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkBullFight>);
}

TEST(XWalkAgentVisionHardwareGroup, TreasureHunt)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkTreasureHunt>);
}

TEST(XWalkAgentVisionHardwareGroup, VideoRecording)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkVideoRecording>);
}

TEST(XWalkAgentVisionHardwareGroup, VideoCar)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkVideoCar>);
}

TEST(XWalkAgentVisionHardwareGroup, CameraCapture)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkCameraCapture>);
}
