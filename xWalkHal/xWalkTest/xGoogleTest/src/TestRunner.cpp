/******************************************************************************
 * @file        TestRunner.cpp
 * @brief       Implements centralized GoogleTest registration and selection.
 *
 * @details
 * Adapts the existing assertion-based HAL host entry points into isolated
 * GoogleTest cases and resolves XML, standard-filter, and custom selections.
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

#include "TestRunner.hpp"

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include <cerrno>
#include <cstring>

/******************************************************************************
 * Legacy test entry-point declarations
 ******************************************************************************/

int xWalkI2cLegacyMain();
int xWalkSpiLegacyMain();
int xWalkGpioLegacyMain();
int xWalkAudioAlsaLegacyMain();
int xWalkConfigLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkConfigStoreLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkTraceLegacyMain();
int xWalkUtilsLegacyMain();
int xWalkUtilsLinuxLegacyMain();
int xWalkLanguageModelLegacyMain();
int xWalkLanguageModelOllamaLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkMusicLegacyMain();
int xWalkMusicAlsaLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkSpeakerLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkSpeakerAlsaLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkPwmLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkAdcLegacyMain();
int xWalkServoLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkAdxl345LegacyMain();
int xWalkLineTrackerLegacyMain();
int xWalkUltrasonicLegacyMain();
int xWalkMotorLegacyMain();
int xWalkLedLegacyMain();
int xWalkRgbLedLegacyMain();
int xWalkBuzzerLegacyMain();
int xWalkCameraLegacyMain();
int xWalkUserButtonLegacyMain();
int xWalkBoardControlLegacyMain();
int xWalkDeviceLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkFirmwareInfoLegacyMain();
int xWalkRobotLegacyMain(int argumentCount, char* argumentValues[]);
int xWalkSpeechToTextLegacyMain();
int xWalkTextToSpeechLegacyMain();
int xWalkSpeechToTextAlsaLegacyMain();
int xWalkTextToSpeechAlsaLegacyMain();
int xWalkVoiceAssistantLegacyMain();
int xWalkVoiceAssistantBackendsLegacyMain();
int xWalkButtonEventSequenceHostTest();
int xWalkInitAnglesSequenceHostTest(int argumentCount, char* argumentValues[]);
int xWalkMotorSequenceHostTest();
int xWalkPiperStreamSequenceHostTest();
int xWalkRobotHat5MotorSequenceHostTest();
int xWalkServoHatSequenceHostTest();
int xWalkServoSequenceHostTest();
int xWalkToneSequenceHostTest();

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains legacy-test adapters private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/** @brief Legacy test entry point that accepts no arguments. */
using legacytestnoargs = int (*)();
/** @brief Legacy test entry point that accepts process-style arguments. */
using legacytestwithargs = int (*)(int argumentCount, char* argumentValues[]);
/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @brief Defines one dynamically registered legacy test scenario.
 */
struct LegacyTestDefinition
{
    /** @brief Exact GoogleTest suite name. */
    xwalk::hal::string suiteName;
    /** @brief Exact GoogleTest case name. */
    xwalk::hal::string caseName;
    /** @brief Non-null no-argument entry point when this form is used. */
    legacytestnoargs noArgumentFunction{};
    /** @brief Non-null argument-taking entry point when this form is used. */
    legacytestwithargs argumentFunction{};
    /** @brief Arguments following the synthesized executable name. */
    xwalk::hal::stringvector arguments;
    /** @brief External executable used by a physical-hardware case. */
    xwalk::hal::string executablePath;
    /** @brief Whether ThreadSanitizer must skip this failure-isolation scenario. */
    xwalk::hal::boolean skipWithThreadSanitizer{};
};

/** @brief Stable sequence of registered legacy test definitions. */
using legacytestdefinitionvector = std::vector<LegacyTestDefinition>;

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Creates one no-argument legacy test definition.
 *
 * @param[in] suiteName
 * Exact GoogleTest suite name.
 *
 * @param[in] caseName
 * Exact GoogleTest case name.
 *
 * @param[in] function
 * Non-null renamed legacy entry point.
 *
 * @return
 * Complete no-argument test definition.
 */
LegacyTestDefinition noArgumentTest(xwalk::hal::cstring suiteName, xwalk::hal::cstring caseName,
    legacytestnoargs function)
{
    return {suiteName, caseName, function, nullptr, {}, {}, false};
}

/**
 * @brief Creates one argument-taking legacy test definition.
 *
 * @param[in] suiteName
 * Exact GoogleTest suite name.
 *
 * @param[in] caseName
 * Exact GoogleTest case name.
 *
 * @param[in] function
 * Non-null renamed legacy entry point.
 *
 * @param[in] arguments
 * Arguments passed after the synthesized executable name.
 *
 * @param[in] skipWithThreadSanitizer
 * Whether the case is skipped in the ThreadSanitizer configuration.
 *
 * @return
 * Complete argument-taking test definition.
 */
