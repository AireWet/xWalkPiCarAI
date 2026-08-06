/******************************************************************************
 * @file        xAgent_Rpi5CarVisionGroupTest.cpp
 * @brief       Tests every vision Agent module through GoogleTest.
 * @details     Runs existing child tests and verifies newer public contracts.
 * @project     xWalk Firmware
 * @module      xWalkVision Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBullFight.h"
#include "xAgent_Rpi5CarFaceTracking.h"
#include "xAgent_Rpi5CarGroupTestSupport.h"
#include "xAgent_Rpi5CarTreasureHunt.h"
#include "xAgent_Rpi5CarVideoCar.h"

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

TEST(XWalkAgentVisionGroup, ComputerVision)
{
    runModuleTest("xWalkComputerVision", "xWalkComputerVisionTest");
}

TEST(XWalkAgentVisionGroup, FaceTracking)
{
    const xwalk::agent::XWalkFaceTrackingConfiguration configuration;
    EXPECT_EQ(configuration.frameWidthPixels, 640U);
    EXPECT_EQ(configuration.frameHeightPixels, 480U);
    EXPECT_DOUBLE_EQ(configuration.maximumAngleDegrees, 35.0);
}

TEST(XWalkAgentVisionGroup, BullFight)
{
    const xwalk::agent::XWalkBullFightConfiguration configuration;
    EXPECT_DOUBLE_EQ(configuration.speedPercent, 50.0);
    EXPECT_EQ(configuration.sampleDelayMs, 50U);
}

TEST(XWalkAgentVisionGroup, TreasureHunt)
{
    EXPECT_EQ(xwalk::agent::XWalkTreasureHunt::colorName(
        xwalk::agent::XWalkComputerVisionColor::Purple), "purple");
    const xwalk::agent::XWalkTreasureHuntConfiguration configuration;
    EXPECT_EQ(configuration.movementDelayMs, 500U);
}

TEST(XWalkAgentVisionGroup, VideoRecording)
{
    runModuleTest("xWalkVideoRecording", "xWalkVideoRecordingTest");
}

TEST(XWalkAgentVisionGroup, VideoCar)
{
    EXPECT_EQ(xwalk::agent::XWalkVideoCar::motionName(
        xwalk::agent::XWalkVideoCarMotion::Forward), "forward");
    const xwalk::agent::XWalkVideoCarConfiguration configuration;
    EXPECT_EQ(configuration.maximumSpeedPercent, 100U);
}

TEST(XWalkAgentVisionGroup, CameraCapture)
{
    runModuleTest("xWalkCameraCapture", "xWalkCameraCaptureTest");
}

} /* namespace */
