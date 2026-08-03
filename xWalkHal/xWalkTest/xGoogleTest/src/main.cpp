/******************************************************************************
 * @file        main.cpp
 * @brief       Provides the single centralized GoogleTest process entry point.
 *
 * @details
 * Initializes GoogleTest, loads XML configuration, processes temporary test
 * selections, applies filter precedence, and runs the selected HAL host tests.
 *
 * @project     xWalk Firmware
 * @module      xGoogleTest
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "TestConfig.hpp"
#include "TestRunner.hpp"

#include <gtest/gtest.h>

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the centralized xWalk HAL host-test executable.
 *
 * @param[in,out] argumentCount
 * Process argument count consumed by GoogleTest and custom selection parsing.
 *
 * @param[in,out] argumentValues
 * Non-owning process argument array valid for the duration of this call.
 *
 * @return
 * Zero when every selected test passes; otherwise a non-zero error status.
 */
int main(int argumentCount, char* argumentValues[])
{
    std::error_code pathError;
    const xwalk::hal::filesystempath executablePath =
        std::filesystem::read_symlink("/proc/self/exe", pathError);
    if (pathError || executablePath.empty())
    {
        std::cerr << "xGoogleTest path error: cannot resolve /proc/self/exe: "
                  << pathError.message() << '\n';
        return EXIT_FAILURE;
    }
    const xwalk::hal::filesystempath executableDirectory =
        executablePath.parent_path();

    xwalk::hal::test::TestProfile profile = xwalk::hal::test::TestProfile::Host;
    xwalk::hal::filesystempath runtimeConfigurationPath =
        executableDirectory / "xHal_Rpi5CarGoogleTestConfig.yml";
    xwalk::hal::string error;
    if (!xwalk::hal::test::TestRunner::processProfile(
            argumentCount, argumentValues, profile,
            runtimeConfigurationPath, error))
    {
        std::cerr << "xGoogleTest profile error: " << error << '\n';
        return EXIT_FAILURE;
    }
#if !defined(XWALK_GOOGLE_TEST_HARDWARE_PROFILE)
    if (profile == xwalk::hal::test::TestProfile::Hardware)
    {
        std::cerr << "xGoogleTest profile error: hardware tests were not built; "
                     "configure an RPI build with BUILD_TESTING=ON\n";
        return EXIT_FAILURE;
    }
#endif

    try
    {
        xwalk::hal::test::TestRunner runner(
            profile, executableDirectory / "xWalkHal",
            runtimeConfigurationPath);
        runner.registerTests();
        const xwalk::hal::boolean standardFilterSelected =
            runner.hasStandardFilter(argumentCount, argumentValues);

        ::testing::InitGoogleTest(&argumentCount, argumentValues);
        GTEST_FLAG_SET(catch_exceptions, false);

        xwalk::hal::test::TestConfig configuration;
        const xwalk::hal::filesystempath configurationPath(
            executableDirectory /
            (profile == xwalk::hal::test::TestProfile::Hardware
                    ? "hardware_test_config.xml" : "test_config.xml"));
        if (!configuration.load(
                configurationPath, runner.availableTests(), error))
        {
            std::cerr << "xGoogleTest configuration error: " << error << '\n';
            std::cerr << runner.validTestsText();
            return EXIT_FAILURE;
        }
        if (!runner.processSelections(argumentCount, argumentValues, error))
        {
            std::cerr << "xGoogleTest selection error: " << error << '\n';
            std::cerr << runner.validTestsText();
            return EXIT_FAILURE;
        }

        runner.applyFilter(configuration, standardFilterSelected);
        return RUN_ALL_TESTS();
    }
    catch (const xwalk::hal::standardexception& exception)
    {
        std::cerr << "xGoogleTest runtime configuration error: "
                  << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}