LegacyTestDefinition argumentTest(xwalk::hal::cstring suiteName, xwalk::hal::cstring caseName,
    legacytestwithargs function, const xwalk::hal::stringvector& arguments,
    xwalk::hal::boolean skipWithThreadSanitizer = false)
{
    return {suiteName, caseName, nullptr, function, arguments, {}, skipWithThreadSanitizer};
}

/**
 * @brief Creates one external hardware-test definition.
 *
 * @param[in] suiteName
 * Exact GoogleTest suite name.
 *
 * @param[in] caseName
 * Exact GoogleTest case name.
 *
 * @param[in] executablePath
 * Absolute path to the existing module hardware-test executable.
 *
 * @param[in] arguments
 * Arguments passed after the executable name.
 *
 * @return
 * Complete external-process test definition.
 */
#if defined(XWALK_GOOGLE_TEST_HARDWARE_PROFILE)
LegacyTestDefinition externalTest(xwalk::hal::cstring suiteName,
    xwalk::hal::cstring caseName, xwalk::hal::cstring executablePath,
    const xwalk::hal::stringvector& arguments = {})
{
    return {suiteName, caseName, nullptr, nullptr, arguments, executablePath, false};
}
#endif

/**
 * @brief Builds the complete host-safe legacy test inventory.
 *
 * @return
 * Definitions in stable module and case order.
 */
