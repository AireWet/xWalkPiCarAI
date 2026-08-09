/******************************************************************************
 * @file        xControllerAppTest.cpp
 * @brief       Verifies the device-free xWalk Controller application.
 *
 * @details
 * Launches the host executable in isolated child processes and checks its
 * help, deployment-option validation, and hardware-command rejection.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp GoogleTest
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerCommand.h"
#include "xWalkControllerConfigTypes.h"

#include "xControllerApplicationSupport.h"
#include "xControllerBootMode.h"
#include "xControllerCommands.h"
#include "xControllerPicarxCommands.h"
#include "xControllerRunner.h"

#include "xHal_Rpi5CarTypes.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <type_traits>

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains child-process helpers and application-level test cases.
 */
namespace
{

static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkSoundOperation>, ctrl::uint8>);
static_assert(std::is_same_v<decltype(XWALK_CNTRL_UNKNOWN_REQ), ctrl::uint16>);
static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkLifecycleAction>, ctrl::uint8>);
static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkMoveAction>, ctrl::uint8>);
static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkTurnDirection>, ctrl::uint8>);
static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkCameraAxis>, ctrl::uint8>);
static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkSensorType>, ctrl::uint8>);
static_assert(std::is_same_v<
    std::underlying_type_t<xwalk::ctrl::XWalkCalibrationMode>, ctrl::uint8>);

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/**
 * @brief Verifies that the application free function exposes generated help.
 */
TEST(XWalkAppGroup, ControllerUsageFunction)
{
    const ctrl::string usage = xwalk::ctrl::XWALK_controllerUsage();
    EXPECT_NE(usage.find("Commands:\n"), ctrl::string::npos);
    EXPECT_NE(usage.find("Examples:\n"), ctrl::string::npos);
    EXPECT_NE(usage.find("--trace VALUE"), ctrl::string::npos);
    EXPECT_NE(usage.find("all.enable"), ctrl::string::npos);
    EXPECT_NE(usage.find("Trace IDs must be unique"), ctrl::string::npos);
}

/** @brief Verifies the extracted application callback context defaults. */
TEST(XWalkAppGroup, ApplicationSupportDefaults)
{
    const xwalk::ctrl::XWalkControllerBootContext bootContext;
    const xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
    xwalk::ctrl::XWalkControllerApplicationContext applicationContext;
    const xwalk::ctrl::XWalkSoundRequest soundRequest;

    EXPECT_EQ(bootContext.commandArguments, nullptr);
    EXPECT_TRUE(bootContext.resourceDirectory.empty());
    EXPECT_TRUE(applicationArguments.traceArguments.empty());
    EXPECT_TRUE(xwalk::ctrl::xWalkApplyTraceConfiguration(applicationArguments));
    EXPECT_EQ(applicationContext.music, nullptr);
    EXPECT_TRUE(applicationContext.resourceDirectory.empty());
    EXPECT_FALSE(xwalk::ctrl::XWALK_performSound(
        &applicationContext, soundRequest));
}

/** @brief Verifies reset and signal-stop transitions for the extracted callback state. */
TEST(XWalkAppGroup, ApplicationOperationRequest)
{
    xwalk::ctrl::XWALK_resetOperationRequest();
    EXPECT_TRUE(xwalk::ctrl::XWALK_continueOperation(nullptr));

    xwalk::ctrl::XWALK_requestOperationStop(0);
    EXPECT_FALSE(xwalk::ctrl::XWALK_continueOperation(nullptr));

    xwalk::ctrl::XWALK_resetOperationRequest();
}

/** @brief Verifies the terminal output, input, EOF, and zero-delay callbacks. */
TEST(XWalkAppGroup, ApplicationTerminalCallbacks)
{
    std::ostringstream output;
    std::istringstream input("response\n");
    std::streambuf* const previousOutput = std::cout.rdbuf(output.rdbuf());
    std::streambuf* const previousInput = std::cin.rdbuf(input.rdbuf());

    xwalk::ctrl::XWALK_outputLine(nullptr, "line");
    const ctrl::string response = xwalk::ctrl::XWALK_inputLine(nullptr, "prompt: ");
    const ctrl::string endOfFileResponse = xwalk::ctrl::XWALK_inputLine(nullptr, "again: ");
    xwalk::ctrl::XWALK_delayMilliseconds(nullptr, 0U);

    std::cin.rdbuf(previousInput);
    std::cout.rdbuf(previousOutput);
    EXPECT_EQ(response, "response");
    EXPECT_EQ(endOfFileResponse, "skip");
    EXPECT_EQ(output.str(), "line\nprompt: again: ");
}

