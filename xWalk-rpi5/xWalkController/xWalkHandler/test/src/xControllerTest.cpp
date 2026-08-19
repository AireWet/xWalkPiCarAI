/******************************************************************************
 * @file        xControllerTest.cpp
 * @brief       Verifies PiCar-X CLI commands with in-memory backends.
 *
 * @details
 * Exercises parsing, movement, line tracking, preset actions, sensors, audio
 * dispatch, and calibration prompts.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler Host Test
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

#include "xController.h"
#include "xControllerCommands.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xControllerTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestBus = ::xwalk::source_types::xcontrollertest::TestBus;
using TestGpio = ::xwalk::source_types::xcontrollertest::TestGpio;
using TestCliBackend = ::xwalk::source_types::xcontrollertest::TestCliBackend;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and callbacks private to this translation unit. */
namespace
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /******************************************************************************
     * Private callback definitions
     ******************************************************************************/

    ctrl::boolean probe(ctrl::contextpointer context, ctrl::uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }

    void writeRegister(ctrl::contextpointer context, ctrl::uint8 address, ctrl::uint8 reg, const ctrl::bytevector& data)
    {
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        static_cast<void>(context);
    }

    /** @brief Accepts one non-throwing simulated fail-safe write. */
    ctrl::boolean tryWriteRegister(ctrl::contextpointer context,
                                   ctrl::uint8 address,
                                   ctrl::uint8 reg,
                                   const ctrl::bytevector& data) noexcept
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        return true;
    }

    ctrl::bytevector read(ctrl::contextpointer context, ctrl::uint8 address, ctrl::size length)
    {
        static_cast<void>(address);
        static_cast<void>(length);
        return static_cast<TestBus*>(context)->sample;
    }

    /** @brief Returns each transmitted SPI byte with every bit inverted. */
    ctrl::bytevector transferSpi(ctrl::contextpointer context, const ctrl::bytevector& transmitData)
    {
        static_cast<void>(context);
        ctrl::bytevector response;
        response.reserve(transmitData.size());
        for (const ctrl::uint8 value : transmitData)
        {
            response.push_back(static_cast<ctrl::uint8>(value ^ 0xFFU));
        }
        return response;
    }

    void configureGpio(ctrl::contextpointer context,
                       ctrl::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       ctrl::boolean initialValue)
    {
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<TestGpio*>(context)->value = initialValue;
    }

    ctrl::boolean readGpio(ctrl::contextpointer context, ctrl::uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<TestGpio*>(context)->value;
    }

    void writeGpio(ctrl::contextpointer context, ctrl::uint8 pin, ctrl::boolean value)
    {
        static_cast<void>(pin);
        static_cast<TestGpio*>(context)->value = value;
    }

    void interruptGpio(ctrl::contextpointer context,
                       ctrl::uint8 pin,
                       XWalkHal::XWalkGpioEdge edge,
                       ctrl::uint32 debounceMs,
                       ctrl::contextpointer handlerContext,
                       XWalkHal::gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }

    void cancelInterrupt(ctrl::contextpointer context, ctrl::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    XWalkHal::XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterrupt};
    }

    void outputLine(ctrl::contextpointer context, ctrl::stringview line)
    {
        static_cast<TestCliBackend*>(context)->outputLines.emplace_back(line);
    }

    ctrl::string inputLine(ctrl::contextpointer context, ctrl::stringview prompt)
    {
        static_cast<void>(prompt);
        TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
        const ::ctrl::boolean inputUnavailable =
            static_cast<::ctrl::boolean>(backend.inputIndex >= backend.inputLines.size());
        if (inputUnavailable)
        {
            return "skip";
        }
        const ctrl::string response = backend.inputLines[backend.inputIndex];
        ++backend.inputIndex;
        return response;
    }

    void delayMilliseconds(ctrl::contextpointer context, ctrl::uint32 durationMs)
    {
        TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
        backend.delays.push_back(durationMs);
        backend.monotonicMilliseconds += static_cast<::ctrl::uint64>(durationMs) + backend.delayOverrunMs;
        backend.leftSpeeds.push_back(backend.motors->left().speed());
        backend.rightSpeeds.push_back(backend.motors->right().speed());
        backend.steeringAngles.push_back(backend.picarx->directionAngleDegrees());
    }

    ::ctrl::uint64 monotonicMilliseconds(ctrl::contextpointer context) noexcept
    {
        return static_cast<TestCliBackend*>(context)->monotonicMilliseconds;
    }

    /** @brief Records one self-drive delay and reports explicit failure status. */
    ctrl::boolean selfDriveDelayMilliseconds(ctrl::contextpointer context, ctrl::uint32 durationMs) noexcept
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
    ctrl::boolean continueOperation(ctrl::contextpointer context)
    {
        TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
        const ctrl::boolean shouldContinue = backend.operationQueries < backend.operationQueryLimit;
        ++backend.operationQueries;
        return shouldContinue;
    }

    ctrl::boolean soundOperation(ctrl::contextpointer context, const xwalk::ctrl::XWalkSoundRequest& request)
    {
        TestCliBackend& backend = *static_cast<TestCliBackend*>(context);
        backend.soundRequest = request;
        return backend.soundAvailable && (!backend.failSound);
    }

    /**
     * @brief Accepts simulated music-output enablement without hardware access.
     * @param[in,out] context Optional test context; unused.
     */
    void enableMusicOutput(ctrl::contextpointer context)
    {
        static_cast<void>(context);
    }

    /**
     * @brief Records one simulated synchronous or background sound request.
     * @param[in,out] context Non-null test backend receiving the file name.
     * @param[in] filename Sound path copied for later assertions.
     * @param[in] normalizedVolume Optional normalized volume; unused by this test backend.
     */
    void playMusicSound(ctrl::contextpointer context, ctrl::stringview filename, ctrl::optionalfloat64 normalizedVolume)
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
    void playMusicFile(ctrl::contextpointer context,
                       ctrl::stringview filename,
                       ctrl::int32 loops,
                       ctrl::float64 startSeconds)
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
    void setMusicVolume(ctrl::contextpointer context, ctrl::float64 normalizedVolume)
    {
        static_cast<void>(context);
        static_cast<void>(normalizedVolume);
    }

    /**
     * @brief Accepts one simulated music transport request.
     * @param[in,out] context Optional test context; unused.
     */
    void controlMusic(ctrl::contextpointer context)
    {
        static_cast<void>(context);
    }

    /**
     * @brief Supplies a deterministic simulated sound length.
     * @param[in,out] context Optional test context; unused.
     * @param[in] filename Sound path; unused.
     * @return One second.
     */
    ctrl::float64 musicSoundLength(ctrl::contextpointer context, ctrl::stringview filename)
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
    void playMusicTone(ctrl::contextpointer context,
                       const ctrl::bytevector& pcmData,
                       ctrl::uint32 sampleRateHz,
                       ctrl::uint8 channelCount)
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
    void testCommands(ctrl::stringview configPath)
    {
        TestBus bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr, &tryWriteRegister);
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
        xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo, tiltServo, grayscale, ultrasonic, config);
        static_cast<void>(picarx.initialize());
        TestCliBackend backend;
        backend.motors = &motors;
        backend.picarx = &picarx;
        const xwalk::ctrl::XWalkControllerCallbacks callbacks{
            &outputLine, &inputLine, &delayMilliseconds, &monotonicMilliseconds, &continueOperation, &soundOperation};
        xwalk::agent::XWalkLineTracking lineTracking(picarx, &backend, &delayMilliseconds);
        const XWalkHal::XWalkMusicCallbacks musicCallbacks{&enableMusicOutput,
                                                           &playMusicSound,
                                                           &playMusicSound,
                                                           &playMusicFile,
                                                           &setMusicVolume,
                                                           &controlMusic,
                                                           &controlMusic,
                                                           &controlMusic,
                                                           &musicSoundLength,
                                                           &playMusicTone};
        XWalkHal::XWalkMusic music(&backend, musicCallbacks);
        xwalk::agent::XWalkSelfDrive selfDrive(picarx,
                                               music,
                                               &backend,
                                               &selfDriveDelayMilliseconds,
                                               nullptr,
                                               XWALK_TEST_SOUND_DIRECTORY,
                                               XWALK_TEST_MUSIC_DIRECTORY);
        xwalk::agent::XWalkGrayscaleCalibration grayscaleCalibration(
            picarx, &backend, &delayMilliseconds, &continueOperation);
        xwalk::agent::XWalkServoMotorCalibration servoMotorCalibration(
            picarx, &backend, &delayMilliseconds, &continueOperation);
        xwalk::agent::XWalkMoveExample moveExample(picarx, &backend, &delayMilliseconds, &continueOperation);
        xwalk::agent::XWalkKeyboardControl keyboardControl(picarx, &backend, &delayMilliseconds, &continueOperation);
        xwalk::agent::XWalkObstacleAvoidance obstacleAvoidance(
            picarx, &backend, &delayMilliseconds, &continueOperation);
        xwalk::agent::XWalkCliffDetection cliffDetection(picarx, &backend, &delayMilliseconds, &continueOperation);
        xwalk::ctrl::XWalkController cli(picarx,
                                         grayscaleCalibration,
                                         servoMotorCalibration,
                                         moveExample,
                                         keyboardControl,
                                         obstacleAvoidance,
                                         cliffDetection,
                                         &backend,
                                         callbacks);
        xwalk::ctrl::XWalkController lineCli(picarx, lineTracking, &backend, callbacks);
        xwalk::ctrl::XWalkController selfDriveCli(picarx, selfDrive, &backend, callbacks);
        XWalkHal::XWalkSpi spi(nullptr, &transferSpi);
        xwalk::agent::XWalkSpiTransfer spiTransfer(spi);
        xwalk::ctrl::XWalkController spiCli(spiTransfer, &backend, callbacks);
        const ctrl::stringvector passingDoctorReport{"=== PiCar-X Bounded Hardware Preflight ===",
                                                     "[PASS] Configuration: ready",
                                                     "[WARN] Optional advisory: incomplete"};
        xwalk::ctrl::XWalkController doctorCli(passingDoctorReport, &backend, callbacks);
        const ctrl::stringvector failingDoctorReport{"[FAIL] I2C: unavailable"};
        xwalk::ctrl::XWalkController failingDoctorCli(failingDoctorReport, &backend, callbacks);
        const ctrl::stringvector selfDriveActions{"shake-head",
                                                  "nod",
                                                  "wave-hands",
                                                  "resist",
                                                  "act-cute",
                                                  "rub-hands",
                                                  "think",
                                                  "twist-body",
                                                  "celebrate",
                                                  "depressed",
                                                  "forward",
                                                  "backward",
                                                  "honking",
                                                  "start-engine",
                                                  "play-background-music",
                                                  "stop-background-music"};

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"-h"}) == 0);
        assert(backend.outputLines.back() == xwalk::ctrl::XWALK_controllerUsage());
        assert(backend.outputLines.back().find("Commands:\n") != ctrl::string::npos);
        assert(backend.outputLines.back().find("Examples:\n") != ctrl::string::npos);
        assert(backend.outputLines.back().find("move forward --speed 40") != ctrl::string::npos);
        assert(backend.outputLines.back().find("line-track <start|stop>") != ctrl::string::npos);
        assert(backend.outputLines.back().find("self-drive <action-name>") != ctrl::string::npos);
        assert(backend.outputLines.back().find("voice-chat") != ctrl::string::npos);
        assert(backend.outputLines.back().find("spi transfer <HEX>") != ctrl::string::npos);
        assert(backend.outputLines.back().find("doctor") != ctrl::string::npos);
        for (const ctrl::string& action : selfDriveActions)
        {
            assert(backend.outputLines.back().find("  " + action) != ctrl::string::npos);
        }
        assert(xwalk::ctrl::XWALK_runControllerCommand(doctorCli, {"doctor"}) == 0);
        assert(backend.outputLines.at(backend.outputLines.size() - 3U) == "=== PiCar-X Bounded Hardware Preflight ===");
        assert(backend.outputLines.back() == "[WARN] Optional advisory: incomplete");

        assert(xwalk::ctrl::XWALK_runControllerCommand(failingDoctorCli, {"doctor"}) == 2);
        assert(backend.outputLines.back() == "[FAIL] I2C: unavailable");

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"line-track", "stop"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"self-drive", "nod"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-chat", "start"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-chat", "stop"}) == 3);

        const ctrl::stringvector voiceCarCommands{"voice-active-car", "voice-active-car-gpt"};
        for (const ctrl::string& command : voiceCarCommands)
        {
            assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {command, "start"}) == 3);

            assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {command, "stop"}) == 3);
        }
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-controlled-car", "start"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-controlled-car", "stop"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-prompt-car", "start"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-prompt-car", "stop"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"text-vision-talk", "start"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"text-vision-talk", "stop"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"online-llm-test", "start"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"online-llm-test", "stop"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"spi", "transfer", "9F000000"}) == 3);

        assert(xwalk::ctrl::XWALK_runControllerCommand(spiCli, {"spi", "transfer", "0x9f00a5"}) == 0);

        ctrl::size delayStart = backend.delays.size();
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"move", "forward", "--speed", "40", "--duration=0.25"}) ==
               0);
        ctrl::uint32 delayedMs{};
        for (ctrl::size index = delayStart; index < backend.delays.size(); ++index)
        {
            assert(backend.delays[index] <= 20U);
            delayedMs += backend.delays[index];
            assert(backend.leftSpeeds[index] == 70.0);
            assert(backend.rightSpeeds[index] == 70.0);
        }
        assert(delayedMs == 250U);
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        backend.delayOverrunMs = 5U;
        const ::ctrl::uint64 moveStartMs = backend.monotonicMilliseconds;
        delayStart = backend.delays.size();
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"move", "backward", "--speed", "40", "--duration=0.6"}) ==
               0);
        delayedMs = 0U;
        for (ctrl::size index = delayStart; index < backend.delays.size(); ++index)
        {
            delayedMs += backend.delays[index];
        }
        const ::ctrl::uint64 moveElapsedMs = backend.monotonicMilliseconds - moveStartMs;
        assert(delayedMs < 600U);
        assert(moveElapsedMs >= 600U);
        assert(moveElapsedMs <= 625U);
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
        backend.delayOverrunMs = 0U;

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"cliff-detection", "stop"}) == 0);
        const ctrl::uint32 cliffQueryStart = backend.operationQueries;
        backend.operationQueryLimit = cliffQueryStart + 2U;
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"cliff-detection", "start"}) == 0);
        assert(backend.operationQueries == (cliffQueryStart + 3U));
        backend.operationQueryLimit = 1'000'000U;

        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"avoid-obstacles", "stop"}) == 0);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"avoid-obstacles", "start"}) == 2);

        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        delayStart = backend.delays.size();
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"turn", "left", "--angle", "20"}) == 0);
        ctrl::boolean observedSteering{false};
        ctrl::boolean observedMovement{false};
        ctrl::boolean observedCenteredSteering{false};
        delayedMs = 0U;
        for (ctrl::size index = delayStart; index < backend.delays.size(); ++index)
        {
            delayedMs += backend.delays[index];
            observedSteering = observedSteering || (backend.steeringAngles[index] == -20.0);
            observedMovement = observedMovement || (backend.leftSpeeds[index] != 0.0);
            observedCenteredSteering =
                observedCenteredSteering || (observedMovement && (backend.steeringAngles[index] == 0.0));
        }
        assert(delayedMs == 1'400U);
        assert(observedSteering);
        assert(observedMovement);
        assert(observedCenteredSteering);

        backend.operationQueryLimit = backend.operationQueries + 2U;
        delayStart = backend.delays.size();
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"move", "forward", "--speed", "40", "--duration", "1"}) ==
               0);
        delayedMs = 0U;
        for (ctrl::size index = delayStart; index < backend.delays.size(); ++index)
        {
            delayedMs += backend.delays[index];
        }
        assert(delayedMs <= 20U);
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
        backend.operationQueryLimit = 1'000'000U;

        delayStart = backend.delays.size();
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"move", "demo"}) == 0);
        delayedMs = 0U;
        for (ctrl::size index = delayStart; index < backend.delays.size(); ++index)
        {
            delayedMs += backend.delays[index];
        }
        assert((backend.delays.size() - delayStart) == 505U);
        assert(delayedMs == 5'900U);
        assert(picarx.directionAngleDegrees() == -1.0);
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        backend.inputLines = {"w", "a", "s", "d", "i", "k", "j", "l", "x", "q"};
        backend.inputIndex = 0U;
        delayStart = backend.delays.size();
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"keyboard-control"}) == 0);
        assert((backend.delays.size() - delayStart) == 201U);

        assert(keyboardControl.panAngleDegrees() == 0.0);
        assert(keyboardControl.tiltAngleDegrees() == 0.0);
        assert(picarx.directionAngleDegrees() == 0.0);
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"cam", "pan", "--angle", "60"}) == 0);
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sensor", "distance"}) == 0);

        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sensor", "grayscale"}) == 0);
        assert(config.get("line_reference") == "[1000,1000,1000]");

        const ctrl::uint32 lineQueryStart = backend.operationQueries;
        backend.operationQueryLimit = lineQueryStart + 2U;
        assert(xwalk::ctrl::XWALK_runControllerCommand(lineCli, {"line-track", "start"}) == 0);
        assert(backend.operationQueries == (lineQueryStart + 3U));
        backend.operationQueryLimit = 1'000'000U;
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        assert(xwalk::ctrl::XWALK_runControllerCommand(lineCli, {"line-track", "stop"}) == 0);

        for (const ctrl::string& action : selfDriveActions)
        {
            assert(xwalk::ctrl::XWALK_runControllerCommand(selfDriveCli, {"self-drive", action}) == 0);
        }
        assert(backend.musicSoundFile ==
               (ctrl::filesystempath(XWALK_TEST_SOUND_DIRECTORY) / "car-start-engine.wav").lexically_normal().string());
        assert(xwalk::ctrl::XWALK_runControllerCommand(selfDriveCli, {"self-drive", "wave", "hands"}) == 0);

        assert(
            xwalk::ctrl::XWALK_runControllerCommand(
                cli,
                {"sound", "play", "../xWalk-rpi5/xWalkAudioResources/sounds/car-double-horn.wav", "--volume", "80"}) ==
            0);
        assert(backend.soundRequest.operation == xwalk::ctrl::XWalkSoundOperation::Play);
        assert(backend.soundRequest.filePath == "../xWalk-rpi5/xWalkAudioResources/sounds/car-double-horn.wav");
        assert(backend.soundRequest.volumePercent.has_value() && (*backend.soundRequest.volumePercent == 80.0));
        assert(xwalk::ctrl::XWALK_runControllerCommand(
                   cli, {"sound", "music", "../xWalk-rpi5/xWalkAudioResources/music/slow-trail-Ahjay_Stelino.mp3"}) ==
               0);
        assert(backend.soundRequest.operation == xwalk::ctrl::XWalkSoundOperation::Music);
        assert(backend.soundRequest.filePath == "../xWalk-rpi5/xWalkAudioResources/music/slow-trail-Ahjay_Stelino.mp3");
        assert(backend.soundRequest.volumePercent.has_value() && (*backend.soundRequest.volumePercent == 20.0));
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sound", "volume", "0"}) == 0);
        assert(backend.soundRequest.operation == xwalk::ctrl::XWalkSoundOperation::Volume);
        assert(backend.soundRequest.volumePercent.has_value() && (*backend.soundRequest.volumePercent == 0.0));
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sound", "volume", "100"}) == 0);
        assert(backend.soundRequest.volumePercent.has_value() && (*backend.soundRequest.volumePercent == 100.0));
        backend.soundAvailable = false;
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sound", "stop"}) == 3);

        backend.inputLines = {"ready", "q", "e", "y"};
        backend.inputIndex = 0U;
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"calibrate", "grayscale"}) == 0);

        assert(config.get("line_reference") == "[1000,1000,1000]");
        assert(config.get("cliff_reference") == "[1000,1000,1000]");
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        backend.inputLines = {
            "skip", "5", "y", "-2", "y", "3", "y", "10", "1", "-1", "raised", "y", "y", "y", "y", "y"};
        backend.inputIndex = 0U;
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"calibrate", "servo-motor"}) == 0);

        assert(config.get("picarx_dir_servo") == "5.000000");
        assert(config.get("picarx_cam_pan_servo") == "-2.000000");
        assert(config.get("picarx_cam_tilt_servo") == "3.000000");
        assert(config.get("picarx_dir_motor") == "[1,-1]");
        assert(config.get("picarx_calibration_verified") == "true");
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        backend.inputLines = {
            "skip", "skip", "skip", "skip", "10", "raised", "y", "y", "y", "y", "y", "ready", "q", "e", "y"};
        backend.inputIndex = 0U;
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"calibrate"}) == 0);

        assert(config.get("picarx_calibration_verified") == "true");
        assert(config.get("picarx_motor_speed_calibration") == "10.000000");
        assert(config.get("line_reference") == "[1000,1000,1000]");
        assert(config.get("cliff_reference") == "[1000,1000,1000]");
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        picarx.clearEmergencyStop();
        picarx.forward(40.0);
        backend.failSound = true;
        assert(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sound", "stop"}) == 3);
        backend.failSound = false;
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"move", "forward", "--speed", "101"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sound", "volume", "-1"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"sound", "volume", "101"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(lineCli, {"line-track", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(selfDriveCli, {"self-drive", "unknown"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-chat", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-active-car-gpt", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-controlled-car", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"voice-prompt-car", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"text-vision-talk", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"online-llm-test", "run"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(cli, {"calibrate", "unknown"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(spiCli, {"spi", "transfer", "ABC"}));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(xwalk::ctrl::XWALK_runControllerCommand(spiCli, {"spi", "transfer", "GG"}));
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
ctrl::int32 main(ctrl::int32 argumentCount, ctrl::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }
    const ctrl::filesystempath configPath(arguments[1]);
    ctrl::filesystempath replacementPath = configPath;
    replacementPath += ".tmp";
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    testCommands(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath.parent_path()));
    return 0;
}