legacytestdefinitionvector buildHostDefinitions(
    const xwalk::hal::filesystempath& binaryDirectoryPath)
{
    const xwalk::hal::string binaryDirectory(binaryDirectoryPath.string());
    const xwalk::hal::string processIdentifier = std::to_string(::getpid());
    return {
        noArgumentTest("TEST_SUITE_XWALK_I2C", "Initialization", &xWalkI2cLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_SPI", "Host", &xWalkSpiLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_GPIO", "Host", &xWalkGpioLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_AUDIO", "AlsaSoftware", &xWalkAudioAlsaLegacyMain),
        argumentTest("TEST_SUITE_XWALK_CONFIG", "Configuration", &xWalkConfigLegacyMain,
            {binaryDirectory + "/xWalkConfig/test-data/section-config/config.ini"}),
        argumentTest("TEST_SUITE_XWALK_CONFIG", "ConfigurationStore",
            &xWalkConfigStoreLegacyMain,
            {binaryDirectory + "/xWalkConfig/test-data/config-store/config-store.config"}),
        noArgumentTest("TEST_SUITE_XWALK_TRACE", "Host", &xWalkTraceLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_UTILS", "Host", &xWalkUtilsLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_UTILS", "LinuxSoftware", &xWalkUtilsLinuxLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_LANGUAGE_MODEL", "Host", &xWalkLanguageModelLegacyMain),
        argumentTest("TEST_SUITE_XWALK_LANGUAGE_MODEL", "Ollama",
            &xWalkLanguageModelOllamaLegacyMain, {binaryDirectory + "/xWalkLanguageModel"}),
        noArgumentTest("TEST_SUITE_XWALK_MUSIC", "Host", &xWalkMusicLegacyMain),
        argumentTest("TEST_SUITE_XWALK_MUSIC", "Alsa", &xWalkMusicAlsaLegacyMain,
            {binaryDirectory + "/xWalkMusic/music-alsa-test.wav"}),
        argumentTest("TEST_SUITE_XWALK_SPEAKER", "Concurrency", &xWalkSpeakerLegacyMain,
            {binaryDirectory + "/xWalkSpeaker/speaker-test.wav",
             binaryDirectory + "/xWalkSpeaker/speaker-test.MP3",
             binaryDirectory + "/xWalkSpeaker/speaker-test.txt", "concurrency"}),
        argumentTest("TEST_SUITE_XWALK_SPEAKER", "Failure", &xWalkSpeakerLegacyMain,
            {binaryDirectory + "/xWalkSpeaker/speaker-failure-test.wav",
             binaryDirectory + "/xWalkSpeaker/speaker-failure-test.MP3",
             binaryDirectory + "/xWalkSpeaker/speaker-failure-test.txt", "failure"}, true),
        argumentTest("TEST_SUITE_XWALK_SPEAKER", "Alsa", &xWalkSpeakerAlsaLegacyMain,
            {binaryDirectory + "/xWalkSpeaker/speaker-alsa-test.wav",
             binaryDirectory + "/xWalkSpeaker/speaker-alsa-test.ogg",
             binaryDirectory + "/xWalkSpeaker/speaker-alsa-test.mp3"}),
        argumentTest("TEST_SUITE_XWALK_PWM", "Address", &xWalkPwmLegacyMain, {"address"}),
        argumentTest("TEST_SUITE_XWALK_PWM", "TimerMapping", &xWalkPwmLegacyMain, {"mapping"}),
        argumentTest("TEST_SUITE_XWALK_PWM", "RegisterData", &xWalkPwmLegacyMain, {"register"}),
        argumentTest("TEST_SUITE_XWALK_PWM", "Percentage", &xWalkPwmLegacyMain, {"percentage"}),
        argumentTest("TEST_SUITE_XWALK_PWM", "Frequency", &xWalkPwmLegacyMain, {"frequency"}),
        argumentTest("TEST_SUITE_XWALK_PWM", "Validation", &xWalkPwmLegacyMain, {"validation"}),
        noArgumentTest("TEST_SUITE_XWALK_ADC", "Host", &xWalkAdcLegacyMain),
        argumentTest("TEST_SUITE_XWALK_SERVO", "Initialization", &xWalkServoLegacyMain,
            {"initialization"}),
        argumentTest("TEST_SUITE_XWALK_SERVO", "Angle", &xWalkServoLegacyMain, {"angle"}),
        argumentTest("TEST_SUITE_XWALK_SERVO", "PulseWidth", &xWalkServoLegacyMain, {"pulse"}),
        argumentTest("TEST_SUITE_XWALK_SERVO", "Validation", &xWalkServoLegacyMain, {"validation"}),
        noArgumentTest("TEST_SUITE_XWALK_ADXL345", "Host", &xWalkAdxl345LegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_LINE_TRACKER", "Host", &xWalkLineTrackerLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_ULTRASONIC", "Host", &xWalkUltrasonicLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_MOTOR", "Host", &xWalkMotorLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_LED", "SingleColor", &xWalkLedLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_LED", "Rgb", &xWalkRgbLedLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_BUZZER", "Host", &xWalkBuzzerLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_CAMERA", "Host", &xWalkCameraLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_USER_BUTTON", "Host", &xWalkUserButtonLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_BOARD_CONTROL", "BoardControl",
            &xWalkBoardControlLegacyMain),
        argumentTest("TEST_SUITE_XWALK_BOARD_CONTROL", "Device", &xWalkDeviceLegacyMain,
            {binaryDirectory + "/xWalkBoardControl/test-device-tree-" + processIdentifier}),
        noArgumentTest("TEST_SUITE_XWALK_BOARD_CONTROL", "FirmwareInfo",
            &xWalkFirmwareInfoLegacyMain),
        argumentTest("TEST_SUITE_XWALK_ROBOT", "Host", &xWalkRobotLegacyMain,
            {binaryDirectory + "/xWalkRobot/test-data/robot.config"}),
        noArgumentTest("TEST_SUITE_XWALK_GPT", "SpeechToText", &xWalkSpeechToTextLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_GPT", "TextToSpeech", &xWalkTextToSpeechLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_GPT", "SpeechToTextAlsa",
            &xWalkSpeechToTextAlsaLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_GPT", "TextToSpeechAlsa",
            &xWalkTextToSpeechAlsaLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_VOICE_ASSISTANT", "Host",
            &xWalkVoiceAssistantLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_VOICE_ASSISTANT", "Backends",
            &xWalkVoiceAssistantBackendsLegacyMain),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "ButtonEvent",
            &xWalkButtonEventSequenceHostTest),
        argumentTest("TEST_SUITE_XWALK_SEQUENCE", "InitAngles",
            &xWalkInitAnglesSequenceHostTest,
            {binaryDirectory + "/xSequenceTest/init-angles.config"}),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "RobotHat5Motor",
            &xWalkRobotHat5MotorSequenceHostTest),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "Motor",
            &xWalkMotorSequenceHostTest),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "ServoHat",
            &xWalkServoHatSequenceHostTest),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "Servo",
            &xWalkServoSequenceHostTest),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "PiperStream",
            &xWalkPiperStreamSequenceHostTest),
        noArgumentTest("TEST_SUITE_XWALK_SEQUENCE", "Tone",
            &xWalkToneSequenceHostTest),
    };
}

#if defined(XWALK_GOOGLE_TEST_HARDWARE_PROFILE)
/**
 * @brief Builds the physical-hardware test inventory.
 *
 * @return
 * External executable definitions in stable module order.
 */
