/******************************************************************************
 * @file        xAgent_Rpi5CarControllerTest.cpp
 * @brief       Verifies PiCar-X CLI commands with in-memory backends.
 *
 * @details
 * Exercises parsing, movement, line tracking, preset actions, sensors, audio
 * dispatch, and calibration prompts.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Host Test
 *
 * @author      Joxy John
 * @date        2026-07-31
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

#include "xAgent_Rpi5CarController.h"

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and callbacks private to this translation unit. */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Supplies one deterministic 1,000-count ADC sample. */
struct TestBus
{
    XWalkHal::bytevector sample{0x03U, 0xE8U};
};

/** @brief Stores one simulated GPIO level. */
struct TestGpio
{
    XWalkHal::boolean value{};
};

/** @brief Records CLI platform interactions. */
struct TestCliBackend
{
    XWalkHal::stringvector outputLines;
    XWalkHal::stringvector inputLines;
    XWalkHal::size inputIndex{};
    XWalkHal::uint32vector delays;
    XWalkHal::float64vector leftSpeeds;
    XWalkHal::float64vector rightSpeeds;
    XWalkHal::float64vector steeringAngles;
    xwalk::hal::XWalkMotors* motors{nullptr};
    xwalk::agent::XWalkPicarx* picarx{nullptr};
    XWalkHal::boolean soundAvailable{true};
    xwalk::agent::XWalkSoundOperation soundOperation{
        xwalk::agent::XWalkSoundOperation::Stop};
    XWalkHal::string soundFile;
    XWalkHal::optionalfloat64 soundVolume;
    XWalkHal::uint32 operationQueries{};
    XWalkHal::uint32 operationQueryLimit{1'000'000U};
    XWalkHal::string musicSoundFile;
    XWalkHal::boolean failDelay{};
    XWalkHal::boolean failSound{};
};

/******************************************************************************
 * Private callback definitions
 ******************************************************************************/

XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::uint8 reg,
    const XWalkHal::bytevector& data)
{
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    static_cast<void>(context);
}

/** @brief Accepts one non-throwing simulated fail-safe write. */
XWalkHal::boolean tryWriteRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data) noexcept
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    return true;
}

XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    return static_cast<TestBus*>(context)->sample;
}

/** @brief Returns each transmitted SPI byte with every bit inverted. */
XWalkHal::bytevector transferSpi(XWalkHal::contextpointer context,
    const XWalkHal::bytevector& transmitData)
{
    static_cast<void>(context);
    XWalkHal::bytevector response;
    response.reserve(transmitData.size());
    for (const XWalkHal::uint8 value : transmitData)
    {
        response.push_back(static_cast<XWalkHal::uint8>(value ^ 0xFFU));
    }
    return response;
}

void configureGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull, XWalkHal::boolean initialValue)
{
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<TestGpio*>(context)->value = initialValue;
}

XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(pin);
    return static_cast<TestGpio*>(context)->value;
}

void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
{
    static_cast<void>(pin);
    static_cast<TestGpio*>(context)->value = value;
}

void interruptGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioEdge edge, XWalkHal::uint32 debounceMs,
    XWalkHal::contextpointer handlerContext, XWalkHal::gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterrupt};
}

void outputLine(XWalkHal::contextpointer context, XWalkHal::stringview line)
{
    static_cast<TestCliBackend*>(context)->outputLines.emplace_back(line);
}

XWalkHal::string inputLine(XWalkHal::contextpointer context, XWalkHal::stringview prompt)
{
    static_cast<void>(prompt);
    TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
    if (backend.inputIndex >= backend.inputLines.size())
    {
        return "skip";
    }
    const XWalkHal::string response = backend.inputLines[backend.inputIndex];
    ++backend.inputIndex;
    return response;
}

void delayMilliseconds(XWalkHal::contextpointer context, XWalkHal::uint32 durationMs)
{
    TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
    backend.delays.push_back(durationMs);
    backend.leftSpeeds.push_back(backend.motors->left().speed());
    backend.rightSpeeds.push_back(backend.motors->right().speed());
    backend.steeringAngles.push_back(backend.picarx->directionAngleDegrees());
}

