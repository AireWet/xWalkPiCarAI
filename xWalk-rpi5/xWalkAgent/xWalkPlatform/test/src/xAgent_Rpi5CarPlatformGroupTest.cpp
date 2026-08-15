/******************************************************************************
 * @file        xAgent_Rpi5CarPlatformGroupTest.cpp
 * @brief       Tests every platform Agent module through GoogleTest.
 * @project     xWalk Firmware
 * @module      xWalkPlatform Group GoogleTest
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

TEST(XWalkAgentPlatformGroup, Boot)
{
    const xwalk::agent::filesystempath binary = xwalk::agent::test::childTestExecutable("xWalkBoot", "xWalkBootTest");
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
