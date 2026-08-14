/******************************************************************************
 * @file        xAgent_Rpi5CarVehicleGroupTest.cpp
 * @brief       Runs every vehicle module host test through GoogleTest.
 * @details     Isolates the existing deterministic child tests by process.
 * @project     xWalk Firmware
 * @module      xWalkVehicle Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarGroupTestSupport.h"

#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

/** @brief Runs one argument-taking child module test in an isolated process. */
void runModuleTest(const char* moduleDirectory, const char* executableName,
    const char* moduleName)
{
    const xwalk::agent::filesystempath binary =
        xwalk::agent::test::childTestExecutable(moduleDirectory, executableName);
    const xwalk::agent::filesystempath directory =
        xwalk::agent::test::groupTestDataDirectory(moduleName);
    std::filesystem::create_directories(directory);
    const xwalk::agent::filesystempath configuration = directory / "test.conf";
    const pid_t childProcess = ::fork();
    ASSERT_GE(childProcess, 0);
    if (childProcess == 0)
    {
        ::execl(binary.c_str(), binary.c_str(), configuration.c_str(),
            static_cast<char*>(nullptr));
        ::_exit(127);
    }
    int status{};
    ASSERT_EQ(::waitpid(childProcess, &status, 0), childProcess);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
}

TEST(XWalkAgentVehicleGroup, Picarx)
{
    runModuleTest("xWalkPicarx", "xWalkPicarxTest", "picarx");
}

TEST(XWalkAgentVehicleGroup, LineTracking)
{
    runModuleTest("xWalkLineTracking", "xWalkLineTrackingTest", "line-tracking");
}

TEST(XWalkAgentVehicleGroup, MoveExample)
{
    runModuleTest("xWalkMoveExample", "xWalkMoveExampleTest", "move-example");
}

TEST(XWalkAgentVehicleGroup, KeyboardControl)
{
    runModuleTest("xWalkKeyboardControl", "xWalkKeyboardControlTest", "keyboard-control");
}

TEST(XWalkAgentVehicleGroup, ObstacleAvoidance)
{
    runModuleTest("xWalkObstacleAvoidance", "xWalkObstacleAvoidanceTest", "obstacle-avoidance");
}

TEST(XWalkAgentVehicleGroup, CliffDetection)
{
    runModuleTest("xWalkCliffDetection", "xWalkCliffDetectionTest", "cliff-detection");
}

TEST(XWalkAgentVehicleGroup, SelfDrive)
{
    runModuleTest("xWalkSelfDrive", "xWalkSelfDriveTest", "self-drive");
}

} /* namespace */