/** @brief Records one self-drive delay and reports explicit failure status. */
XWalkHal::boolean selfDriveDelayMilliseconds(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMs) noexcept
{
    TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
    backend.delays.push_back(durationMs);
    backend.leftSpeeds.push_back(backend.motors->left().speed());
    backend.rightSpeeds.push_back(backend.motors->right().speed());
    backend.steeringAngles.push_back(backend.picarx->directionAngleDegrees());
    return !backend.failDelay;
}

/**
 * @brief Supplies a bounded foreground line-tracking continuation sequence.
 * @param[in,out] context Non-null test backend that records query count.
 * @return `true` until the configured number of test steps has been reached.
 */
XWalkHal::boolean continueOperation(XWalkHal::contextpointer context)
{
    TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
    const XWalkHal::boolean shouldContinue = backend.operationQueries < backend.operationQueryLimit;
    ++backend.operationQueries;
    return shouldContinue;
}

XWalkHal::boolean soundOperation(XWalkHal::contextpointer context,
    xwalk::agent::XWalkSoundOperation operation, XWalkHal::stringview filePath,
    XWalkHal::optionalfloat64 volumePercent)
{
    TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
    backend.soundOperation = operation;
    backend.soundFile = filePath;
    backend.soundVolume = volumePercent;
    return backend.soundAvailable && (!backend.failSound);
}

/**
 * @brief Accepts simulated music-output enablement without hardware access.
 * @param[in,out] context Optional test context; unused.
 */
void enableMusicOutput(XWalkHal::contextpointer context)
{
    static_cast<void>(context);
}

/**
 * @brief Records one simulated synchronous or background sound request.
 * @param[in,out] context Non-null test backend receiving the file name.
 * @param[in] filename Sound path copied for later assertions.
 * @param[in] normalizedVolume Optional normalized volume; unused by this test backend.
 */
void playMusicSound(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::optionalfloat64 normalizedVolume)
{
    static_cast<void>(normalizedVolume);
    static_cast<TestCliBackend*>(context)->musicSoundFile = filename;
}

/**
 * @brief Accepts one simulated streamed-music request.
 * @param[in,out] context Optional test context; unused.
 * @param[in] filename Music path; unused.
 * @param[in] loops Additional playback repetitions; unused.
 * @param[in] startSeconds Playback offset in seconds; unused.
 */
void playMusicFile(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::int32 loops, XWalkHal::float64 startSeconds)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    static_cast<void>(loops);
    static_cast<void>(startSeconds);
}

/**
 * @brief Accepts one simulated streamed-music volume request.
 * @param[in,out] context Optional test context; unused.
 * @param[in] normalizedVolume Normalized volume; unused.
 */
void setMusicVolume(XWalkHal::contextpointer context, XWalkHal::float64 normalizedVolume)
{
    static_cast<void>(context);
    static_cast<void>(normalizedVolume);
}

/**
 * @brief Accepts one simulated music transport request.
 * @param[in,out] context Optional test context; unused.
 */
void controlMusic(XWalkHal::contextpointer context)
{
    static_cast<void>(context);
}

/**
 * @brief Supplies a deterministic simulated sound length.
 * @param[in,out] context Optional test context; unused.
 * @param[in] filename Sound path; unused.
 * @return One second.
 */
XWalkHal::float64 musicSoundLength(XWalkHal::contextpointer context, XWalkHal::stringview filename)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    return 1.0;
}

/**
 * @brief Accepts one simulated PCM tone request.
 * @param[in,out] context Optional test context; unused.
 * @param[in] pcmData Complete PCM bytes; unused.
 * @param[in] sampleRateHz Positive sample rate in Hertz; unused.
 * @param[in] channelCount Interleaved channel count; unused.
 */
void playMusicTone(XWalkHal::contextpointer context, const XWalkHal::bytevector& pcmData,
    XWalkHal::uint32 sampleRateHz, XWalkHal::uint8 channelCount)
{
    static_cast<void>(context);
    static_cast<void>(pcmData);
    static_cast<void>(sampleRateHz);
    static_cast<void>(channelCount);
}

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/**
 * @brief Exercises every CLI command group through one complete host composition.
 * @param[in] configPath Test-owned configuration path below the build directory.
 */