/** @brief Verifies every specialized Controller command boot-mode selection. */
TEST(XWalkAppGroup, ControllerBootModes)
{
    struct BootModeTestCase
    {
        ctrl::cstring command;
        xwalk::agent::uint8 mode;
    };
    const ctrl::fixedarray<BootModeTestCase, 23U> bootModeCases{{
        {"line-track", XWALK_BOOT_LINE_TRACKING_REQ},
        {"computer-vision", XWALK_BOOT_COMPUTER_VISION_REQ},
        {"stare-at-you", XWALK_BOOT_FACE_TRACKING_REQ},
        {"bull-fight", XWALK_BOOT_BULL_FIGHT_REQ},
        {"treasure-hunt", XWALK_BOOT_TREASURE_HUNT_REQ},
        {"record-video", XWALK_BOOT_VIDEO_RECORDING_REQ},
        {"video-car", XWALK_BOOT_VIDEO_CAR_REQ},
        {"app-control", XWALK_BOOT_APP_CONTROL_REQ},
        {"sound-background-music", XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ},
        {"doctor", XWALK_BOOT_DOCTOR_REQ},
        {"self-drive", XWALK_BOOT_SELF_DRIVE_REQ},
        {"sound", XWALK_BOOT_SOUND_REQ},
        {"voice-chat", XWALK_BOOT_VOICE_CHAT_REQ},
        {"voice-active-car", XWALK_BOOT_VOICE_ACTIVE_CAR_REQ},
        {"voice-active-car-gpt", XWALK_BOOT_VOICE_ACTIVE_CAR_GPT_REQ},
        {"gpt-car", XWALK_BOOT_GPT_CAR_REQ},
        {"voice-controlled-car", XWALK_BOOT_VOICE_CONTROLLED_CAR_REQ},
        {"voice-prompt-car", XWALK_BOOT_VOICE_PROMPT_CAR_REQ},
        {"storytelling-robot", XWALK_BOOT_STORYTELLING_ROBOT_REQ},
        {"text-vision-talk", XWALK_BOOT_TEXT_VISION_TALK_REQ},
        {"online-llm-test", XWALK_BOOT_ONLINE_LLM_TEST_REQ},
        {"servo-zeroing", XWALK_BOOT_SERVO_ZEROING_REQ},
        {"spi", XWALK_BOOT_SPI_TRANSFER_REQ}
    }};

    for (const BootModeTestCase& testCase : bootModeCases)
    {
        EXPECT_EQ(xwalk::ctrl::XWALK_selectBootMode({testCase.command}),
            testCase.mode);
    }
    EXPECT_EQ(xwalk::ctrl::XWALK_selectBootMode({}),
        XWALK_BOOT_BASE_REQ);
    EXPECT_EQ(xwalk::ctrl::XWALK_selectBootMode({"unsupported-command"}),
        XWALK_BOOT_BASE_REQ);
}

/** @brief Verifies the extracted runner dispatches a boot-owned Doctor report. */
TEST(XWalkAppGroup, ControllerRunnerDoctor)
{
    const ctrl::stringvector commandArguments{"doctor"};
    const ctrl::stringvector doctorLines{"[PASS] host-only runner test"};
    xwalk::ctrl::XWalkControllerBootContext bootContext{
        &commandArguments, {}};
    xwalk::agent::XWalkBootServices services;
    services.doctorLines = &doctorLines;

    EXPECT_EQ(xwalk::ctrl::XWALK_runController(&bootContext, services), 0);
}

