/******************************************************************************
 * @file        xAgent_Rpi5CarAgentHardwareGoogleTest.cpp
 * @brief       Verifies every xWalkAgent group in the hardware build profile.
 *
 * @details
 * Runs each hardware-profile group GoogleTest executable in an isolated process.
 * Execution remains opt-in and requires the documented Raspberry Pi safety approval.
 *
 * @project     xWalk Firmware
 * @module      xWalkAgent Hardware GoogleTest
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
     * @brief Runs one Agent hardware-profile group test in an isolated process.
     * @param[in] groupDirectory Group build-directory name.
     * @param[in] executableName Group hardware GoogleTest executable name.
     */
    void runHardwareGroupTest(const char* groupDirectory, const char* executableName)
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

    TEST(XWalkAgentHardware, VehicleGroup)
    {
        runHardwareGroupTest("xWalkVehicle", "xWalkAgentVehicleGroupHardwareTest");
    }

    TEST(XWalkAgentHardware, CalibrationGroup)
    {
        runHardwareGroupTest("xWalkCalibration", "xWalkAgentCalibrationGroupHardwareTest");
    }

    TEST(XWalkAgentHardware, VisionGroup)
    {
        runHardwareGroupTest("xWalkVision", "xWalkAgentVisionGroupHardwareTest");
    }

    TEST(XWalkAgentHardware, MediaGroup)
    {
        runHardwareGroupTest("xWalkMedia", "xWalkAgentMediaGroupHardwareTest");
    }

    TEST(XWalkAgentHardware, VoiceGroup)
    {
        runHardwareGroupTest("xWalkVoice", "xWalkAgentVoiceGroupHardwareTest");
    }

    TEST(XWalkAgentHardware, ConnectivityGroup)
    {
        runHardwareGroupTest("xWalkConnectivity", "xWalkAgentConnectivityGroupHardwareTest");
    }

    TEST(XWalkAgentHardware, PlatformGroup)
    {
        runHardwareGroupTest("xWalkPlatform", "xWalkAgentPlatformGroupHardwareTest");
    }

} /* namespace */
