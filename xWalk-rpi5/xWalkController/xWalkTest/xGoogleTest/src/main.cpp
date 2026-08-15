/******************************************************************************
 * @file        main.cpp
 * @brief       Provides the standalone xWalk CLI GoogleTest entry point.
 *
 * @details
 * Loads strict XML selection, registers the Controller unit scenario, and
 * isolates its assertion-based legacy entry point in a child process.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI GoogleTest
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xCliTestConfig.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>

/** @brief Renamed entry point for the existing Controller host test. */
int xWalkControllerLegacyMain(int argumentCount, char* argumentValues[]);

/** @brief Contains CLI unit-test registration and process-isolation helpers. */
namespace
{

    using clitestwithargs = int (*)(int argumentCount, char* argumentValues[]);

    /**
     * @brief Runs one argument-taking assertion-based test in a child process.
     * @param[in] function Non-null test entry point.
     * @param[in] arguments Complete owned arguments including a synthesized executable name.
     */
    void runIsolated(clitestwithargs function, ctrl::stringvector arguments)
    {
        ASSERT_NE(function, nullptr);
        ctrl::charpointervector argumentPointers;
        argumentPointers.reserve(arguments.size() + 1U);
        for (ctrl::string& argument : arguments)
        {
            argumentPointers.push_back(argument.data());
        }
        argumentPointers.push_back(nullptr);

        const pid_t childProcess = ::fork();
        ASSERT_GE(childProcess, 0) << "fork failed for CLI test";
        if (childProcess == 0)
        {
            ::_exit(function(static_cast<int>(arguments.size()), argumentPointers.data()));
        }

        int childStatus{};
        const pid_t completedProcess = ::waitpid(childProcess, &childStatus, 0);
        ASSERT_EQ(completedProcess, childProcess) << "waitpid failed for CLI test";
        ASSERT_TRUE(WIFEXITED(childStatus));
        EXPECT_EQ(WEXITSTATUS(childStatus), 0);
    }

    TEST(XWalkControllerGroup, Controller)
    {
        ctrl::errorcode pathError;
        const ctrl::filesystempath executablePath = std::filesystem::read_symlink("/proc/self/exe", pathError);
        ASSERT_FALSE(pathError) << "cannot resolve CLI GoogleTest executable: " << pathError.message();
        ASSERT_FALSE(executablePath.empty());

        const ctrl::filesystempath configurationPath =
            executablePath.parent_path() / "xWalkController" / "xWalkApp" / "test-data" / "cli-central-test.conf";
        runIsolated(&xWalkControllerLegacyMain, {"xWalkControllerTest", configurationPath.string()});
    }

} /* namespace */

/**
 * @brief Runs the standalone xWalk CLI unit-test executable.
 * @param[in,out] argumentCount Process argument count consumed by GoogleTest.
 * @param[in,out] argumentValues Non-owning process argument array valid for this call.
 * @return Zero when every selected test passes; otherwise a non-zero status.
 */
int main(int argumentCount, char* argumentValues[])
{
    ctrl::errorcode pathError;
    const ctrl::filesystempath executablePath = std::filesystem::read_symlink("/proc/self/exe", pathError);
    const ::ctrl::boolean pathErrorExecutablePathInvalid =
        static_cast<::ctrl::boolean>(pathError || executablePath.empty());
    if (pathErrorExecutablePathInvalid)
    {
        std::cerr << "xCliGoogleTest path error: cannot resolve /proc/self/exe: " << pathError.message() << '\n';
        return EXIT_FAILURE;
    }

    xwalk::agent::test::XWalkCliTestConfig configuration;
    ctrl::string error;
    const ctrl::filesystempath configurationPath = executablePath.parent_path() / "xCliGoogleTestConfig.xml";
    const ctrl::boolean configurationLoaded =
        configuration.load(configurationPath, xwalk::agent::test::availableCliTests(), error);
    if (configurationLoaded == false)
    {
        std::cerr << "xCliGoogleTest configuration error: " << error << '\n';
        return EXIT_FAILURE;
    }

    const ctrl::boolean standardFilterSelected = xwalk::agent::test::hasGoogleTestFilter(argumentCount, argumentValues);
    ::testing::InitGoogleTest(&argumentCount, argumentValues);
    GTEST_FLAG_SET(catch_exceptions, false);
    if (!standardFilterSelected)
    {
        GTEST_FLAG_SET(filter, xwalk::agent::test::configuredCliTestFilter(configuration));
    }
    return RUN_ALL_TESTS();
}