/** @brief Verifies the PiCar-X router rejects an empty request before hardware access. */
TEST(XWalkAppGroup, PicarxRouterValidation)
{
    const ctrl::stringvector doctorLines{"[PASS] router validation"};
    xwalk::ctrl::XWalkControllerApplicationContext applicationContext;
    const xwalk::ctrl::XWalkControllerCallbacks callbacks{
        &xwalk::ctrl::XWALK_outputLine,
        &xwalk::ctrl::XWALK_inputLine,
        &xwalk::ctrl::XWALK_delayMilliseconds,
        &xwalk::ctrl::XWALK_continueOperation,
        &xwalk::ctrl::XWALK_performSound};
    xwalk::ctrl::XWalkController controller(
        doctorLines, &applicationContext, callbacks);
    const xwalk::ctrl::XWalkControllerCommandRequest request;

    EXPECT_THROW(xwalk::ctrl::XWALK_runPicarxControllerCommand(controller, request),
        ctrl::invalidargument);
}

/** @brief Verifies the self-contained common header's plain-data defaults. */
TEST(XWalkAppGroup, AgentConfigurationTypeDefaults)
{
    const xwalk::ctrl::XWalkAppConfig appConfig;
    const xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
    const xwalk::ctrl::XWalkControllerCommandRequest commandRequest;
    const xwalk::ctrl::XWalkNoArgumentRequest noArgumentRequest;
    const xwalk::ctrl::XWalkLifecycleRequest lifecycleRequest;
    const xwalk::ctrl::XWalkMoveRequest moveRequest;
    const xwalk::ctrl::XWalkTurnRequest turnRequest;
    const xwalk::ctrl::XWalkCameraRequest cameraRequest;
    const xwalk::ctrl::XWalkSensorRequest sensorRequest;
    const xwalk::ctrl::XWalkSelfDriveRequest selfDriveRequest;
    const xwalk::ctrl::XWalkSpiRequest spiRequest;
    const xwalk::ctrl::XWalkGptCarRequest gptCarRequest;
    const xwalk::ctrl::XWalkCalibrationRequest calibrationRequest;
    const xwalk::ctrl::XWalkSoundRequest soundRequest;
    const xwalk::ctrl::XWalkServoCalibrationConfig servoConfig;
    const ctrl::int32 statusCode{};

    EXPECT_TRUE(appConfig.configurationFilePath.empty());
    EXPECT_TRUE(appConfig.resourceDirectory.empty());
    EXPECT_TRUE(applicationArguments.commandArguments.empty());
    EXPECT_TRUE(applicationArguments.appConfig.configurationFilePath.empty());
    EXPECT_TRUE(applicationArguments.appConfig.resourceDirectory.empty());
    EXPECT_EQ(commandRequest.command, XWALK_CNTRL_UNKNOWN_REQ);
    EXPECT_TRUE(commandRequest.arguments.empty());
    static_cast<void>(noArgumentRequest);
    EXPECT_EQ(lifecycleRequest.action, xwalk::ctrl::XWalkLifecycleAction::Start);
    EXPECT_EQ(moveRequest.action, xwalk::ctrl::XWalkMoveAction::Forward);
    EXPECT_DOUBLE_EQ(moveRequest.speedPercent, 50.0);
    EXPECT_EQ(moveRequest.durationMs, 1'000U);
    EXPECT_EQ(turnRequest.direction, xwalk::ctrl::XWalkTurnDirection::Left);
    EXPECT_DOUBLE_EQ(turnRequest.angleDegrees, 30.0);
    EXPECT_EQ(cameraRequest.axis, xwalk::ctrl::XWalkCameraAxis::Pan);
    EXPECT_DOUBLE_EQ(cameraRequest.angleDegrees, 0.0);
    EXPECT_EQ(sensorRequest.type, xwalk::ctrl::XWalkSensorType::Distance);
    EXPECT_TRUE(selfDriveRequest.action.empty());
    EXPECT_TRUE(spiRequest.transmitData.empty());
    EXPECT_EQ(gptCarRequest.action, xwalk::ctrl::XWalkLifecycleAction::Start);
    EXPECT_FALSE(gptCarRequest.keyboardInput);
    EXPECT_TRUE(gptCarRequest.withImage);
    EXPECT_EQ(calibrationRequest.mode, xwalk::ctrl::XWalkCalibrationMode::Complete);
    EXPECT_EQ(soundRequest.operation, xwalk::ctrl::XWalkSoundOperation::Stop);
    EXPECT_TRUE(soundRequest.filePath.empty());
    EXPECT_FALSE(soundRequest.volumePercent.has_value());
    EXPECT_TRUE(servoConfig.title.empty());
    EXPECT_TRUE(servoConfig.prompt.empty());
    EXPECT_DOUBLE_EQ(servoConfig.minimumAngleDegrees, 0.0);
    EXPECT_DOUBLE_EQ(servoConfig.maximumAngleDegrees, 0.0);
    EXPECT_EQ(servoConfig.servoId, 0U);
    EXPECT_EQ(statusCode, 0);
}

/** @brief Maps every supported top-level spelling to one typed command. */
TEST(XWalkAppGroup, ControllerCommandRequests)
{
    struct CommandTestCase
    {
        ctrl::cstring name;
        ctrl::uint16 command;
    };
    const ctrl::fixedarray<CommandTestCase, 34U> commandCases{{
        {"-h", XWALK_CNTRL_HELP_REQ},
        {"--help", XWALK_CNTRL_HELP_REQ},
        {"help", XWALK_CNTRL_HELP_REQ},
        {"spi", XWALK_CNTRL_SPI_REQ},
        {"doctor", XWALK_CNTRL_DOCTOR_REQ},
        {"servo-zeroing", XWALK_CNTRL_SERVO_ZEROING_REQ},
        {"computer-vision", XWALK_CNTRL_COMPUTER_VISION_REQ},
        {"record-video", XWALK_CNTRL_RECORD_VIDEO_REQ},
        {"sound-background-music", XWALK_CNTRL_SOUND_BACKGROUND_MUSIC_REQ},
        {"text-vision-talk", XWALK_CNTRL_TEXT_VISION_TALK_REQ},
        {"online-llm-test", XWALK_CNTRL_ONLINE_LLM_TEST_REQ},
        {"move", XWALK_CNTRL_MOVE_REQ},
        {"keyboard-control", XWALK_CNTRL_KEYBOARD_CONTROL_REQ},
        {"avoid-obstacles", XWALK_CNTRL_AVOID_OBSTACLES_REQ},
        {"cliff-detection", XWALK_CNTRL_CLIFF_DETECTION_REQ},
        {"stare-at-you", XWALK_CNTRL_STARE_AT_YOU_REQ},
        {"bull-fight", XWALK_CNTRL_BULL_FIGHT_REQ},
        {"treasure-hunt", XWALK_CNTRL_TREASURE_HUNT_REQ},
        {"video-car", XWALK_CNTRL_VIDEO_CAR_REQ},
        {"app-control", XWALK_CNTRL_APP_CONTROL_REQ},
        {"turn", XWALK_CNTRL_TURN_REQ},
        {"cam", XWALK_CNTRL_CAMERA_REQ},
        {"sensor", XWALK_CNTRL_SENSOR_REQ},
        {"line-track", XWALK_CNTRL_LINE_TRACK_REQ},
        {"self-drive", XWALK_CNTRL_SELF_DRIVE_REQ},
        {"sound", XWALK_CNTRL_SOUND_REQ},
        {"voice-chat", XWALK_CNTRL_VOICE_CHAT_REQ},
        {"voice-active-car", XWALK_CNTRL_VOICE_ACTIVE_CAR_REQ},
        {"voice-active-car-gpt", XWALK_CNTRL_VOICE_ACTIVE_CAR_REQ},
        {"gpt-car", XWALK_CNTRL_GPT_CAR_REQ},
        {"voice-controlled-car", XWALK_CNTRL_VOICE_CONTROLLED_CAR_REQ},
        {"voice-prompt-car", XWALK_CNTRL_VOICE_PROMPT_CAR_REQ},
        {"storytelling-robot", XWALK_CNTRL_STORYTELLING_ROBOT_REQ},
        {"calibrate", XWALK_CNTRL_CALIBRATE_REQ}
    }};

    for (const CommandTestCase& commandCase : commandCases)
    {
        const xwalk::ctrl::XWalkControllerCommandRequest request =
            xwalk::ctrl::XWALK_parseControllerCommand(
                {commandCase.name, "test-argument"});
        EXPECT_EQ(request.command, commandCase.command);
        ASSERT_EQ(request.arguments.size(), 2U);
        EXPECT_EQ(request.arguments[0U], commandCase.name);
        EXPECT_EQ(request.arguments[1U], "test-argument");
    }

    const xwalk::ctrl::XWalkControllerCommandRequest unknownRequest =
        xwalk::ctrl::XWALK_parseControllerCommand({"unsupported-command"});
    EXPECT_EQ(unknownRequest.command,
        XWALK_CNTRL_UNKNOWN_REQ);
    EXPECT_THROW(xwalk::ctrl::XWALK_parseControllerCommand({}),
        std::invalid_argument);
}

/** @brief Verifies aggregate construction of validated boundary requests. */
TEST(XWalkAppGroup, AgentConfigurationTypeConstruction)
{
    const xwalk::ctrl::XWalkMoveRequest moveRequest{
        xwalk::ctrl::XWalkMoveAction::Backward, 75.0, 2'500U};
    const xwalk::ctrl::XWalkTurnRequest turnRequest{
        xwalk::ctrl::XWalkTurnDirection::Right, 20.0};
    const xwalk::ctrl::XWalkCameraRequest cameraRequest{
        xwalk::ctrl::XWalkCameraAxis::Tilt, -15.0};
    const xwalk::ctrl::XWalkSensorRequest sensorRequest{
        xwalk::ctrl::XWalkSensorType::Grayscale};
    const xwalk::ctrl::XWalkSpiRequest spiRequest{{0x12U, 0xABU}};
    const xwalk::ctrl::XWalkGptCarRequest gptCarRequest{
        xwalk::ctrl::XWalkLifecycleAction::Start, true, false};
    const xwalk::ctrl::XWalkSoundRequest soundRequest{
        xwalk::ctrl::XWalkSoundOperation::Volume, {}, 100.0};
    const xwalk::ctrl::XWalkServoCalibrationConfig servoConfig{
        "Steering", "Offset: ", -20.0, 20.0, 0U};

    EXPECT_EQ(moveRequest.action, xwalk::ctrl::XWalkMoveAction::Backward);
    EXPECT_DOUBLE_EQ(moveRequest.speedPercent, 75.0);
    EXPECT_EQ(moveRequest.durationMs, 2'500U);
    EXPECT_EQ(turnRequest.direction, xwalk::ctrl::XWalkTurnDirection::Right);
    EXPECT_DOUBLE_EQ(turnRequest.angleDegrees, 20.0);
    EXPECT_EQ(cameraRequest.axis, xwalk::ctrl::XWalkCameraAxis::Tilt);
    EXPECT_DOUBLE_EQ(cameraRequest.angleDegrees, -15.0);
    EXPECT_EQ(sensorRequest.type, xwalk::ctrl::XWalkSensorType::Grayscale);
    ASSERT_EQ(spiRequest.transmitData.size(), 2U);
    EXPECT_EQ(spiRequest.transmitData[1U], 0xABU);
    EXPECT_TRUE(gptCarRequest.keyboardInput);
    EXPECT_FALSE(gptCarRequest.withImage);
    ASSERT_TRUE(soundRequest.volumePercent.has_value());
    EXPECT_DOUBLE_EQ(*soundRequest.volumePercent, 100.0);
    EXPECT_DOUBLE_EQ(servoConfig.minimumAngleDegrees, -20.0);
    EXPECT_DOUBLE_EQ(servoConfig.maximumAngleDegrees, 20.0);
}

/**
 * @brief Verifies shared host and hardware application-argument parsing.
 */
TEST(XWalkAppGroup, ControllerApplicationArguments)
{
    ctrl::string executable{"xwalk-picarx-control"};
    ctrl::string configurationOption{"--deployment-config"};
    ctrl::string configurationPath{"/var/lib/xwalk/picar-x.conf"};
    ctrl::string resourceOption{"--resource-directory=/usr/share/xwalk"};
    ctrl::string traceEnableOption{"--trace"};
    ctrl::string traceEnableSelector{"RPI.001.enable"};
    ctrl::string traceDisableOption{"--trace=CTRL.2001.disable"};
    ctrl::string traceAllOption{"--trace"};
    ctrl::string traceAllSelector{"all.enable"};
    ctrl::string command{"help"};
    ctrl::charpointer arguments[]{executable.data(), configurationOption.data(),
        configurationPath.data(), resourceOption.data(), traceEnableOption.data(),
        traceEnableSelector.data(), traceDisableOption.data(), traceAllOption.data(),
        traceAllSelector.data(), command.data()};
    xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
    const xwalk::ctrl::XWalkAppConfig defaultConfig{
        "/default/config", "/default/resources"};

    EXPECT_TRUE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
        10, arguments, defaultConfig, applicationArguments));
    EXPECT_EQ(applicationArguments.appConfig.configurationFilePath, configurationPath);
    EXPECT_EQ(applicationArguments.appConfig.resourceDirectory, "/usr/share/xwalk");
    ASSERT_EQ(applicationArguments.traceArguments.size(), 3U);
    EXPECT_EQ(applicationArguments.traceArguments[0U], "RPI.001.enable");
    EXPECT_EQ(applicationArguments.traceArguments[1U], "CTRL.2001.disable");
    EXPECT_EQ(applicationArguments.traceArguments[2U], "all.enable");
    ASSERT_EQ(applicationArguments.commandArguments.size(), 1U);
    EXPECT_EQ(applicationArguments.commandArguments[0U], command);
    EXPECT_TRUE(xwalk::ctrl::XWALK_isControllerHelpRequest(
        applicationArguments.commandArguments));
    EXPECT_FALSE(xwalk::ctrl::XWALK_isControllerHelpRequest({}));
    EXPECT_FALSE(xwalk::ctrl::XWALK_isControllerHelpRequest({"help", "extra"}));
    EXPECT_FALSE(xwalk::ctrl::XWALK_isControllerHelpRequest({"move"}));
}

/**
 * @brief Verifies rejection of a relative global path in shared parsing.
 */
TEST(XWalkAppGroup, InvalidControllerApplicationArguments)
{
    ctrl::string executable{"xwalk-picarx-control"};
    ctrl::string option{"--resource-directory"};
    ctrl::string relativePath{"relative/resources"};
    ctrl::string command{"help"};
    ctrl::charpointer arguments[]{executable.data(), option.data(),
        relativePath.data(), command.data()};
    xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
    const xwalk::ctrl::XWalkAppConfig defaultConfig{
        "/default/config", "/default/resources"};

    EXPECT_FALSE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
        4, arguments, defaultConfig, applicationArguments));

    ctrl::string traceOption{"--trace"};
    ctrl::string invalidUid{"RPI.Camera.enable"};
    ctrl::charpointer traceArguments[]{executable.data(), traceOption.data(),
        invalidUid.data(), command.data()};
    EXPECT_FALSE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
        4, traceArguments, defaultConfig, applicationArguments));

    ctrl::string invalidOperation{"RPI.001.start"};
    ctrl::charpointer invalidOperationArguments[]{executable.data(), traceOption.data(),
        invalidOperation.data(), command.data()};
    EXPECT_FALSE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
        4, invalidOperationArguments, defaultConfig, applicationArguments));

    ctrl::string legacyOption{"--trace-enable"};
    ctrl::string legacyUid{"RPI.001"};
    ctrl::charpointer legacyArguments[]{executable.data(), legacyOption.data(),
        legacyUid.data(), command.data()};
    EXPECT_TRUE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
        4, legacyArguments, defaultConfig, applicationArguments));
}