legacytestdefinitionvector buildHardwareDefinitions(
    const xwalk::hal::filesystempath& runtimeConfigurationPath)
{
    const YAML::Node root = YAML::LoadFile(runtimeConfigurationPath.string());
    const YAML::Node schemaVersion = root["schema_version"];
    const YAML::Node board = root["board"];
    const YAML::Node ai = root["ai"];
    const YAML::Node configuredTests = root["hardware_tests"];
    const hal::boolean rootIsMapSchemaVersionInvalid =
        static_cast<hal::boolean>(
            !root.IsMap() || !schemaVersion.IsScalar() ||
        (schemaVersion.as<xwalk::hal::uint32>() != 1U) || !board.IsMap() ||
        !ai.IsMap() || !configuredTests.IsMap());
    if (rootIsMapSchemaVersionInvalid)
    {
        XHAL_THROW_RUNTIME_ERROR(
            "xGoogleTest hardware runtime YAML must contain schema_version 1, board, ai, and hardware_tests");
    }

    const auto configuredArguments = [&configuredTests](
        xwalk::hal::stringview suiteName,
        xwalk::hal::stringview caseName) -> xwalk::hal::stringvector
    {
        const YAML::Node arguments =
            configuredTests[xwalk::hal::string(suiteName)]
                           [xwalk::hal::string(caseName)]["arguments"];
        const hal::boolean argumentsInvalid =
            static_cast<hal::boolean>(
                !arguments || !arguments.IsSequence());
        if (argumentsInvalid)
        {
            XHAL_THROW_RUNTIME_ERROR("missing hardware runtime YAML arguments for " +
                xwalk::hal::string(suiteName) + "." + xwalk::hal::string(caseName));
        }
        xwalk::hal::stringvector values;
        for (const YAML::Node& argument : arguments)
        {
            const hal::boolean scalarNotMatched =
                static_cast<hal::boolean>(
                    !argument.IsScalar());
            if (scalarNotMatched)
            {
                XHAL_THROW_RUNTIME_ERROR("hardware runtime YAML arguments must be scalar values");
            }
            values.push_back(argument.as<xwalk::hal::string>());
        }
        return values;
    };

    return {
        externalTest("TEST_SUITE_XWALK_ADC", "HardwareRead", XWALK_HARDWARE_ADC_TEST),
        externalTest("TEST_SUITE_XWALK_ADXL345", "HardwareRead",
            XWALK_HARDWARE_ADXL345_TEST),
        externalTest("TEST_SUITE_XWALK_AUDIO", "SilentPlayback",
            XWALK_HARDWARE_AUDIO_TEST),
        externalTest("TEST_SUITE_XWALK_BOARD_CONTROL", "Disable",
            XWALK_HARDWARE_BOARD_CONTROL_TEST),
        externalTest("TEST_SUITE_XWALK_BOARD_CONTROL", "DeviceDiscovery",
            XWALK_HARDWARE_DEVICE_TEST),
        externalTest("TEST_SUITE_XWALK_BOARD_CONTROL", "FirmwareRead",
            XWALK_HARDWARE_FIRMWARE_TEST),
        externalTest("TEST_SUITE_XWALK_BUZZER", "Inactive",
            XWALK_HARDWARE_BUZZER_TEST),
        externalTest("TEST_SUITE_XWALK_CAMERA", "StillCapture",
            XWALK_HARDWARE_CAMERA_TEST,
            configuredArguments("TEST_SUITE_XWALK_CAMERA", "StillCapture")),
        externalTest("TEST_SUITE_XWALK_GPIO", "Output", XWALK_HARDWARE_GPIO_TEST,
            configuredArguments("TEST_SUITE_XWALK_GPIO", "Output")),
        externalTest("TEST_SUITE_XWALK_GPT", "Microphone",
            XWALK_HARDWARE_GPT_STT_TEST),
        externalTest("TEST_SUITE_XWALK_GPT", "Playback",
            XWALK_HARDWARE_GPT_TTS_TEST),
        externalTest("TEST_SUITE_XWALK_I2C", "Probe", XWALK_HARDWARE_I2C_TEST,
            configuredArguments("TEST_SUITE_XWALK_I2C", "Probe")),
        externalTest("TEST_SUITE_XWALK_LANGUAGE_MODEL", "Ollama",
            XWALK_HARDWARE_LANGUAGE_MODEL_TEST),
        externalTest("TEST_SUITE_XWALK_LED", "Inactive", XWALK_HARDWARE_LED_TEST),
        externalTest("TEST_SUITE_XWALK_LED", "RgbOutput", XWALK_HARDWARE_RGB_LED_TEST),
        externalTest("TEST_SUITE_XWALK_MOTOR", "Stop", XWALK_HARDWARE_MOTOR_TEST),
        externalTest("TEST_SUITE_XWALK_MUSIC", "LowVolume",
            XWALK_HARDWARE_MUSIC_TEST),
        externalTest("TEST_SUITE_XWALK_PWM", "ZeroOutput", XWALK_HARDWARE_PWM_TEST),
        externalTest("TEST_SUITE_XWALK_SERVO", "Initialization",
            XWALK_HARDWARE_SERVO_TEST),
        externalTest("TEST_SUITE_XWALK_SPEAKER", "LowVolume",
            XWALK_HARDWARE_SPEAKER_TEST),
        externalTest("TEST_SUITE_XWALK_SPI", "Transfer", XWALK_HARDWARE_SPI_TEST,
            configuredArguments("TEST_SUITE_XWALK_SPI", "Transfer")),
        externalTest("TEST_SUITE_XWALK_TRACE", "TargetCompile",
            XWALK_HARDWARE_TRACE_TEST),
        externalTest("TEST_SUITE_XWALK_USER_BUTTON", "Monitor",
            XWALK_HARDWARE_USER_BUTTON_TEST),
        externalTest("TEST_SUITE_XWALK_UTILS", "Platform",
            XWALK_HARDWARE_UTILS_TEST),
        externalTest("TEST_SUITE_XWALK_VOICE_ASSISTANT", "Backends",
            XWALK_HARDWARE_VOICE_ASSISTANT_TEST),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "ButtonEvent",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "ButtonEvent")),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "InitAngles",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "InitAngles")),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "RobotHat5Motor",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "RobotHat5Motor")),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "Motor",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "Motor")),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "ServoHat",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "ServoHat")),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "Servo",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "Servo")),
        externalTest("TEST_SUITE_XWALK_SEQUENCE", "Tone",
            XWALK_HARDWARE_SEQUENCE_TEST,
            configuredArguments("TEST_SUITE_XWALK_SEQUENCE", "Tone")),
    };
}
#endif

