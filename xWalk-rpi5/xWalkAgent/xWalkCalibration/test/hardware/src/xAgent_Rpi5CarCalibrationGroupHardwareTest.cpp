/******************************************************************************
 * @file        xAgent_Rpi5CarCalibrationGroupHardwareTest.cpp
 * @brief       Verifies the calibration group in the Raspberry Pi build profile.
 *
 * @details
 * Registers one GoogleTest case per calibration coordinator in the hardware profile.
 *
 * @project     xWalk Firmware
 * @module      xWalkCalibration Group Hardware Test
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

#include "xAgent_Rpi5CarGrayscaleCalibration.h"
#include "xAgent_Rpi5CarServoMotorCalibration.h"
#include "xAgent_Rpi5CarServoZeroing.h"

#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentCalibrationHardwareGroup, GrayscaleCalibration)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkGrayscaleCalibration>);
}

TEST(XWalkAgentCalibrationHardwareGroup, ServoMotorCalibration)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkServoMotorCalibration>);
}

TEST(XWalkAgentCalibrationHardwareGroup, ServoZeroing)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkServoZeroing>);
}
