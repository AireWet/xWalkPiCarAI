/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceGroupTest.cpp
 * @brief       Tests every voice Agent module through GoogleTest.
 * @details     Runs existing child tests and verifies newer public contracts.
 * @project     xWalk Firmware
 * @module      xWalkVoice Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarGroupTestSupport.h"
#include "xAgent_Rpi5CarOnlineLlmTest.h"
#include "xAgent_Rpi5CarGptCar.h"
#include "xAgent_Rpi5CarStorytellingRobot.h"
#include "xAgent_Rpi5CarTextVisionTalk.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"
#include "xAgent_Rpi5CarVoicePromptCar.h"

#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

/** @brief Runs one no-argument child module test in an isolated process. */
void runModuleTest(const char* moduleDirectory, const char* executableName)
{
    const xwalk::agent::filesystempath binary =
        xwalk::agent::test::childTestExecutable(moduleDirectory, executableName);
    const pid_t childProcess = ::fork();
    ASSERT_GE(childProcess, 0);
    if (childProcess == 0)
    {
        ::execl(binary.c_str(), binary.c_str(), static_cast<char*>(nullptr));
        ::_exit(127);
    }
    int status{};
    ASSERT_EQ(::waitpid(childProcess, &status, 0), childProcess);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

TEST(XWalkAgentVoiceGroup, LocalVoiceChatbot)
{
    runModuleTest("xWalkLocalVoiceChatbot", "xWalkLocalVoiceChatbotTest");
}

TEST(XWalkAgentVoiceGroup, VoicePromptCar)
{
    const xwalk::agent::XWalkVoicePromptCarConfiguration configuration;
    EXPECT_DOUBLE_EQ(configuration.speedPercent, 30.0);
    EXPECT_DOUBLE_EQ(configuration.steeringAngle, 20.0);
    EXPECT_EQ(configuration.driveDurationMs, 2'000U);
}

TEST(XWalkAgentVoiceGroup, StorytellingRobot)
{
    const xwalk::agent::XWalkStorytellingRobotConfiguration configuration;
    EXPECT_EQ(configuration.outwardLegDurationMs, 3'000U);
    EXPECT_EQ(configuration.homeLegDurationMs, 6'000U);
    EXPECT_FALSE(configuration.greeting.empty());
}

TEST(XWalkAgentVoiceGroup, VoiceControlledCar)
{
    EXPECT_TRUE(xwalk::agent::XWalkVoiceControlledCar::containsWakeWord(
        "noise HEY ROBOT noise"));
    EXPECT_EQ(xwalk::agent::XWalkVoiceControlledCar::classifyCommand(
        "please go forward"), xwalk::agent::XWalkVoiceControlledCarCommand::Forward);
    EXPECT_EQ(xwalk::agent::XWalkVoiceControlledCar::classifyCommand(
        "sleep now"), xwalk::agent::XWalkVoiceControlledCarCommand::Sleep);
}

TEST(XWalkAgentVoiceGroup, TextVisionTalk)
{
    const xwalk::agent::XWalkTextVisionTalkConfiguration configuration;
    EXPECT_EQ(configuration.maximumMessages, 20U);
    EXPECT_EQ(configuration.cameraWarmupMs, 2'000U);
    EXPECT_FALSE(configuration.welcome.empty());
}

TEST(XWalkAgentVoiceGroup, OnlineLlmTest)
{
    const xwalk::agent::XWalkOnlineLlmTestConfiguration configuration;
    EXPECT_EQ(configuration.maximumMessages, 20U);
    EXPECT_EQ(configuration.instructions, "You are a helpful assistant.");
}

TEST(XWalkAgentVoiceGroup, VoiceActiveCar)
{
    runModuleTest("xWalkVoiceActiveCar", "xWalkVoiceActiveCarTest");
}

TEST(XWalkAgentVoiceGroup, VoiceActiveCarGpt)
{
    runModuleTest("xWalkVoiceActiveCarGpt", "xWalkVoiceActiveCarGptTest");
}

TEST(XWalkAgentVoiceGroup, GptCar)
{
    runModuleTest("xWalkGptCar", "xWalkGptCarTest");
}

} /* namespace */