/**
 * @brief Runs the sibling host application with up to three arguments.
 *
 * @param[in] firstArgument Non-null first process argument.
 * @param[in] secondArgument Optional second process argument.
 * @param[in] thirdArgument Optional third process argument.
 *
 * @return
 * The child exit status, or minus one when process setup or collection fails.
 *
 * @post
 * The child process has terminated and no physical hardware has been accessed.
 */
int runHostApplication(ctrl::cstring firstArgument,
    ctrl::cstring secondArgument = nullptr,
    ctrl::cstring thirdArgument = nullptr)
{
    ctrl::errorcode pathError;
    const ctrl::filesystempath testExecutable =
        std::filesystem::read_symlink("/proc/self/exe", pathError);
    const ::ctrl::boolean pathErrorTestExecutableInvalid =
        static_cast<::ctrl::boolean>(
            pathError || testExecutable.empty());
    if (pathErrorTestExecutableInvalid)
    {
        return -1;
    }
    const ctrl::filesystempath applicationExecutable =
        testExecutable.parent_path() / "xwalk-picarx-control";
    const pid_t childProcess = ::fork();
    if (childProcess < 0)
    {
        return -1;
    }
    if (childProcess == 0)
    {
        const int nullDescriptor = ::open("/dev/null", O_WRONLY);
        if (nullDescriptor < 0)
        {
            ::_exit(126);
        }
        const ::ctrl::boolean nullDescriptorInvalid =
            static_cast<::ctrl::boolean>(
                (::dup2(nullDescriptor, STDOUT_FILENO) < 0) ||
            (::dup2(nullDescriptor, STDERR_FILENO) < 0));
        if (nullDescriptorInvalid)
        {
            ::_exit(126);
        }
        static_cast<void>(::close(nullDescriptor));
        if (thirdArgument != nullptr)
        {
            ::execl(applicationExecutable.c_str(), applicationExecutable.c_str(),
                firstArgument, secondArgument, thirdArgument,
                static_cast<char*>(nullptr));
        }
        else if (secondArgument != nullptr)
        {
            ::execl(applicationExecutable.c_str(), applicationExecutable.c_str(),
                firstArgument, secondArgument, static_cast<char*>(nullptr));
        }
        else
        {
            ::execl(applicationExecutable.c_str(), applicationExecutable.c_str(),
                firstArgument, static_cast<char*>(nullptr));
        }
        ::_exit(127);
    }

    int childStatus{};
    const ::ctrl::boolean childProcessChildStatusDifferent =
        static_cast<::ctrl::boolean>(
            ::waitpid(childProcess, &childStatus, 0) != childProcess);
    if (childProcessChildStatusDifferent)
    {
        return -1;
    }
    const ::ctrl::boolean childExitedAbnormally =
        static_cast<::ctrl::boolean>(
            !WIFEXITED(childStatus));
    if (childExitedAbnormally)
    {
        return -1;
    }
    return WEXITSTATUS(childStatus);
}

