/******************************************************************************
 * @file        xAgent_Rpi5CarAgentGoogleTest.cpp
 * @brief       Verifies every xWalkAgent functional group through GoogleTest.
 *
 * @details
 * Runs each deterministic group GoogleTest executable in an isolated process.
 * Every group executable retains its own case per child Agent module.
 *
 * @project     xWalk Firmware
 * @module      xWalkAgent GoogleTest
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarGroupTestSupport.h"

#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

    /**
     * @brief Runs one Agent group GoogleTest executable in an isolated process.
     * @param[in] groupDirectory Group build-directory name.
     * @param[in] executableName Group GoogleTest executable name.
     */
    void runGroupTest(const char* groupDirectory, const char* executableName)
    {
        const xwalk::agent::filesystempath binary =
            xwalk::agent::test::childTestExecutable(groupDirectory, executableName);
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

    TEST(XWalkAgent, VehicleGroup)
    {
        runGroupTest("xWalkVehicle", "xWalkAgentVehicleGroupTest");
    }

    TEST(XWalkAgent, CalibrationGroup)
    {
        runGroupTest("xWalkCalibration", "xWalkAgentCalibrationGroupTest");
    }

    TEST(XWalkAgent, VisionGroup)
    {
        runGroupTest("xWalkVision", "xWalkAgentVisionGroupTest");
    }

    TEST(XWalkAgent, MediaGroup)
    {
        runGroupTest("xWalkMedia", "xWalkAgentMediaGroupTest");
    }

    TEST(XWalkAgent, VoiceGroup)
    {
        runGroupTest("xWalkVoice", "xWalkAgentVoiceGroupTest");
    }

    TEST(XWalkAgent, ConnectivityGroup)
    {
        runGroupTest("xWalkConnectivity", "xWalkAgentConnectivityGroupTest");
    }

    TEST(XWalkAgent, PlatformGroup)
    {
        runGroupTest("xWalkPlatform", "xWalkAgentPlatformGroupTest");
    }

} /* namespace */
