/******************************************************************************
 * @file        xAgent_Rpi5CarVehicleGroupHardwareTest.cpp
 * @brief       Verifies the vehicle group in the Raspberry Pi build profile.
 *
 * @details
 * Registers one GoogleTest case per vehicle coordinator in the hardware profile.
 *
 * @project     xWalk Firmware
 * @module      xWalkVehicle Group Hardware Test
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

#include "xAgent_Rpi5CarCliffDetection.h"
#include "xAgent_Rpi5CarKeyboardControl.h"
#include "xAgent_Rpi5CarLineTracking.h"
#include "xAgent_Rpi5CarMoveExample.h"
#include "xAgent_Rpi5CarObstacleAvoidance.h"
#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarSelfDrive.h"

#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentVehicleHardwareGroup, Picarx)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkPicarx>);
}

TEST(XWalkAgentVehicleHardwareGroup, LineTracking)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkLineTracking>);
}

TEST(XWalkAgentVehicleHardwareGroup, MoveExample)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkMoveExample>);
}

TEST(XWalkAgentVehicleHardwareGroup, KeyboardControl)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkKeyboardControl>);
}

TEST(XWalkAgentVehicleHardwareGroup, ObstacleAvoidance)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkObstacleAvoidance>);
}

TEST(XWalkAgentVehicleHardwareGroup, CliffDetection)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkCliffDetection>);
}

TEST(XWalkAgentVehicleHardwareGroup, SelfDrive)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkSelfDrive>);
}