/**
 * @brief Selects the compiled inventory for one profile.
 *
 * @param[in] profile
 * Requested host or hardware profile.
 *
 * @return
 * Definitions belonging only to the selected profile.
 */
legacytestdefinitionvector buildDefinitions(xwalk::hal::test::TestProfile profile,
    const xwalk::hal::filesystempath& binaryDirectory,
    const xwalk::hal::filesystempath& runtimeConfigurationPath)
{
#if defined(XWALK_GOOGLE_TEST_HARDWARE_PROFILE)
    if (profile == xwalk::hal::test::TestProfile::Hardware)
    {
        return buildHardwareDefinitions(runtimeConfigurationPath);
    }
#else
    static_cast<void>(profile);
    static_cast<void>(runtimeConfigurationPath);
#endif
    return buildHostDefinitions(binaryDirectory);
}

/**
 * @brief Finds one suite by exact name.
 *
 * @param[in,out] suites
 * Suite collection to search.
 *
 * @param[in] name
 * Exact suite name.
 *
 * @return
 * Non-owning suite pointer when found; otherwise `nullptr`.
 */
xwalk::hal::test::TestSuiteConfig* findSuite(
    xwalk::hal::test::testsuiteconfigvector& suites, xwalk::hal::stringview name)
{
    for (xwalk::hal::test::TestSuiteConfig& suite : suites)
    {
        if (suite.name == name)
        {
            return &suite;
        }
    }
    return nullptr;
}

/**
 * @brief Finds one case by exact name.
 *
 * @param[in,out] suite
 * Suite whose cases are searched.
 *
 * @param[in] name
 * Exact case name.
 *
 * @return
 * Non-owning case pointer when found; otherwise `nullptr`.
 */
xwalk::hal::test::TestCaseConfig* findCase(
    xwalk::hal::test::TestSuiteConfig& suite, xwalk::hal::stringview name)
{
    for (xwalk::hal::test::TestCaseConfig& testCase : suite.cases)
    {
        if (testCase.name == name)
        {
            return &testCase;
        }
    }
    return nullptr;
}

/**
 * @brief Finds one case by exact name.
 *
 * @param[in] suite
 * Suite whose cases are searched.
 *
 * @param[in] name
 * Exact case name.
 *
 * @return
 * Non-owning case pointer when found; otherwise `nullptr`.
 */
const xwalk::hal::test::TestCaseConfig* findCase(
    const xwalk::hal::test::TestSuiteConfig& suite, xwalk::hal::stringview name)
{
    for (const xwalk::hal::test::TestCaseConfig& testCase : suite.cases)
    {
        if (testCase.name == name)
        {
            return &testCase;
        }
    }
    return nullptr;
}

/**
 * @brief Runs one legacy test definition with process-style arguments.
 *
 * @param[in] definition
 * Complete test definition containing exactly one callable entry point.
 *
 * @return
 * Legacy test return status.
 */
int invokeLegacyTest(const LegacyTestDefinition& definition)
{
    const hal::boolean executablePathAvailable =
        static_cast<hal::boolean>(
            !definition.executablePath.empty());
    if (executablePathAvailable)
    {
        xwalk::hal::stringvector arguments;
        arguments.push_back(definition.executablePath);
        arguments.insert(arguments.end(), definition.arguments.begin(),
            definition.arguments.end());
        xwalk::hal::charpointervector pointers;
        for (xwalk::hal::string& argument : arguments)
        {
            pointers.push_back(argument.data());
        }
        pointers.push_back(nullptr);
        ::execv(definition.executablePath.c_str(), pointers.data());
        std::cerr << "unable to execute hardware test '" << definition.executablePath
                  << "': " << std::strerror(errno) << '\n';
        return EXIT_FAILURE;
    }
    if (definition.noArgumentFunction != nullptr)
    {
        return definition.noArgumentFunction();
    }

    xwalk::hal::stringvector arguments;
    arguments.push_back("xGoogleTest");
    for (const xwalk::hal::string& argument : definition.arguments)
    {
        arguments.push_back(argument);
    }
    xwalk::hal::charpointervector pointers;
    for (xwalk::hal::string& argument : arguments)
    {
        pointers.push_back(argument.data());
    }
    return definition.argumentFunction(static_cast<int>(pointers.size()), pointers.data());
}

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @brief Executes one existing assertion-based test as a GoogleTest case.
 */
