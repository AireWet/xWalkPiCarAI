/******************************************************************************
 * @file        xAgent_Rpi5CarConnectivityGroupTest.cpp
 * @brief       Tests every connectivity Agent module through GoogleTest.
 * @project     xWalk Firmware
 * @module      xWalkConnectivity Group GoogleTest
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarAppControl.h"
#include "xAgent_Rpi5CarGroupTestSupport.h"

#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{

/** @brief Runs the existing SPI child module test in an isolated process. */
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

TEST(XWalkAgentConnectivityGroup, AppControl)
{
    const xwalk::agent::XWalkAppControlConfiguration configuration;
    EXPECT_EQ(configuration.controllerName, "Picarx-001");
    EXPECT_EQ(configuration.controllerPort, 8'765U);
    EXPECT_EQ(configuration.maximumLineRecoverySamples, 1'000U);
}

TEST(XWalkAgentConnectivityGroup, SpiTransfer)
{
    runModuleTest("xWalkSpiTransfer", "xWalkSpiTransferTest");
}

} /* namespace */
