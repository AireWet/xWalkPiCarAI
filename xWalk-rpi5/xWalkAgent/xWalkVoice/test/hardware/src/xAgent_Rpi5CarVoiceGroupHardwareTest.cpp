/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceGroupHardwareTest.cpp
 * @brief       Verifies the voice group in the Raspberry Pi build profile.
 *
 * @details
 * Registers one GoogleTest case per voice coordinator in the hardware profile.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoice Group Hardware Test
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

#include "xAgent_Rpi5CarLocalVoiceChatbot.h"
#include "xAgent_Rpi5CarGptCar.h"
#include "xAgent_Rpi5CarOnlineLlmTest.h"
#include "xAgent_Rpi5CarStorytellingRobot.h"
#include "xAgent_Rpi5CarTextVisionTalk.h"
#include "xAgent_Rpi5CarVoiceActiveCar.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"
#include "xAgent_Rpi5CarVoicePromptCar.h"

#include <gtest/gtest.h>
#include <type_traits>

TEST(XWalkAgentVoiceHardwareGroup, LocalVoiceChatbot)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkLocalVoiceChatbot>);
}

TEST(XWalkAgentVoiceHardwareGroup, VoicePromptCar)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkVoicePromptCar>);
}

TEST(XWalkAgentVoiceHardwareGroup, StorytellingRobot)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkStorytellingRobot>);
}

TEST(XWalkAgentVoiceHardwareGroup, VoiceControlledCar)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkVoiceControlledCar>);
}

TEST(XWalkAgentVoiceHardwareGroup, TextVisionTalk)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkTextVisionTalk>);
}

TEST(XWalkAgentVoiceHardwareGroup, OnlineLlmTest)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkOnlineLlmTest>);
}

TEST(XWalkAgentVoiceHardwareGroup, VoiceActiveCar)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkVoiceActiveCar>);
}

TEST(XWalkAgentVoiceHardwareGroup, VoiceActiveCarGpt)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkVoiceActiveCarGpt>);
}

TEST(XWalkAgentVoiceHardwareGroup, GptCar)
{
    EXPECT_TRUE(std::is_class_v<xwalk::agent::XWalkGptCar>);
}