class LegacyGoogleTest : public ::testing::Test
{
public:

/**************************************************************************
 * Public constructors and destructor
 **************************************************************************/

    /**
     * @brief Stores one immutable legacy test definition.
     *
     * @param[in] definition
     * Test entry point and owned argument values copied into this instance.
     */
    explicit LegacyGoogleTest(LegacyTestDefinition definition)
        : definitionValue(std::move(definition))
    {
    }

protected:

/**************************************************************************
 * Protected member functions
 **************************************************************************/

    /**
     * @brief Runs the legacy scenario in an isolated child process.
     *
     * @post
     * A normal zero exit passes; a signal or non-zero exit records a GoogleTest
     * failure without terminating the remaining central test cases.
     */
    void TestBody() override
    {
#if defined(XWALK_GOOGLE_TEST_THREAD_SANITIZER)
        if (definitionValue.skipWithThreadSanitizer)
        {
            GTEST_SKIP() << "legacy failure-isolation scenario is excluded under ThreadSanitizer";
        }
#endif
#if defined(__linux__)
        const pid_t childProcess = ::fork();
        ASSERT_GE(childProcess, 0) << "fork failed for legacy test";
        if (childProcess == 0)
        {
            const int result = invokeLegacyTest(definitionValue);
            ::exit(result);
        }

        int childStatus{};
        const pid_t completedProcess = ::waitpid(childProcess, &childStatus, 0);
        ASSERT_EQ(completedProcess, childProcess) << "waitpid failed for legacy test";
        const hal::boolean childTerminatedBySignal =
            static_cast<hal::boolean>(
                WIFSIGNALED(childStatus));
        if (childTerminatedBySignal)
        {
            ADD_FAILURE() << "legacy test terminated by signal " << WTERMSIG(childStatus);
            return;
        }
        ASSERT_TRUE(WIFEXITED(childStatus)) << "legacy test ended without an exit status";
        EXPECT_EQ(WEXITSTATUS(childStatus), EXIT_SUCCESS)
            << "legacy test returned a failure status";
#else
        ADD_FAILURE() << "central legacy-test isolation requires a Linux host";
#endif
    }

private:

/**************************************************************************
 * Private data members
 **************************************************************************/

    /** @brief Owned entry-point definition executed by this test instance. */
    LegacyTestDefinition definitionValue;
};

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Public constructor definitions
 ******************************************************************************/

/**
 * @brief Creates the complete centralized test inventory.
 *
 * @param[in] profile
 * Host or hardware inventory selected before registration.
 *
 * @param[in] binaryDirectory
 * Absolute HAL build directory containing generated host-test data.

 * @param[in] runtimeConfigurationPath
 * YAML file containing board and AI hardware-test arguments.
 *
 * @post
 * `availableTests()` contains every host-safe scenario compiled into the
 * central executable.
 */
