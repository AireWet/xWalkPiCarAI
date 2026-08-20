/******************************************************************************
 * @file        main.cpp
 * @brief       Provides the standalone xWalk CLI sequence-test entry point.
 *
 * @details
 * Loads strict XML selection, registers bounded Controller command sequences,
 * and isolates every assertion-based scenario in its own child process.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 *
 * @author      Joxy John
 * @date        2026-08-06
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
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTypes.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>

/** @brief Entry point for the generic bounded Controller sequence test. */
int xWalkControllerSequenceHostTest();

#define XWALK_DECLARE_COMMAND_TEST(Name)                                                                               \
    int xWalk##Name##CommandSequenceHostTest(int argumentCount, char* argumentValues[])
XWALK_DECLARE_COMMAND_TEST(Help);
XWALK_DECLARE_COMMAND_TEST(Doctor);
XWALK_DECLARE_COMMAND_TEST(ServoZeroing);
XWALK_DECLARE_COMMAND_TEST(Move);
XWALK_DECLARE_COMMAND_TEST(KeyboardControl);
XWALK_DECLARE_COMMAND_TEST(ObstacleAvoidance);
XWALK_DECLARE_COMMAND_TEST(CliffDetection);
XWALK_DECLARE_COMMAND_TEST(ComputerVision);
XWALK_DECLARE_COMMAND_TEST(FaceTracking);
XWALK_DECLARE_COMMAND_TEST(BullFight);
XWALK_DECLARE_COMMAND_TEST(TreasureHunt);
XWALK_DECLARE_COMMAND_TEST(VideoRecording);
XWALK_DECLARE_COMMAND_TEST(VideoCar);
XWALK_DECLARE_COMMAND_TEST(AppControl);
XWALK_DECLARE_COMMAND_TEST(SoundBackgroundMusic);
XWALK_DECLARE_COMMAND_TEST(Turn);
XWALK_DECLARE_COMMAND_TEST(Camera);
XWALK_DECLARE_COMMAND_TEST(Sensor);
XWALK_DECLARE_COMMAND_TEST(Spi);
XWALK_DECLARE_COMMAND_TEST(LineTracking);
XWALK_DECLARE_COMMAND_TEST(SelfDrive);
XWALK_DECLARE_COMMAND_TEST(Sound);
XWALK_DECLARE_COMMAND_TEST(VoiceChat);
XWALK_DECLARE_COMMAND_TEST(VoiceActiveCar);
XWALK_DECLARE_COMMAND_TEST(VoiceActiveCarGpt);
XWALK_DECLARE_COMMAND_TEST(GptCar);
XWALK_DECLARE_COMMAND_TEST(VoiceControlledCar);
XWALK_DECLARE_COMMAND_TEST(VoicePromptCar);
XWALK_DECLARE_COMMAND_TEST(StorytellingRobot);
XWALK_DECLARE_COMMAND_TEST(TextVisionTalk);
XWALK_DECLARE_COMMAND_TEST(OnlineLlmTest);
XWALK_DECLARE_COMMAND_TEST(Calibration);
#undef XWALK_DECLARE_COMMAND_TEST

/** @brief Contains CLI sequence registration and process-isolation helpers. */
namespace
{

    using sequencetestnoargs = int (*)();
    using sequencetestwithargs = int (*)(int argumentCount, char* argumentValues[]);

    /**
     * @brief Runs one no-argument assertion-based sequence in a child process.
     * @param[in] function Non-null sequence entry point.
     */
    void runIsolated(sequencetestnoargs function)
    {
        ASSERT_NE(function, nullptr);
        const pid_t childProcess = ::fork();
        ASSERT_GE(childProcess, 0) << "fork failed for CLI sequence test";
        if (childProcess == 0)
        {
            const int status = function();
#if defined(XWALK_GCC_COVERAGE)
            __gcov_dump();
#endif
            ::_exit(status);
        }
        int childStatus{};
        const pid_t completedProcess = ::waitpid(childProcess, &childStatus, 0);
        ASSERT_EQ(completedProcess, childProcess);
        ASSERT_TRUE(WIFEXITED(childStatus));
        EXPECT_EQ(WEXITSTATUS(childStatus), 0);
    }

