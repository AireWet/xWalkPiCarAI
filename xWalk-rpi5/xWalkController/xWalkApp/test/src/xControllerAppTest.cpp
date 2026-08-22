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
#include "xControllerDeploymentConfig.h"
#include "xControllerBootMode.h"
#include "xControllerCommands.h"
#include "xControllerPicarxCommands.h"
#include "xControllerRunner.h"
#include "xControllerAppTestSupport.h"

#include "xHal_Rpi5CarTypes.h"
#include "xHal_Rpi5CarCameraStream.h"

#include "xAgent_Rpi5CarVideoStreaming.h"
#include "xAgent_Rpi5CarBootTypes.h"

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <type_traits>

#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>
#include "xControllerAppTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using BootModeTestCase = ::xwalk::source_types::xcontrollerapptest::BootModeTestCase;
using CommandTestCase = ::xwalk::source_types::xcontrollerapptest::CommandTestCase;
using InvalidOverride = ::xwalk::source_types::xcontrollerapptest::InvalidOverride;
using ValidOverride = ::xwalk::source_types::xcontrollerapptest::ValidOverride;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains child-process helpers and application-level test cases.
 */
namespace
{

    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkSoundOperation>, ctrl::uint8>);
    static_assert(std::is_same_v<decltype(XWALK_CNTRL_UNKNOWN_REQ), ctrl::uint16>);
    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkLifecycleAction>, ctrl::uint8>);
    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkMoveAction>, ctrl::uint8>);
    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkTurnDirection>, ctrl::uint8>);
    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkCameraAxis>, ctrl::uint8>);
    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkSensorType>, ctrl::uint8>);
    static_assert(std::is_same_v<std::underlying_type_t<xwalk::ctrl::XWalkCalibrationMode>, ctrl::uint8>);

    /**
     * @brief Validates one deployment override while retaining production defaults.
     * @param[in] name Configuration key written to the isolated fixture.
     * @param[in] value Configuration value written to the isolated fixture.
     * @return Complete validation report for the temporary configuration.
     */
    xwalk::ctrl::XWalkDeploymentConfigReport validateDeploymentOverride(ctrl::stringview name, ctrl::stringview value)
    {
        const ctrl::filesystempath path =
            ctrl::filesystempath("/tmp") /
            ("xwalk-deployment-override-" + std::to_string(static_cast<unsigned long>(::getpid())) + "-" +
             ctrl::string(name) + ".conf");
        {
            std::ofstream output(path);
            const ctrl::boolean outputOpen = output.is_open();
            if (!outputOpen)
            {
                return {false, {"[FAIL] temporary configuration could not be opened"}};
            }
            output << name << " = " << value << '\n';
        }
        const xwalk::ctrl::XWalkDeploymentConfigReport report =
            xwalk::ctrl::XWALK_validateDeploymentConfig(path.string());
        std::error_code removeError;
        static_cast<void>(std::filesystem::remove(path, removeError));
        return report;
    }

    /** @brief Validates one paired video-stream backend and source selection. */
    xwalk::ctrl::XWalkDeploymentConfigReport validateVideoStreamSelection(ctrl::stringview backend,
                                                                          ctrl::stringview device)
    {
        const ctrl::filesystempath path =
            ctrl::filesystempath("/tmp") /
            ("xwalk-video-stream-selection-" + std::to_string(static_cast<unsigned long>(::getpid())) + ".conf");
        {
            std::ofstream output(path);
            const ctrl::boolean outputOpen = output.is_open();
            if (!outputOpen)
            {
                return {false, {"[FAIL] temporary configuration could not be opened"}};
            }
            output << "video_stream_camera_backend = " << backend << '\n';
            output << "video_stream_camera_device = " << device << '\n';
        }
        const xwalk::ctrl::XWalkDeploymentConfigReport report =
            xwalk::ctrl::XWALK_validateDeploymentConfig(path.string());
        std::error_code removeError;
        static_cast<void>(std::filesystem::remove(path, removeError));
        return report;
    }

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
        const xwalk::ctrl::XWalkRunArgs bootContext;
        const xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
        xwalk::ctrl::XWalkControllerApplicationContext applicationContext;
        const xwalk::ctrl::XWalkSoundRequest soundRequest;

        EXPECT_EQ(bootContext.commandArguments, nullptr);
        EXPECT_TRUE(bootContext.resourceDirectory.empty());
        EXPECT_TRUE(applicationArguments.traceArguments.empty());
        EXPECT_FALSE(applicationArguments.validateConfiguration);
        EXPECT_FALSE(applicationArguments.printEffectiveConfiguration);
        EXPECT_FALSE(applicationArguments.diagnose);
        EXPECT_FALSE(applicationArguments.noHardware);
        EXPECT_TRUE(xwalk::ctrl::xWalkApplyTraceConfiguration(applicationArguments));
        EXPECT_EQ(applicationContext.music, nullptr);
        EXPECT_TRUE(applicationContext.resourceDirectory.empty());
        EXPECT_FALSE(xwalk::ctrl::XWALK_performSound(&applicationContext, soundRequest));
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

    /** @brief Verifies that installed cancellation handlers interrupt input and request graceful shutdown. */
    TEST(XWalkAppGroup, ApplicationSignalHandlers)
    {
        struct sigaction previousInterruptAction
        {
        };
        struct sigaction previousTerminationAction
        {
        };
        sigset_t previousSignalMask;
        ASSERT_EQ(::sigaction(SIGINT, nullptr, &previousInterruptAction), 0);
        ASSERT_EQ(::sigaction(SIGTERM, nullptr, &previousTerminationAction), 0);
        ASSERT_EQ(::pthread_sigmask(SIG_SETMASK, nullptr, &previousSignalMask), 0);

        xwalk::ctrl::XWALK_resetOperationRequest();
        EXPECT_TRUE(xwalk::ctrl::XWALK_prepareOperationSignalHandling());
        sigset_t preparedSignalMask;
        EXPECT_EQ(::pthread_sigmask(SIG_SETMASK, nullptr, &preparedSignalMask), 0);
        EXPECT_EQ(::sigismember(&preparedSignalMask, SIGINT), 1);
        EXPECT_EQ(::sigismember(&preparedSignalMask, SIGTERM), 1);
        EXPECT_TRUE(xwalk::ctrl::XWALK_activateOperationSignalHandling());
        sigset_t activeSignalMask;
        EXPECT_EQ(::pthread_sigmask(SIG_SETMASK, nullptr, &activeSignalMask), 0);
        EXPECT_EQ(::sigismember(&activeSignalMask, SIGINT), 0);
        EXPECT_EQ(::sigismember(&activeSignalMask, SIGTERM), 0);
        struct sigaction installedInterruptAction
        {
        };
        struct sigaction installedTerminationAction
        {
        };
        EXPECT_EQ(::sigaction(SIGINT, nullptr, &installedInterruptAction), 0);
        EXPECT_EQ(::sigaction(SIGTERM, nullptr, &installedTerminationAction), 0);
        EXPECT_EQ(installedInterruptAction.sa_flags & SA_RESTART, 0);
        EXPECT_EQ(installedTerminationAction.sa_flags & SA_RESTART, 0);
        EXPECT_EQ(::raise(SIGINT), 0);
        EXPECT_FALSE(xwalk::ctrl::XWALK_continueOperation(nullptr));

        EXPECT_EQ(::sigaction(SIGINT, &previousInterruptAction, nullptr), 0);
        EXPECT_EQ(::sigaction(SIGTERM, &previousTerminationAction, nullptr), 0);
        EXPECT_EQ(::pthread_sigmask(SIG_SETMASK, &previousSignalMask, nullptr), 0);
        xwalk::ctrl::XWALK_resetOperationRequest();
    }

    /** @brief Verifies that SIGINT interrupts a blocked terminal read and returns through normal cleanup. */
    TEST(XWalkAppGroup, ApplicationSignalInterruptsInput)
    {
        int inputPipe[2]{};
        int readyPipe[2]{};
        ASSERT_EQ(::pipe(inputPipe), 0);
        ASSERT_EQ(::pipe(readyPipe), 0);
        const pid_t childProcess = ::fork();
        ASSERT_GE(childProcess, 0);
        if (childProcess == 0)
        {
            static_cast<void>(::close(inputPipe[1]));
            static_cast<void>(::close(readyPipe[0]));
            const ctrl::int32 duplicateResult = ::dup2(inputPipe[0], STDIN_FILENO);
            if (duplicateResult < 0)
            {
                ::_exit(2);
            }
            xwalk::ctrl::XWALK_resetOperationRequest();
            const ctrl::boolean signalHandlingPrepared = xwalk::ctrl::XWALK_prepareOperationSignalHandling();
            const ctrl::boolean signalHandlingActivated = xwalk::ctrl::XWALK_activateOperationSignalHandling();
            if ((!signalHandlingPrepared) || (!signalHandlingActivated))
            {
                ::_exit(3);
            }
            const char ready{'1'};
            const ssize_t writtenSize = ::write(readyPipe[1], &ready, 1U);
            if (writtenSize != 1)
            {
                ::_exit(4);
            }
            const ctrl::string response = xwalk::ctrl::XWALK_inputLine(nullptr, "blocked> ");
            const ctrl::boolean cancelled = static_cast<ctrl::boolean>(!xwalk::ctrl::XWALK_continueOperation(nullptr));
            ::_exit(((response == "skip") && cancelled) ? 0 : 5);
        }

        static_cast<void>(::close(inputPipe[0]));
        static_cast<void>(::close(readyPipe[1]));
        char ready{};
        ASSERT_EQ(::read(readyPipe[0], &ready, 1U), 1);
        EXPECT_EQ(::kill(childProcess, SIGINT), 0);
        static_cast<void>(::close(inputPipe[1]));
        static_cast<void>(::close(readyPipe[0]));
        int childStatus{};
        ASSERT_EQ(::waitpid(childProcess, &childStatus, 0), childProcess);
        EXPECT_TRUE(WIFEXITED(childStatus));
        EXPECT_EQ(WEXITSTATUS(childStatus), 0);
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
        const ctrl::uint64 monotonicStartMs = xwalk::ctrl::XWALK_monotonicMilliseconds(nullptr);
        const ctrl::uint64 monotonicEndMs = xwalk::ctrl::XWALK_monotonicMilliseconds(nullptr);

        std::cin.rdbuf(previousInput);
        std::cout.rdbuf(previousOutput);
        EXPECT_EQ(response, "response");
        EXPECT_EQ(endOfFileResponse, "skip");
        EXPECT_EQ(output.str(), "line\nprompt: again: ");
        EXPECT_LE(monotonicStartMs, monotonicEndMs);
    }

    /** @brief Verifies every specialized Controller command boot-mode selection. */
    TEST(XWalkAppGroup, ControllerBootModes)
    {
        const ctrl::fixedarray<BootModeTestCase, 24U> bootModeCases{
            {{"line-track", XWALK_BOOT_LINE_TRACKING_REQ},
             {"computer-vision", XWALK_BOOT_COMPUTER_VISION_REQ},
             {"stare-at-you", XWALK_BOOT_FACE_TRACKING_REQ},
             {"bull-fight", XWALK_BOOT_BULL_FIGHT_REQ},
             {"treasure-hunt", XWALK_BOOT_TREASURE_HUNT_REQ},
             {"record-video", XWALK_BOOT_VIDEO_RECORDING_REQ},
             {"video-car", XWALK_BOOT_VIDEO_CAR_REQ},
             {"video-stream", XWALK_BOOT_VIDEO_STREAMING_REQ},
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
             {"spi", XWALK_BOOT_SPI_TRANSFER_REQ}}};

        for (const BootModeTestCase& testCase : bootModeCases)
        {
            EXPECT_EQ(xwalk::ctrl::XWALK_selectBootMode({testCase.command}), testCase.mode);
        }
        EXPECT_EQ(xwalk::ctrl::XWALK_selectBootMode({}), XWALK_BOOT_BASE_REQ);
        EXPECT_EQ(xwalk::ctrl::XWALK_selectBootMode({"unsupported-command"}), XWALK_BOOT_BASE_REQ);
    }

    /** @brief Verifies the extracted runner dispatches a boot-owned Doctor report. */
    TEST(XWalkAppGroup, ControllerRunnerDoctor)
    {
        const ctrl::stringvector commandArguments{"doctor"};
        const ctrl::stringvector doctorLines{"[PASS] host-only runner test"};
        xwalk::ctrl::XWalkRunArgs bootContext{&commandArguments, {}};
        xwalk::agent::XWalkBootServices services;
        services.doctorLines = &doctorLines;

        EXPECT_EQ(xwalk::ctrl::XWALK_runController(&bootContext, services), 0);
    }

    /** @brief Verifies video streaming dispatch does not require a PiCar-X vehicle backend. */
    TEST(XWalkAppGroup, VideoStreamingDispatchBeforePicarxRequirement)
    {
        const ctrl::stringvector doctorLines{"[PASS] camera-only dispatch test"};
        xwalk::ctrl::XWalkControllerApplicationContext applicationContext;
        const xwalk::ctrl::XWalkControllerCallbacks callbacks{&xwalk::ctrl::XWALK_outputLine,
                                                              &xwalk::ctrl::XWALK_inputLine,
                                                              &xwalk::ctrl::XWALK_delayMilliseconds,
                                                              &xwalk::ctrl::XWALK_monotonicMilliseconds,
                                                              &xwalk::ctrl::XWALK_continueOperation,
                                                              &xwalk::ctrl::XWALK_performSound};
        xwalk::ctrl::XWalkController controller(doctorLines, &applicationContext, callbacks);
        std::ostringstream output;
        std::streambuf* const previousOutput = std::cout.rdbuf(output.rdbuf());

        const ctrl::int32 status = xwalk::ctrl::XWALK_runControllerCommand(controller, {"video-stream"});

        std::cout.rdbuf(previousOutput);
        EXPECT_EQ(status, 3);
        EXPECT_EQ(output.str().find("PiCar-X backend unavailable"), ctrl::string::npos);
    }

    /** @brief Verifies camera-only cancellation stops cleanly without a PiCar-X emergency-stop target. */
    TEST(XWalkAppGroup, VideoStreamingCancellationWithoutVehicle)
    {
        xwalk::ctrl::test::app::CameraOnlyStreamingTestState state;
        xwalk::hal::XWalkCameraStream camera(&state, xwalk::ctrl::test::app::cameraOnlyStreamingCallbacks());
        xwalk::agent::XWalkMjpegHttpConfiguration configuration;
        xwalk::agent::XWalkVideoStreaming streaming(
            camera, &xwalk::ctrl::test::app::cameraOnlyStreamingClock, configuration);
        xwalk::ctrl::XWalkControllerApplicationContext applicationContext;
        const xwalk::ctrl::XWalkControllerCallbacks callbacks{&xwalk::ctrl::XWALK_outputLine,
                                                              &xwalk::ctrl::XWALK_inputLine,
                                                              &xwalk::ctrl::XWALK_delayMilliseconds,
                                                              &xwalk::ctrl::XWALK_monotonicMilliseconds,
                                                              &xwalk::ctrl::XWALK_continueOperation,
                                                              &xwalk::ctrl::XWALK_performSound};
        xwalk::ctrl::test::app::CameraOnlyControllerProbe controller(streaming, &applicationContext, callbacks);

        xwalk::ctrl::XWALK_resetOperationRequest();
        xwalk::ctrl::XWALK_requestOperationStop(0);
        const ctrl::boolean operationContinues = controller.operationMayContinueForTest();
        xwalk::ctrl::XWALK_resetOperationRequest();

        EXPECT_FALSE(operationContinues);
        EXPECT_FALSE(state.cameraStarted);
        EXPECT_EQ(state.stopCount, 0U);
    }

    /** @brief Verifies the PiCar-X router rejects an empty request before hardware access. */
    TEST(XWalkAppGroup, PicarxRouterValidation)
    {
        const ctrl::stringvector doctorLines{"[PASS] router validation"};
        xwalk::ctrl::XWalkControllerApplicationContext applicationContext;
        const xwalk::ctrl::XWalkControllerCallbacks callbacks{&xwalk::ctrl::XWALK_outputLine,
                                                              &xwalk::ctrl::XWALK_inputLine,
                                                              &xwalk::ctrl::XWALK_delayMilliseconds,
                                                              &xwalk::ctrl::XWALK_monotonicMilliseconds,
                                                              &xwalk::ctrl::XWALK_continueOperation,
                                                              &xwalk::ctrl::XWALK_performSound};
        xwalk::ctrl::XWalkController controller(doctorLines, &applicationContext, callbacks);
        const xwalk::ctrl::XWalkControllerCommandRequest request;

        EXPECT_THROW(xwalk::ctrl::XWALK_runPicarxControllerCommand(controller, request), ctrl::invalidargument);
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
        const ctrl::fixedarray<CommandTestCase, 35U> commandCases{
            {{"-h", XWALK_CNTRL_HELP_REQ},
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
             {"video-stream", XWALK_CNTRL_VIDEO_STREAM_REQ},
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
             {"calibrate", XWALK_CNTRL_CALIBRATE_REQ}}};

        for (const CommandTestCase& commandCase : commandCases)
        {
            const xwalk::ctrl::XWalkControllerCommandRequest request =
                xwalk::ctrl::XWALK_parseControllerCommand({commandCase.name, "test-argument"});
            EXPECT_EQ(request.command, commandCase.command);
            ASSERT_EQ(request.arguments.size(), 2U);
            EXPECT_EQ(request.arguments[0U], commandCase.name);
            EXPECT_EQ(request.arguments[1U], "test-argument");
        }

        const xwalk::ctrl::XWalkControllerCommandRequest unknownRequest =
            xwalk::ctrl::XWALK_parseControllerCommand({"unsupported-command"});
        EXPECT_EQ(unknownRequest.command, XWALK_CNTRL_UNKNOWN_REQ);
        EXPECT_THROW(xwalk::ctrl::XWALK_parseControllerCommand({}), std::invalid_argument);
    }

    /** @brief Verifies aggregate construction of validated boundary requests. */
    TEST(XWalkAppGroup, AgentConfigurationTypeConstruction)
    {
        const xwalk::ctrl::XWalkMoveRequest moveRequest{xwalk::ctrl::XWalkMoveAction::Backward, 75.0, 2'500U};
        const xwalk::ctrl::XWalkTurnRequest turnRequest{xwalk::ctrl::XWalkTurnDirection::Right, 20.0};
        const xwalk::ctrl::XWalkCameraRequest cameraRequest{xwalk::ctrl::XWalkCameraAxis::Tilt, -15.0};
        const xwalk::ctrl::XWalkSensorRequest sensorRequest{xwalk::ctrl::XWalkSensorType::Grayscale};
        const xwalk::ctrl::XWalkSpiRequest spiRequest{{0x12U, 0xABU}};
        const xwalk::ctrl::XWalkGptCarRequest gptCarRequest{xwalk::ctrl::XWalkLifecycleAction::Start, true, false};
        const xwalk::ctrl::XWalkSoundRequest soundRequest{xwalk::ctrl::XWalkSoundOperation::Volume, {}, 100.0};
        const xwalk::ctrl::XWalkServoCalibrationConfig servoConfig{"Steering", "Offset: ", -20.0, 20.0, 0U};

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
        ctrl::string configurationPath{"../build-rpi/runtime/picar-x.conf"};
        ctrl::string resourceOption{"--resource-directory=./xWalkAudioResources"};
        ctrl::string traceEnableOption{"--trace"};
        ctrl::string traceEnableSelector{"RPI.001.enable"};
        ctrl::string traceDisableOption{"--trace=CTRL.2001.disable"};
        ctrl::string traceAllOption{"--trace"};
        ctrl::string traceAllSelector{"all.enable"};
        ctrl::string command{"help"};
        ctrl::charpointer arguments[]{executable.data(),
                                      configurationOption.data(),
                                      configurationPath.data(),
                                      resourceOption.data(),
                                      traceEnableOption.data(),
                                      traceEnableSelector.data(),
                                      traceDisableOption.data(),
                                      traceAllOption.data(),
                                      traceAllSelector.data(),
                                      command.data()};
        xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
        const xwalk::ctrl::XWalkAppConfig defaultConfig{"/default/config", "/default/resources"};

        EXPECT_TRUE(
            xwalk::ctrl::xWalkParseControllerApplicationArguments(10, arguments, defaultConfig, applicationArguments));
        EXPECT_EQ(applicationArguments.appConfig.configurationFilePath, configurationPath);
        EXPECT_EQ(applicationArguments.appConfig.resourceDirectory, "./xWalkAudioResources");
        ASSERT_EQ(applicationArguments.traceArguments.size(), 3U);
        EXPECT_EQ(applicationArguments.traceArguments[0U], "RPI.001.enable");
        EXPECT_EQ(applicationArguments.traceArguments[1U], "CTRL.2001.disable");
        EXPECT_EQ(applicationArguments.traceArguments[2U], "all.enable");
        ASSERT_EQ(applicationArguments.commandArguments.size(), 1U);
        EXPECT_EQ(applicationArguments.commandArguments[0U], command);
        EXPECT_TRUE(xwalk::ctrl::XWALK_isControllerHelpRequest(applicationArguments.commandArguments));
        EXPECT_FALSE(xwalk::ctrl::XWALK_isControllerHelpRequest({}));
        EXPECT_FALSE(xwalk::ctrl::XWALK_isControllerHelpRequest({"help", "extra"}));
        EXPECT_FALSE(xwalk::ctrl::XWALK_isControllerHelpRequest({"move"}));
    }

    /**
     * @brief Verifies rejection of incomplete global options in shared parsing.
     */
    TEST(XWalkAppGroup, InvalidControllerApplicationArguments)
    {
        ctrl::string executable{"xwalk-picarx-control"};
        ctrl::string option{"--resource-directory="};
        ctrl::string command{"help"};
        ctrl::charpointer arguments[]{executable.data(), option.data(), command.data()};
        xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
        const xwalk::ctrl::XWalkAppConfig defaultConfig{"/default/config", "/default/resources"};

        EXPECT_FALSE(
            xwalk::ctrl::xWalkParseControllerApplicationArguments(3, arguments, defaultConfig, applicationArguments));

        ctrl::string traceOption{"--trace"};
        ctrl::string invalidUid{"RPI.Camera.enable"};
        ctrl::charpointer traceArguments[]{executable.data(), traceOption.data(), invalidUid.data(), command.data()};
        EXPECT_FALSE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
            4, traceArguments, defaultConfig, applicationArguments));

        ctrl::string invalidOperation{"RPI.001.start"};
        ctrl::charpointer invalidOperationArguments[]{
            executable.data(), traceOption.data(), invalidOperation.data(), command.data()};
        EXPECT_FALSE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
            4, invalidOperationArguments, defaultConfig, applicationArguments));

        ctrl::string legacyOption{"--trace-enable"};
        ctrl::string legacyUid{"RPI.001"};
        ctrl::charpointer legacyArguments[]{executable.data(), legacyOption.data(), legacyUid.data(), command.data()};
        EXPECT_TRUE(xwalk::ctrl::xWalkParseControllerApplicationArguments(
            4, legacyArguments, defaultConfig, applicationArguments));
    }

    /** @brief Verifies standalone no-hardware global options are removed before command parsing. */
    TEST(XWalkAppGroup, NoHardwareApplicationArguments)
    {
        ctrl::string executable{"xwalk-picarx-control"};
        ctrl::string validate{"--validate-config"};
        ctrl::string print{"--print-effective-config"};
        ctrl::string diagnose{"--diagnose"};
        ctrl::string noHardware{"--no-hardware"};
        ctrl::charpointer arguments[]{
            executable.data(), validate.data(), print.data(), diagnose.data(), noHardware.data()};
        xwalk::ctrl::XWalkControllerApplicationArguments applicationArguments;
        const xwalk::ctrl::XWalkAppConfig defaultConfig{"/absolute/config.conf", "/absolute/resources"};

        EXPECT_TRUE(
            xwalk::ctrl::xWalkParseControllerApplicationArguments(5, arguments, defaultConfig, applicationArguments));
        EXPECT_TRUE(applicationArguments.commandArguments.empty());
        EXPECT_TRUE(applicationArguments.validateConfiguration);
        EXPECT_TRUE(applicationArguments.printEffectiveConfiguration);
        EXPECT_TRUE(applicationArguments.diagnose);
        EXPECT_TRUE(applicationArguments.noHardware);
    }

    /** @brief Verifies layered validation and redacted effective output without device access. */
    TEST(XWalkAppGroup, NoHardwareDeploymentConfiguration)
    {
        const ctrl::filesystempath configurationDirectory(XWALK_CONTROLLER_TRACE_EXAMPLE_DIRECTORY);
        const ctrl::filesystempath repositoryConfiguration = configurationDirectory / "picar-x.conf";
        const xwalk::ctrl::XWalkDeploymentConfigReport validReport =
            xwalk::ctrl::XWALK_validateDeploymentConfig(repositoryConfiguration.string());
        EXPECT_TRUE(validReport.valid);
        ASSERT_FALSE(validReport.lines.empty());
        EXPECT_NE(validReport.lines.back().find("No hardware device"), ctrl::string::npos);
        const ctrl::stringvector repositoryEffective =
            xwalk::ctrl::XWALK_effectiveDeploymentConfig(repositoryConfiguration.string());
        EXPECT_NE(std::find(repositoryEffective.begin(), repositoryEffective.end(), "hardware_board = robot_hat_v4"),
                  repositoryEffective.end());
        EXPECT_NE(
            std::find(repositoryEffective.begin(), repositoryEffective.end(), "hardware_gpio_device = /dev/gpiochip4"),
            repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(), repositoryEffective.end(), "camera_connection = csi"),
                  repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(),
                            repositoryEffective.end(),
                            "video_stream_camera_backend = libcamera"),
                  repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(), repositoryEffective.end(), "video_stream_camera_device = csi"),
                  repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(),
                            repositoryEffective.end(),
                            "voice_active_car_gpt_maximum_output_tokens = 256"),
                  repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(),
                            repositoryEffective.end(),
                            "voice_active_car_gpt_model = llama3.2:3b"),
                  repositoryEffective.end());
        EXPECT_NE(
            std::find(repositoryEffective.begin(), repositoryEffective.end(), "voice_active_car_gpt_provider = ollama"),
            repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(),
                            repositoryEffective.end(),
                            "voice_active_car_gpt_endpoint = http://127.0.0.1:11434/api/chat"),
                  repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(),
                            repositoryEffective.end(),
                            "voice_active_car_gpt_with_image = false"),
                  repositoryEffective.end());
        EXPECT_NE(std::find(repositoryEffective.begin(),
                            repositoryEffective.end(),
                            "voice_active_car_gpt_continuous_conversation = true"),
                  repositoryEffective.end());
        const hal::XWalkConfigStore profileStore(repositoryConfiguration.string());
        EXPECT_EQ(profileStore.get("voice_active_car_model", ""), "gpt-4o-mini");
        EXPECT_EQ(profileStore.get("voice_active_car_api_key_environment", ""), "OPENAI_API_KEY");
        EXPECT_EQ(profileStore.get("voice_active_car_endpoint", ""), "https://api.openai.com/v1/chat/completions");
        EXPECT_EQ(profileStore.get("gpt_car_model", ""), "gpt-4o");
        EXPECT_EQ(profileStore.get("gpt_car_api_key_environment", ""), "OPENAI_API_KEY");
        EXPECT_EQ(profileStore.get("gpt_car_endpoint", ""), "https://api.openai.com/v1/chat/completions");

        const ctrl::filesystempath invalidConfiguration =
            ctrl::filesystempath("/tmp") /
            ("xwalk-invalid-deployment-config-" + std::to_string(static_cast<unsigned long>(::getpid())) + ".conf");
        {
            std::ofstream output(invalidConfiguration);
            ASSERT_TRUE(output.is_open());
            output << "deployment_config_version = 99\n";
            output << "hardware_board = unsupported\n";
            output << "hardware_v5_right_reverse_pwm_channel = P12\n";
            output << "hardware_v4_right_direction_pin = D4\n";
            output << "picarx_motor_watchdog_timeout_ms = 0\n";
            output << "computer_vision_camera_backend = unsupported\n";
            output << "voice_language_model_endpoint = https://user@example.invalid/api\n";
            output << "voice_language_model_api_key_environment = literal-secret\n";
        }
        const xwalk::ctrl::XWalkDeploymentConfigReport invalidReport =
            xwalk::ctrl::XWALK_validateDeploymentConfig(invalidConfiguration.string());
        EXPECT_FALSE(invalidReport.valid);
        EXPECT_GE(std::count_if(invalidReport.lines.begin(),
                                invalidReport.lines.end(),
                                [](const ctrl::string& line)
                                {
                                    return line.rfind("[FAIL]", 0U) == 0U;
                                }),
                  5);
        const ctrl::stringvector effective =
            xwalk::ctrl::XWALK_effectiveDeploymentConfig(invalidConfiguration.string());
        EXPECT_NE(
            std::find(effective.begin(), effective.end(), "voice_language_model_api_key_environment = <redacted>"),
            effective.end());
        EXPECT_NE(std::find(effective.begin(), effective.end(), "deployment_config_version = 99"), effective.end());
        std::error_code removeError;
        static_cast<void>(std::filesystem::remove(invalidConfiguration, removeError));
        EXPECT_FALSE(removeError);
    }

    /** @brief Rejects boundary violations for every deployment value family. */
    TEST(XWalkAppGroup, DeploymentConfigurationRejectsUnsafeBoundaries)
    {
        const ctrl::fixedarray<InvalidOverride, 57U> invalidOverrides{
            {{"deployment_config_version", "2"},
             {"hardware_board", "unknown"},
             {"hardware_i2c_device", "i2c-1"},
             {"hardware_gpio_device", "gpiochip0"},
             {"hardware_spi_device", "spidev0.0"},
             {"hardware_gpio_minimum_line_count", "0"},
             {"hardware_v5_left_forward_pwm_channel", "P16"},
             {"hardware_v5_right_reverse_pwm_channel", "P12"},
             {"hardware_v4_left_pwm_channel", "Q1"},
             {"hardware_v4_right_pwm_channel", "P13"},
             {"hardware_v4_left_direction_pin", "D17"},
             {"hardware_v4_right_direction_pin", "D4"},
             {"picarx_dir_motor", "[0,1]"},
             {"picarx_dir_servo", "181"},
             {"picarx_max_motor_output_percent", "101"},
             {"picarx_calibration_verified", "yes"},
             {"picarx_apply_persisted_servo_positions", "1"},
             {"picarx_motor_watchdog_timeout_ms", "60001"},
             {"camera_connection", "network"},
             {"computer_vision_camera_backend", "unknown"},
             {"computer_vision_width", "15"},
             {"computer_vision_height", "4321"},
             {"computer_vision_read_timeout_ms", "0"},
             {"video_recording_fps", "121"},
             {"voice_language_model_provider", "unknown"},
             {"voice_language_model_endpoint", "https://user@example.invalid/api"},
             {"voice_vosk_endpoint_start_seconds", "20"},
             {"voice_vosk_endpoint_end_seconds", "20"},
             {"voice_vosk_endpoint_max_seconds", "0"},
             {"voice_vosk_silence_peak_threshold", "0"},
             {"voice_vosk_trace_transcript", "yes"},
             {"voice_active_car_gpt_provider", "unknown"},
             {"voice_active_car_gpt_endpoint",
              "http://generativelanguage.googleapis.com/v1beta/openai/chat/completions"},
             {"voice_active_car_gpt_model", ""},
             {"voice_active_car_gpt_maximum_output_tokens", "0"},
             {"voice_active_car_gpt_maximum_output_tokens", "16385"},
             {"voice_active_car_gpt_timeout_ms", "0"},
             {"voice_active_car_gpt_timeout_ms", "300001"},
             {"voice_active_car_gpt_maximum_messages", "0"},
             {"voice_active_car_gpt_maximum_messages", "201"},
             {"voice_active_car_gpt_with_image", "true"},
             {"voice_active_car_gpt_with_image", "1"},
             {"voice_active_car_gpt_continuous_conversation", "yes"},
             {"voice_active_car_gpt_conversation_idle_timeout_ms", "0"},
             {"voice_active_car_gpt_conversation_idle_timeout_ms", "300001"},
             {"voice_active_car_gpt_conversation_maximum_rounds", "0"},
             {"voice_active_car_gpt_conversation_maximum_rounds", "101"},
             {"voice_active_car_gpt_conversation_maximum_misses", "0"},
             {"voice_active_car_gpt_sleep_phrases", "goodbye jarvis,  ,stop listening"},
             {"voice_active_car_gpt_sleep_phrases", "\"Goodbye Jarvis,goodbye jarvis\""},
             {"voice_active_car_gpt_web_search_enabled", "yes"},
             {"voice_active_car_gpt_web_search_endpoint", "http://192.168.1.2:8080/search"},
             {"voice_active_car_gpt_web_search_maximum_results", "0"},
             {"voice_active_car_gpt_web_search_maximum_results", "11"},
             {"voice_active_car_gpt_web_search_timeout_ms", "0"},
             {"voice_active_car_gpt_web_search_maximum_response_bytes", "1023"},
             {"app_control_port", "0"}}};

        for (const InvalidOverride& invalidOverride : invalidOverrides)
        {
            const xwalk::ctrl::XWalkDeploymentConfigReport report =
                validateDeploymentOverride(invalidOverride.name, invalidOverride.value);
            EXPECT_FALSE(report.valid) << invalidOverride.name;
        }
    }

    /** @brief Accepts supported alternatives and exact deployment boundaries. */
    TEST(XWalkAppGroup, DeploymentConfigurationAcceptsSupportedAlternatives)
    {
        const ctrl::fixedarray<ValidOverride, 31U> validOverrides{
            {{"hardware_board", "robot_hat_v4"},
             {"hardware_board", "robot_hat_v5"},
             {"hardware_gpio_minimum_line_count", "1024"},
             {"hardware_v4_left_direction_pin", "SW"},
             {"hardware_v4_left_direction_pin", "USER"},
             {"hardware_v4_left_direction_pin", "MCURST"},
             {"hardware_v4_left_direction_pin", "BOARD_TYPE"},
             {"hardware_v4_left_direction_pin", "BLEINT"},
             {"hardware_v4_left_direction_pin", "RST"},
             {"hardware_v4_left_direction_pin", "LED"},
             {"hardware_v4_left_direction_pin", "BLERST"},
             {"hardware_v4_left_direction_pin", "CE"},
             {"picarx_dir_motor", "[-1,1]"},
             {"picarx_dir_motor", "[1,-1]"},
             {"picarx_dir_motor", "[-1,-1]"},
             {"picarx_dir_servo", "-180"},
             {"picarx_cam_tilt_servo", "180"},
             {"picarx_max_motor_output_percent", "100"},
             {"picarx_motor_watchdog_timeout_ms", "60000"},
             {"camera_connection", "usb"},
             {"computer_vision_camera_backend", "automatic"},
             {"computer_vision_camera_backend", "gstreamer"},
             {"voice_capture_device", "plughw:CARD=Device,DEV=0"},
             {"voice_mixer_device", "pulse"},
             {"voice_mixer_element", "Master"},
             {"voice_piper_executable", "/opt/xwalk/piper-tts/venv/bin/piper"},
             {"voice_active_car_gpt_maximum_output_tokens", "16384"},
             {"voice_active_car_gpt_continuous_conversation", "false"},
             {"voice_active_car_gpt_conversation_idle_timeout_ms", "300000"},
             {"voice_active_car_gpt_conversation_maximum_rounds", "100"},
             {"app_control_port", "65535"}}};

        for (const ValidOverride& validOverride : validOverrides)
        {
            const xwalk::ctrl::XWalkDeploymentConfigReport report =
                validateDeploymentOverride(validOverride.name, validOverride.value);
            EXPECT_TRUE(report.valid) << validOverride.name << '=' << validOverride.value;
        }
    }

    /** @brief Accepts only safe paired video-stream backend and source selections. */
    TEST(XWalkAppGroup, DeploymentConfigurationValidatesVideoStreamSelection)
    {
        EXPECT_TRUE(validateVideoStreamSelection("libcamera", "csi").valid);
        EXPECT_TRUE(validateVideoStreamSelection("v4l2", "/dev/video12").valid);
        EXPECT_FALSE(validateVideoStreamSelection("gstreamer", "libcamerasrc ! appsink").valid);
        EXPECT_FALSE(validateVideoStreamSelection("automatic", "/dev/video0").valid);
        EXPECT_FALSE(validateVideoStreamSelection("libcamera", "libcamerasrc ! filesink location=/tmp/frame").valid);
        EXPECT_FALSE(validateVideoStreamSelection("v4l2", "/dev/media0").valid);
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
        const ctrl::filesystempath testExecutable = std::filesystem::read_symlink("/proc/self/exe", pathError);
        const ::ctrl::boolean pathErrorTestExecutableInvalid =
            static_cast<::ctrl::boolean>(pathError || testExecutable.empty());
        if (pathErrorTestExecutableInvalid)
        {
            return -1;
        }
        const ctrl::filesystempath applicationExecutable = testExecutable.parent_path() / "xwalk-picarx-control";
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
            const ::ctrl::boolean nullDescriptorInvalid = static_cast<::ctrl::boolean>(
                (::dup2(nullDescriptor, STDOUT_FILENO) < 0) || (::dup2(nullDescriptor, STDERR_FILENO) < 0));
            if (nullDescriptorInvalid)
            {
                ::_exit(126);
            }
            static_cast<void>(::close(nullDescriptor));
            if (thirdArgument != nullptr)
            {
                ::execl(applicationExecutable.c_str(),
                        applicationExecutable.c_str(),
                        firstArgument,
                        secondArgument,
                        thirdArgument,
                        static_cast<char*>(nullptr));
            }
            else if (secondArgument != nullptr)
            {
                ::execl(applicationExecutable.c_str(),
                        applicationExecutable.c_str(),
                        firstArgument,
                        secondArgument,
                        static_cast<char*>(nullptr));
            }
            else
            {
                ::execl(applicationExecutable.c_str(),
                        applicationExecutable.c_str(),
                        firstArgument,
                        static_cast<char*>(nullptr));
            }
            ::_exit(127);
        }

        int childStatus{};
        const ::ctrl::boolean childProcessChildStatusDifferent =
            static_cast<::ctrl::boolean>(::waitpid(childProcess, &childStatus, 0) != childProcess);
        if (childProcessChildStatusDifferent)
        {
            return -1;
        }
        const ::ctrl::boolean childExitedAbnormally = static_cast<::ctrl::boolean>(!WIFEXITED(childStatus));
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

    /** @brief Verifies the host CLI performs validation without constructing a boot backend. */
    TEST(XWalkAppGroup, NoHardwareConfigurationCli)
    {
        const ctrl::string configuration =
            "--deployment-config=" +
            (ctrl::filesystempath(XWALK_CONTROLLER_TRACE_EXAMPLE_DIRECTORY) / "picar-x.conf").string();
        EXPECT_EQ(runHostApplication(configuration.c_str(), "--validate-config"), 0);
        EXPECT_EQ(runHostApplication(configuration.c_str(), "--print-effective-config"), 0);
        EXPECT_EQ(runHostApplication("--diagnose", "--no-hardware"), 2);
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
        EXPECT_EQ(runHostApplication("--trace", "RPIAGENT.enable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "RPIAGENT.disable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "LIB.enable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "LIB.disable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "RPI.001.enable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "RPI.001.disable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "CTRL.001.enable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "CTRL.001.disable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "RPIAGENT.001.enable"), 0);
        EXPECT_EQ(runHostApplication("--trace", "RPIAGENT.001.disable"), 0);
        const ctrl::filesystempath jsonExample =
            ctrl::filesystempath(XWALK_CONTROLLER_TRACE_EXAMPLE_DIRECTORY) / "xwalk-traces.json";
        EXPECT_EQ(runHostApplication("--trace", jsonExample.c_str()), 0);
        EXPECT_EQ(runHostApplication("--trace", "RPI.99999.enable"), 2);
        EXPECT_EQ(runHostApplication("--trace", "UNKNOWN.enable"), 2);
        EXPECT_EQ(runHostApplication("--trace", "all.true"), 2);
        EXPECT_EQ(runHostApplication("--trace", "all.001.enable"), 2);
        EXPECT_EQ(runHostApplication("--trace", "CTRL..enable"), 2);
    }

    /**
     * @brief Verifies acceptance of a relative deployment-configuration path.
     */
    TEST(XWalkAppGroup, RelativeDeploymentConfiguration)
    {
        EXPECT_EQ(runHostApplication("--deployment-config", "../build-rpi/runtime/picar-x.conf", "--help"), 0);
    }

    /**
     * @brief Verifies that host execution rejects a hardware command.
     */
    TEST(XWalkAppGroup, HardwareCommandUnavailable)
    {
        EXPECT_EQ(runHostApplication("move", "forward"), 3);
    }

} /* namespace */