/**
 * @brief Verifies that the host application accepts the generated help command.
 */
TEST(XWalkAppGroup, Help)
{
    EXPECT_EQ(runHostApplication("--help"), 0);
}

/**
 * @brief Verifies unified all-trace control without constructing hardware.
 */
TEST(XWalkAppGroup, TraceConfiguration)
{
    EXPECT_EQ(runHostApplication("--trace", "all.disable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "all.enable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "RPI.enable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "RPI.disable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "CTRL.enable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "CTRL.disable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "RPI.001.enable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "RPI.001.disable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "CTRL.001.enable"), 0);
    EXPECT_EQ(runHostApplication("--trace", "CTRL.001.disable"), 0);
    const ctrl::filesystempath jsonExample =
        ctrl::filesystempath(XWALK_CONTROLLER_TRACE_EXAMPLE_DIRECTORY) /
        "xwalk-traces.json";
    EXPECT_EQ(runHostApplication("--trace", jsonExample.c_str()), 0);
    EXPECT_EQ(runHostApplication("--trace", "RPI.99999.enable"), 2);
    EXPECT_EQ(runHostApplication("--trace", "UNKNOWN.enable"), 2);
    EXPECT_EQ(runHostApplication("--trace", "all.true"), 2);
    EXPECT_EQ(runHostApplication("--trace", "all.001.enable"), 2);
    EXPECT_EQ(runHostApplication("--trace", "CTRL..enable"), 2);
}

/**
 * @brief Verifies rejection of a relative deployment-configuration path.
 */
TEST(XWalkAppGroup, InvalidDeploymentConfiguration)
{
    EXPECT_EQ(runHostApplication(
        "--deployment-config", "relative.conf", "--help"), 2);
}

/**
 * @brief Verifies that host execution rejects a hardware command.
 */
TEST(XWalkAppGroup, HardwareCommandUnavailable)
{
    EXPECT_EQ(runHostApplication("move", "forward"), 3);
}

} /* namespace */