TestRunner::TestRunner(TestProfile profile, filesystempath binaryDirectory,
    filesystempath runtimeConfigurationPath)
    : profileValue(profile), binaryDirectoryValue(std::move(binaryDirectory)),
      runtimeConfigurationPathValue(std::move(runtimeConfigurationPath))
{
    for (const LegacyTestDefinition& definition :
         buildDefinitions(profileValue, binaryDirectoryValue,
             runtimeConfigurationPathValue))
    {
        TestSuiteConfig* suite = findSuite(availableSuites, definition.suiteName);
        if (suite == nullptr)
        {
            availableSuites.push_back({definition.suiteName, true, {}});
            suite = &availableSuites.back();
        }
        suite->cases.push_back({definition.caseName, true});
    }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Registers the inventory with GoogleTest.
 *
 * @post
 * Every available suite and case can be selected by a GoogleTest filter.
 */
void TestRunner::registerTests() const
{
    for (const LegacyTestDefinition& definition :
         buildDefinitions(profileValue, binaryDirectoryValue,
             runtimeConfigurationPathValue))
    {
        // GoogleTest owns the registered factory; Clang Analyzer does not model that transfer.
        // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
        static_cast<void>(::testing::RegisterTest(
            definition.suiteName.c_str(), definition.caseName.c_str(), nullptr, nullptr,
            __FILE__, __LINE__, [definition]() -> LegacyGoogleTest*
            {
                return new LegacyGoogleTest(definition);
            }));
    }
}

/**
 * @brief Parses and removes the explicit test-profile argument.
 *
 * @param[in,out] argumentCount
 * Original process argument count, compacted after parsing.
 *
 * @param[in,out] argumentValues
 * Mutable process argument array.
 *
 * @param[out] profile
 * Host by default, or the explicitly selected profile.
 *
 * @param[in,out] runtimeConfigurationPath
 * Default YAML path replaced by an explicit `--runtime-config` value.
 *
 * @param[out] error
 * Empty on success; otherwise a profile-selection diagnostic.
 *
 * @return
 * `true` when the profile arguments are valid; otherwise `false`.
 */
boolean TestRunner::processProfile(int32& argumentCount, charpointer argumentValues[],
    TestProfile& profile, filesystempath& runtimeConfigurationPath, string& error)
{
    profile = TestProfile::Host;
    error.clear();
    boolean profileSeen = false;
    boolean runtimeConfigurationSeen = false;
    int32 outputIndex = 1;
    for (int32 index = 1; index < argumentCount; ++index)
    {
        const string argument(argumentValues[index]);
        if (argument == "--runtime-config")
        {
            if (runtimeConfigurationSeen || ((index + 1) >= argumentCount))
            {
                error = "--runtime-config requires one YAML path and may appear once";
                return false;
            }
            runtimeConfigurationPath = argumentValues[index + 1];
            runtimeConfigurationSeen = true;
            ++index;
            continue;
        }
        const hal::boolean runtimeConfigAssignmentMatched =
            static_cast<hal::boolean>(
                argument.rfind("--runtime-config=", 0U) == 0U);
        if (runtimeConfigAssignmentMatched)
        {
            const hal::boolean runtimeConfigurationSeenArgumentInvalid =
                static_cast<hal::boolean>(
                    runtimeConfigurationSeen || (argument.size() == 17U));
            if (runtimeConfigurationSeenArgumentInvalid)
            {
                error = "--runtime-config requires one YAML path and may appear once";
                return false;
            }
            runtimeConfigurationPath = argument.substr(17U);
            runtimeConfigurationSeen = true;
            continue;
        }
        const hal::boolean differentTestProfileAssignmentMatched =
            static_cast<hal::boolean>(
                argument.rfind("--test-profile=", 0U) != 0U);
        if (differentTestProfileAssignmentMatched)
        {
            argumentValues[outputIndex] = argumentValues[index];
            ++outputIndex;
            continue;
        }
        if (profileSeen)
        {
            error = "--test-profile may be specified only once";
            return false;
        }
        const string value = argument.substr(string("--test-profile=").size());
        if (value == "host")
        {
            profile = TestProfile::Host;
        }
        else if (value == "hardware")
        {
            profile = TestProfile::Hardware;
        }
        else
        {
            error = "test profile must be host or hardware: " + value;
            return false;
        }
        profileSeen = true;
    }
    argumentCount = outputIndex;
    argumentValues[outputIndex] = nullptr;
    return true;
}

/**
 * @brief Returns the registered suite and case inventory.
 *
 * @return
 * Read-only inventory valid for this object's lifetime.
 */
const testsuiteconfigvector& TestRunner::availableTests() const noexcept
{
    return availableSuites;
}

/**
 * @brief Detects an explicit standard GoogleTest filter.
 *
 * @param[in] argumentCount
 * Number of original process arguments.
 *
 * @param[in] argumentValues
 * Non-owning process argument array.
 *
 * @return
 * `true` when `--gtest_filter` is present; otherwise `false`.
 */
boolean TestRunner::hasStandardFilter(int32 argumentCount, charpointer argumentValues[]) const
{
    for (int32 index = 1; index < argumentCount; ++index)
    {
        const stringview argument(argumentValues[index]);
        const hal::boolean argumentGtestFilterInvalid =
            static_cast<hal::boolean>(
                (argument == "--gtest_filter") || argument.rfind("--gtest_filter=", 0U) == 0U);
        if (argumentGtestFilterInvalid)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Parses and removes custom suite or case selections.
 *
 * @param[in,out] argumentCount
 * GoogleTest-filtered argument count; reduced after recognized selections.
 *
 * @param[in,out] argumentValues
 * Non-owning argument array compacted in place.
 *
 * @param[out] error
 * Empty on success; otherwise the invalid selection and valid inventory.
 *
 * @return
 * `true` when every remaining argument is a valid selection; otherwise `false`.
 */
boolean TestRunner::processSelections(
    int32& argumentCount, charpointer argumentValues[], string& error)
{
    selections.clear();
    error.clear();
    for (int32 index = 1; index < argumentCount; ++index)
    {
        const string argument(argumentValues[index]);
        const size firstSeparator = argument.find(':');
        const size secondSeparator = firstSeparator == string::npos
            ? string::npos : argument.find(':', firstSeparator + 1U);
        const size thirdSeparator = secondSeparator == string::npos
            ? string::npos : argument.find(':', secondSeparator + 1U);
        if ((firstSeparator == string::npos) || (thirdSeparator != string::npos))
        {
            error = "expected SUITE:STATE or SUITE:CASE:STATE, received '" + argument + "'";
            return false;
        }

        const string suiteName = argument.substr(0U, firstSeparator);
        const string caseName = secondSeparator == string::npos ? string() :
            argument.substr(firstSeparator + 1U,
                secondSeparator - firstSeparator - 1U);
        const string state = secondSeparator == string::npos
            ? argument.substr(firstSeparator + 1U) : argument.substr(secondSeparator + 1U);
        if ((state != "0") && (state != "1"))
        {
            error = "selection state must be 0 or 1: " + argument;
            return false;
        }

        const TestSuiteConfig* suite = findSuite(availableSuites, suiteName);
        if (suite == nullptr)
        {
            error = "unknown test suite: " + suiteName;
            return false;
        }
        const hal::boolean requestedCaseMissing =
            static_cast<hal::boolean>(
                !caseName.empty() && (findCase(*suite, caseName) == nullptr));
        if (requestedCaseMissing)
        {
            error = "unknown test case: ";
            error += suiteName;
            error += ".";
            error += caseName;
            return false;
        }
        selections.push_back({suiteName, caseName, state == "1"});
    }
    argumentCount = 1;
    argumentValues[1] = nullptr;
    return true;
}

/**
 * @brief Applies the filter selected by the documented precedence.
 *
 * @param[in] configuration
 * Validated XML configuration used when no higher-priority selection exists.
 *
 * @param[in] standardFilterSelected
 * Whether the original command line supplied `--gtest_filter`.
 */
void TestRunner::applyFilter(const TestConfig& configuration, boolean standardFilterSelected) const
{
    const hal::boolean standardFilterOnly =
        static_cast<hal::boolean>(
            selections.empty() && standardFilterSelected);
    if (standardFilterOnly)
    {
        return;
    }

    testsuiteconfigvector selectedSuites = configuration.suites();
    const hal::boolean selectionsAvailable =
        static_cast<hal::boolean>(
            !selections.empty());
    if (selectionsAvailable)
    {
        boolean hasEnabledSelection = false;
        for (const TestSelection& selection : selections)
        {
            hasEnabledSelection = hasEnabledSelection || selection.enabled;
        }
        if (hasEnabledSelection)
        {
            for (TestSuiteConfig& suite : selectedSuites)
            {
                suite.enabled = true;
                for (TestCaseConfig& testCase : suite.cases)
                {
                    testCase.enabled = false;
                }
            }
        }

        for (const TestSelection& selection : selections)
        {
            TestSuiteConfig* suite = findSuite(selectedSuites, selection.suiteName);
            const hal::boolean caseNameEmpty =
                static_cast<hal::boolean>(
                    selection.caseName.empty());
            if (caseNameEmpty)
            {
                suite->enabled = selection.enabled;
                for (TestCaseConfig& testCase : suite->cases)
                {
                    testCase.enabled = selection.enabled;
                }
            }
            else
            {
                TestCaseConfig* testCase = findCase(*suite, selection.caseName);
                testCase->enabled = selection.enabled;
                if (selection.enabled)
                {
                    suite->enabled = true;
                }
            }
        }
    }

    string filter;
    for (const TestSuiteConfig& suite : selectedSuites)
    {
        if (!suite.enabled)
        {
            continue;
        }
        for (const TestCaseConfig& testCase : suite.cases)
        {
            if (!testCase.enabled)
            {
                continue;
            }
            const hal::boolean filterAvailable =
                static_cast<hal::boolean>(
                    !filter.empty());
            if (filterAvailable)
            {
                filter += ':';
            }
            filter += suite.name + "." + testCase.name;
        }
    }
    const hal::boolean filterEmpty =
        static_cast<hal::boolean>(
            filter.empty());
    if (filterEmpty)
    {
        filter = "-*";
    }
    GTEST_FLAG_SET(filter, filter);
}

/**
 * @brief Formats every valid suite and case for an error diagnostic.
 *
 * @return
 * Multi-line owned text listing the complete registered inventory.
 */
string TestRunner::validTestsText() const
{
    string text("Valid test suites and cases:\n");
    for (const TestSuiteConfig& suite : availableSuites)
    {
        text += "  " + suite.name + '\n';
        for (const TestCaseConfig& testCase : suite.cases)
        {
            text += "    " + testCase.name + '\n';
        }
    }
    return text;
}

} /* namespace xwalk::hal::test */
