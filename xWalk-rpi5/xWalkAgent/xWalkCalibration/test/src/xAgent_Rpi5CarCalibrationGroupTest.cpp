/******************************************************************************
 * @file        xAgent_Rpi5CarCalibrationGroupTest.cpp
 * @brief       Runs every calibration module host test through GoogleTest.
 * @details     Isolates the existing deterministic child tests by process.
 * @project     xWalk Firmware
 * @module      xWalkCalibration Group GoogleTest
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

    /** @brief Runs one child calibration test with an optional configuration path. */
    void runModuleTest(const char* moduleDirectory,
                       const char* executableName,
                       const char* moduleName,
                       bool needsConfiguration)
    {
        const xwalk::agent::filesystempath binary =
            xwalk::agent::test::childTestExecutable(moduleDirectory, executableName);
        const xwalk::agent::filesystempath directory = xwalk::agent::test::groupTestDataDirectory(moduleName);
        std::filesystem::create_directories(directory);
        const xwalk::agent::filesystempath configuration = directory / "test.conf";
        const pid_t childProcess = ::fork();
        ASSERT_GE(childProcess, 0);
        if (childProcess == 0)
        {
            if (needsConfiguration)
            {
                ::execl(binary.c_str(), binary.c_str(), configuration.c_str(), static_cast<char*>(nullptr));
            }
            else
            {
                ::execl(binary.c_str(), binary.c_str(), static_cast<char*>(nullptr));
            }
            ::_exit(127);
        }
        int status{};
        ASSERT_EQ(::waitpid(childProcess, &status, 0), childProcess);
        ASSERT_TRUE(WIFEXITED(status));
        EXPECT_EQ(WEXITSTATUS(status), 0);
    }

    TEST(XWalkAgentCalibrationGroup, GrayscaleCalibration)
    {
        runModuleTest("xWalkGrayscaleCalibration", "xWalkGrayscaleCalibrationTest", "grayscale", true);
    }

    TEST(XWalkAgentCalibrationGroup, ServoMotorCalibration)
    {
        runModuleTest("xWalkServoMotorCalibration", "xWalkServoMotorCalibrationTest", "servo-motor", true);
    }

    TEST(XWalkAgentCalibrationGroup, ServoZeroing)
    {
        runModuleTest("xWalkServoZeroing", "xWalkServoZeroingTest", "servo-zeroing", false);
    }

} /* namespace */