void testCommands(XWalkHal::stringview configPath)
{
    TestBus bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm leftPwm(i2c, "P13", 0x14U, timerState);
    xwalk::hal::XWalkPwm rightPwm(i2c, "P12", 0x14U, timerState);
    xwalk::hal::XWalkPwm directionPwm(i2c, "P2", 0x14U, timerState);
    xwalk::hal::XWalkPwm panPwm(i2c, "P0", 0x14U, timerState);
    xwalk::hal::XWalkPwm tiltPwm(i2c, "P1", 0x14U, timerState);
    TestGpio leftBackend;
    TestGpio rightBackend;
    TestGpio triggerBackend;
    TestGpio echoBackend;
    const XWalkHal::XWalkGpioCallbacks gpioBackendCallbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio leftDirection(&leftBackend, gpioBackendCallbacks, "D4");
    xwalk::hal::XWalkGpio rightDirection(&rightBackend, gpioBackendCallbacks, "D5");
    xwalk::hal::XWalkGpio trigger(&triggerBackend, gpioBackendCallbacks, "D2");
    xwalk::hal::XWalkGpio echo(&echoBackend, gpioBackendCallbacks, "D3");
    xwalk::hal::XWalkMotor leftMotor(leftPwm, leftDirection);
    xwalk::hal::XWalkMotor rightMotor(rightPwm, rightDirection);
    xwalk::hal::XWalkMotors motors(leftMotor, rightMotor);
    xwalk::hal::XWalkServo directionServo(directionPwm);
    xwalk::hal::XWalkServo panServo(panPwm);
    xwalk::hal::XWalkServo tiltServo(tiltPwm);
    xwalk::hal::XWalkAdc adc0(i2c, "A0", 0x14U);
    xwalk::hal::XWalkAdc adc1(i2c, "A1", 0x14U);
    xwalk::hal::XWalkAdc adc2(i2c, "A2", 0x14U);
    xwalk::hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
    xwalk::hal::XWalkUltrasonic ultrasonic(trigger, echo, 0U);
    xwalk::hal::XWalkConfigStore config(configPath);
    config.set("picarx_max_motor_output_percent", "100");
    config.set("picarx_calibration_verified", "true");
    xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo, tiltServo,
        grayscale, ultrasonic, config);
    TestCliBackend backend;
    backend.motors = &motors;
    backend.picarx = &picarx;
    const xwalk::agent::XWalkControllerCallbacks callbacks{&outputLine, &inputLine,
        &delayMilliseconds, &continueOperation, &soundOperation};
    xwalk::agent::XWalkLineTracking lineTracking(picarx, &backend, &delayMilliseconds);
    const XWalkHal::XWalkMusicCallbacks musicCallbacks{&enableMusicOutput, &playMusicSound,
        &playMusicSound, &playMusicFile, &setMusicVolume, &controlMusic, &controlMusic,
        &controlMusic, &musicSoundLength, &playMusicTone};
    XWalkHal::XWalkMusic music(&backend, musicCallbacks);
    xwalk::agent::XWalkSelfDrive selfDrive(picarx, music, &backend,
        &selfDriveDelayMilliseconds, nullptr, XWALK_TEST_SOUND_DIRECTORY);
    xwalk::agent::XWalkController cli(picarx, &backend, callbacks);
    xwalk::agent::XWalkController lineCli(picarx, lineTracking, &backend, callbacks);
    xwalk::agent::XWalkController selfDriveCli(picarx, selfDrive, &backend, callbacks);
    XWalkHal::XWalkSpi spi(nullptr, &transferSpi);
    xwalk::agent::XWalkSpiTransfer spiTransfer(spi);
    xwalk::agent::XWalkController spiCli(spiTransfer, &backend, callbacks);
    const XWalkHal::stringvector passingDoctorReport{
        "=== PiCar-X Passive Hardware Preflight ===", "[PASS] Configuration: ready"};
    xwalk::agent::XWalkController doctorCli(passingDoctorReport, &backend, callbacks);
    const XWalkHal::stringvector failingDoctorReport{"[FAIL] I2C: unavailable"};
    xwalk::agent::XWalkController failingDoctorCli(failingDoctorReport, &backend, callbacks);
    const XWalkHal::stringvector selfDriveActions{
        "shake-head", "nod", "wave-hands", "resist", "act-cute", "rub-hands",
        "think", "twist-body", "celebrate", "depressed", "forward", "backward",
        "honking", "start-engine"};

    assert(cli.run({"-h"}) == 0);
    assert(backend.outputLines.back() == xwalk::agent::XWalkController::usage());
    assert(backend.outputLines.back().find("Commands:\n") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("Examples:\n") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("move forward --speed 40") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("line-track <start|stop>") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("self-drive <action-name>") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("voice-chat") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("spi transfer <HEX>") != XWalkHal::string::npos);
    assert(backend.outputLines.back().find("doctor") != XWalkHal::string::npos);
    for (const XWalkHal::string& action : selfDriveActions)
    {
        assert(backend.outputLines.back().find("  " + action) != XWalkHal::string::npos);
    }
    assert(doctorCli.run({"doctor"}) == 0);
    assert(backend.outputLines.back() == "[PASS] Configuration: ready");
    assert(failingDoctorCli.run({"doctor"}) == 2);
    assert(backend.outputLines.back() == "[FAIL] I2C: unavailable");
    assert(cli.run({"line-track", "stop"}) == 3);
    assert(backend.outputLines.back() == "Line-tracking backend unavailable");
    assert(cli.run({"self-drive", "nod"}) == 3);
    assert(backend.outputLines.back() == "Self-drive backend unavailable");
    assert(cli.run({"voice-chat", "start"}) == 3);
    assert(backend.outputLines.back() == "Local voice-chatbot backend unavailable");
    assert(cli.run({"voice-chat", "stop"}) == 3);
    assert(backend.outputLines.back() == "Local voice-chatbot backend unavailable");
    const XWalkHal::stringvector voiceCarCommands{"voice-active-car",
        "voice-active-car-gpt"};
    for (const XWalkHal::string& command : voiceCarCommands)
    {
        assert(cli.run({command, "start"}) == 3);
        assert(backend.outputLines.back() == "Voice-active-car backend unavailable");
        assert(cli.run({command, "stop"}) == 3);
        assert(backend.outputLines.back() == "Voice-active-car backend unavailable");
    }
    assert(cli.run({"voice-controlled-car", "start"}) == 3);
    assert(backend.outputLines.back() == "Voice-controlled-car backend unavailable");
    assert(cli.run({"voice-controlled-car", "stop"}) == 3);
    assert(backend.outputLines.back() == "Voice-controlled-car backend unavailable");
    assert(cli.run({"voice-prompt-car", "start"}) == 3);
    assert(backend.outputLines.back() == "Voice-prompt-car backend unavailable");
    assert(cli.run({"voice-prompt-car", "stop"}) == 3);
    assert(backend.outputLines.back() == "Voice-prompt-car backend unavailable");
    assert(cli.run({"spi", "transfer", "9F000000"}) == 3);
    assert(backend.outputLines.back() == "SPI backend unavailable");
    assert(spiCli.run({"spi", "transfer", "0x9f00a5"}) == 0);
    assert(backend.outputLines.back() == "60 FF 5A");

    XWalkHal::size delayStart = backend.delays.size();
    assert(cli.run({"move", "forward", "--speed", "40", "--duration=0.25"}) == 0);
    XWalkHal::uint32 delayedMs{};
    for (XWalkHal::size index = delayStart; index < backend.delays.size(); ++index)
    {
        assert(backend.delays[index] <= 20U);
        delayedMs += backend.delays[index];
        assert(backend.leftSpeeds[index] == 70.0);
        assert(backend.rightSpeeds[index] == 70.0);
    }
    assert(delayedMs == 250U);
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    delayStart = backend.delays.size();
    assert(cli.run({"turn", "left", "--angle", "20"}) == 0);
    XWalkHal::boolean observedSteering{false};
    XWalkHal::boolean observedMovement{false};
    XWalkHal::boolean observedCenteredSteering{false};
    delayedMs = 0U;
    for (XWalkHal::size index = delayStart; index < backend.delays.size(); ++index)
    {
        delayedMs += backend.delays[index];
        observedSteering = observedSteering || (backend.steeringAngles[index] == -20.0);
        observedMovement = observedMovement || (backend.leftSpeeds[index] != 0.0);
        observedCenteredSteering = observedCenteredSteering ||
            (observedMovement && (backend.steeringAngles[index] == 0.0));
    }
    assert(delayedMs == 1'400U);
    assert(observedSteering);
    assert(observedMovement);
    assert(observedCenteredSteering);

    backend.operationQueryLimit = backend.operationQueries + 2U;
    delayStart = backend.delays.size();
    assert(cli.run({"move", "forward", "--speed", "40", "--duration", "1"}) == 0);
    delayedMs = 0U;
    for (XWalkHal::size index = delayStart; index < backend.delays.size(); ++index)
    {
        delayedMs += backend.delays[index];
    }
    assert(delayedMs <= 20U);
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);
    backend.operationQueryLimit = 1'000'000U;

    assert(cli.run({"cam", "pan", "--angle", "60"}) == 0);
    assert(cli.run({"sensor", "distance"}) == 0);
    assert(backend.outputLines.back() == "-1.0");
    assert(cli.run({"sensor", "grayscale"}) == 0);
    assert(config.get("line_reference") == "[1000,1000,1000]");

    const XWalkHal::uint32 lineQueryStart = backend.operationQueries;
    backend.operationQueryLimit = lineQueryStart + 2U;
    assert(lineCli.run({"line-track", "start"}) == 0);
    assert(backend.operationQueries == (lineQueryStart + 3U));
    backend.operationQueryLimit = 1'000'000U;
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);
    assert(backend.outputLines.back() == "Line tracking stopped");
    assert(lineCli.run({"line-track", "stop"}) == 0);

    for (const XWalkHal::string& action : selfDriveActions)
    {
        assert(selfDriveCli.run({"self-drive", action}) == 0);
    }
    assert(backend.musicSoundFile ==
        (XWalkHal::filesystempath(XWALK_TEST_SOUND_DIRECTORY) /
            "car-start-engine.wav").lexically_normal().string());
    assert(selfDriveCli.run({"self-drive", "wave", "hands"}) == 0);

    assert(cli.run({"sound", "play", "../xWalkSounds/car-double-horn.wav", "--volume", "80"}) == 0);
    assert(backend.soundOperation == xwalk::agent::XWalkSoundOperation::Play);
    assert(backend.soundFile == "../xWalkSounds/car-double-horn.wav");
    assert(backend.soundVolume.has_value() && (*backend.soundVolume == 80.0));
    assert(cli.run({"sound", "music",
        "../xWalkMusics/slow-trail-Ahjay_Stelino.mp3"}) == 0);
    assert(backend.soundOperation == xwalk::agent::XWalkSoundOperation::Music);
    assert(backend.soundFile == "../xWalkMusics/slow-trail-Ahjay_Stelino.mp3");
    assert(backend.soundVolume.has_value() && (*backend.soundVolume == 20.0));
    backend.soundAvailable = false;
    assert(cli.run({"sound", "stop"}) == 3);
    assert(backend.outputLines.back() == "Sound backend unavailable");

    backend.inputLines = {"skip", "skip", "skip", "10", "raised", "y", "y", "y", "y",
        "sample", "y", "sample", "y"};
    assert(cli.run({"calibrate"}) == 0);
    assert(backend.outputLines.back() == "Calibration complete!");
    assert(config.get("picarx_calibration_verified") == "true");
    assert(config.get("picarx_motor_speed_calibration") == "10.000000");
    assert(config.get("line_reference") == "[1000,1000,1000]");
    assert(config.get("cliff_reference") == "[1000,1000,1000]");
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    picarx.clearEmergencyStop();
    picarx.forward(40.0);
    backend.failSound = true;
    assert(cli.run({"sound", "stop"}) == 3);
    backend.failSound = false;
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(cli.run({"move", "forward", "--speed", "101"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(lineCli.run({"line-track", "run"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(selfDriveCli.run({"self-drive", "unknown"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(cli.run({"voice-chat", "run"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(cli.run({"voice-active-car-gpt", "run"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(cli.run({"voice-controlled-car", "run"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(cli.run({"voice-prompt-car", "run"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(spiCli.run({"spi", "transfer", "ABC"}));
    });
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(spiCli.run({"spi", "transfer", "GG"}));
    });
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all PiCar-X CLI host-test scenarios.
 * @param[in] argumentCount Must equal two so one test configuration path is available.
 * @param[in] arguments Non-owning process argument array containing the test path at index one.
 * @return Zero when every assertion passes; one when the path is absent.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }
    const XWalkHal::filesystempath configPath(arguments[1]);
    XWalkHal::filesystempath replacementPath = configPath;
    replacementPath += ".tmp";
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    testCommands(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath.parent_path()));
    return 0;
}