    /**
     * @brief Runs one argument-taking assertion-based sequence in a child process.
     * @param[in] function Non-null sequence entry point.
     * @param[in] arguments Complete owned arguments including a synthesized executable name.
     */
    void runIsolated(sequencetestwithargs function, ctrl::stringvector arguments)
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
        ASSERT_GE(childProcess, 0) << "fork failed for CLI sequence test";
        if (childProcess == 0)
        {
            const int status = function(static_cast<int>(arguments.size()), argumentPointers.data());
#if defined(XWALK_GCC_COVERAGE)
            __gcov_dump();
#endif
            ::_exit(status);
        }
        int childStatus{};
        const pid_t completedProcess = ::waitpid(childProcess, &childStatus, 0);
        ASSERT_EQ(completedProcess, childProcess);
        ASSERT_TRUE(WIFEXITED(childStatus));
        EXPECT_EQ(WEXITSTATUS(childStatus), 0);
    }

    /**
     * @brief Runs one command sequence with a unique build-local configuration path.
     * @param[in] function Non-null command-sequence entry point.
     * @param[in] name Stable GoogleTest case and configuration name.
     */
    void runCommandSequence(sequencetestwithargs function, ctrl::stringview name)
    {
        ctrl::errorcode pathError;
        const ctrl::filesystempath executablePath = std::filesystem::read_symlink("/proc/self/exe", pathError);
        ASSERT_FALSE(pathError) << "cannot resolve CLI sequence executable: " << pathError.message();
        ASSERT_FALSE(executablePath.empty());

        const ctrl::filesystempath path = executablePath.parent_path() / "xWalkController" / "xWalkTest" /
                                          "sequence-data" / (ctrl::string(name) + ".conf");
        runIsolated(function, {ctrl::string(name), path.string()});
    }

    TEST(XWalkControllerGroup, ControllerCommands)
    {
        runIsolated(&xWalkControllerSequenceHostTest);
    }

#define XWALK_REGISTER_COMMAND_TEST(Group, Name)                                                                       \
    TEST(Group, Name)                                                                                                  \
    {                                                                                                                  \
        runCommandSequence(&xWalk##Name##CommandSequenceHostTest, #Name);                                              \
    }
    XWALK_REGISTER_COMMAND_TEST(XWalkControllerGroup, Help)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentPlatformGroup, Doctor)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentCalibrationGroup, ServoZeroing)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentCalibrationGroup, Calibration)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, Move)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, KeyboardControl)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, ObstacleAvoidance)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, CliffDetection)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, Turn)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, Sensor)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, LineTracking)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVehicleGroup, SelfDrive)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, ComputerVision)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, FaceTracking)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, BullFight)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, TreasureHunt)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, VideoRecording)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, VideoCar)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVisionGroup, Camera)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentConnectivityGroup, AppControl)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentConnectivityGroup, Spi)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentMediaGroup, SoundBackgroundMusic)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentMediaGroup, Sound)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, VoiceChat)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, VoiceActiveCar)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, VoiceActiveCarGpt)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, GptCar)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, VoiceControlledCar)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, VoicePromptCar)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, StorytellingRobot)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, TextVisionTalk)
    XWALK_REGISTER_COMMAND_TEST(XWalkAgentVoiceGroup, OnlineLlmTest)
#undef XWALK_REGISTER_COMMAND_TEST

} /* namespace */

/**
 * @brief Runs the standalone bounded CLI sequence-test executable.
 * @param[in,out] argumentCount Process argument count consumed by GoogleTest.
 * @param[in,out] argumentValues Non-owning process argument array valid for this call.
 * @return Zero when every selected sequence passes; otherwise a non-zero status.
 */
int main(int argumentCount, char* argumentValues[])
{
    ctrl::errorcode pathError;
    const ctrl::filesystempath executablePath = std::filesystem::read_symlink("/proc/self/exe", pathError);
    const ::ctrl::boolean pathErrorExecutablePathInvalid =
        static_cast<::ctrl::boolean>(pathError || executablePath.empty());
    if (pathErrorExecutablePathInvalid)
    {
        std::cerr << "xCliSequenceTest path error: cannot resolve /proc/self/exe: " << pathError.message() << '\n';
        return EXIT_FAILURE;
    }

    xwalk::agent::test::XWalkCliTestConfig configuration;
    ctrl::string error;
    const ctrl::filesystempath configurationPath = executablePath.parent_path() / "xCliSequenceTestConfig.xml";
    const ctrl::boolean configurationLoaded =
        configuration.load(configurationPath, xwalk::agent::test::availableCliSequenceTests(), error);
    if (configurationLoaded == false)
    {
        std::cerr << "xCliSequenceTest configuration error: " << error << '\n';
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
