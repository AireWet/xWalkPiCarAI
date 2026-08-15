/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveTest.cpp
 * @brief       Verifies preset actions with in-memory hardware and audio.
 *
 * @details
 * Covers gesture dispatch, movement completion, sound routing, validation,
 * status transitions, and background first-in, first-out action processing.
 *
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive Host Test
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

#include "xAgent_Rpi5CarSelfDrive.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xAgent_Rpi5CarSelfDriveTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestBus = ::xwalk::source_types::xagent_rpi5carselfdrivetest::TestBus;
using TestGpio = ::xwalk::source_types::xagent_rpi5carselfdrivetest::TestGpio;
using TestBackend = ::xwalk::source_types::xagent_rpi5carselfdrivetest::TestBackend;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains deterministic test state and callbacks.
 */
namespace
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /******************************************************************************
     * Private callback definitions
     ******************************************************************************/

    /** @brief Accepts every simulated I2C address. */
    agent::boolean probe(agent::contextpointer context, agent::uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }

    /** @brief Accepts one simulated I2C register write. */
    void
    writeRegister(agent::contextpointer context, agent::uint8 address, agent::uint8 reg, const agent::bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }

    /** @brief Accepts one non-throwing simulated fail-safe write. */
    agent::boolean tryWriteRegister(agent::contextpointer context,
                                    agent::uint8 address,
                                    agent::uint8 reg,
                                    const agent::bytevector& data) noexcept
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
        return true;
    }

    /** @brief Returns the deterministic two-byte ADC sample. */
    agent::bytevector readBus(agent::contextpointer context, agent::uint8 address, agent::size length)
    {
        static_cast<void>(address);
        static_cast<void>(length);
        return static_cast<TestBus*>(context)->sample;
    }

    /** @brief Stores the configured simulated GPIO level. */
    void configureGpio(agent::contextpointer context,
                       agent::uint8 pin,
                       XWalkHal::XWalkGpioMode mode,
                       XWalkHal::XWalkGpioPull pull,
                       agent::boolean initialValue)
    {
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<TestGpio*>(context)->value = initialValue;
    }

    /** @brief Returns one simulated GPIO level. */
    agent::boolean readGpio(agent::contextpointer context, agent::uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<TestGpio*>(context)->value;
    }

    /** @brief Stores one simulated GPIO output level. */
    void writeGpio(agent::contextpointer context, agent::uint8 pin, agent::boolean value)
    {
        static_cast<void>(pin);
        static_cast<TestGpio*>(context)->value = value;
    }

    /** @brief Accepts one simulated GPIO interrupt registration. */
    void interruptGpio(agent::contextpointer context,
                       agent::uint8 pin,
                       XWalkHal::XWalkGpioEdge edge,
                       agent::uint32 debounceMs,
                       agent::contextpointer handlerContext,
                       XWalkHal::gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }

    /** @brief Accepts one simulated GPIO interrupt cancellation. */
    void cancelInterrupt(agent::contextpointer context, agent::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }

    /** @brief Returns the complete simulated GPIO callback table. */
    XWalkHal::XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterrupt};
    }

    /** @brief Records that the audio output was enabled. */
    void enableOutput(agent::contextpointer context)
    {
        static_cast<TestBackend*>(context)->outputEnabled = true;
    }

    /** @brief Accepts one synchronous sound request. */
    void playSound(agent::contextpointer context, agent::stringview filename, agent::optionalfloat64 volume)
    {
        static_cast<void>(context);
        static_cast<void>(filename);
        static_cast<void>(volume);
    }

    /** @brief Records one asynchronous preset sound request. */
    void playSoundBackground(agent::contextpointer context, agent::stringview filename, agent::optionalfloat64 volume)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.backgroundFiles.emplace_back(filename);
        backend.backgroundVolumes.push_back(volume.has_value() ? *volume : -1.0);
    }

    /** @brief Accepts one unused streamed-music request. */
    void playMusic(agent::contextpointer context,
                   agent::stringview filename,
                   agent::int32 loops,
                   agent::float64 startSeconds)
    {
        static_cast<void>(context);
        static_cast<void>(filename);
        static_cast<void>(loops);
        static_cast<void>(startSeconds);
    }

    /** @brief Accepts one unused music-volume request. */
    void setMusicVolume(agent::contextpointer context, agent::float64 volume)
    {
        static_cast<void>(context);
        static_cast<void>(volume);
    }

    /** @brief Accepts one unused music-control request. */
    void controlMusic(agent::contextpointer context)
    {
        static_cast<void>(context);
    }

    /** @brief Returns a deterministic zero-second sound length. */
    agent::float64 soundLength(agent::contextpointer context, agent::stringview filename)
    {
        static_cast<void>(context);
        static_cast<void>(filename);
        return 0.0;
    }

    /** @brief Accepts one unused generated-tone request. */
    void playTone(agent::contextpointer context,
                  const agent::bytevector& data,
                  agent::uint32 sampleRateHz,
                  agent::uint8 channelCount)
    {
        static_cast<void>(context);
        static_cast<void>(data);
        static_cast<void>(sampleRateHz);
        static_cast<void>(channelCount);
    }

    /** @brief Records one requested preset-action delay without sleeping. */
    agent::boolean delayMilliseconds(agent::contextpointer context, agent::uint32 durationMs) noexcept
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.delays.push_back(durationMs);
        const agent::boolean backendFailDelayWhileMovingMotorsInvalid =
            static_cast<agent::boolean>(backend.failDelayWhileMoving && ((backend.motors->left().speed() != 0.0) ||
                                                                         (backend.motors->right().speed() != 0.0)));
        if (backendFailDelayWhileMovingMotorsInvalid)
        {
            return false;
        }
        return true;
    }

    /** @brief Supplies a deterministic cancellation sequence for one preset action. */
    agent::boolean continueOperation(agent::contextpointer context)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        const agent::boolean shouldContinue = backend.continueQueries < backend.continueQueryLimit;
        ++backend.continueQueries;
        return shouldContinue;
    }

    /******************************************************************************
     * Test function definitions
     ******************************************************************************/

    /**
     * @brief Exercises the complete preset catalog and background queue.
     *
     * @param[in] configPath
     * Test-owned configuration path below the module build directory.
     */
    void testSelfDriveBehavior(agent::stringview configPath)
    {
        TestBus bus;
        xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &readBus, nullptr, &tryWriteRegister);
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

        TestBackend backend;
        backend.motors = &motors;
        const XWalkHal::XWalkMusicCallbacks musicCallbacks{&enableOutput,
                                                           &playSound,
                                                           &playSoundBackground,
                                                           &playMusic,
                                                           &setMusicVolume,
                                                           &controlMusic,
                                                           &controlMusic,
                                                           &controlMusic,
                                                           &soundLength,
                                                           &playTone};
        xwalk::hal::XWalkMusic music(&backend, musicCallbacks);
        xwalk::agent::XWalkSelfDrive selfDrive(
            picarx, music, &backend, &delayMilliseconds, nullptr, XWALK_TEST_SOUND_DIRECTORY);
        assert(backend.outputEnabled);

        agent::size delayCount = backend.delays.size();
        assert(selfDrive.doAction("wave hands"));
        assert(backend.delays.size() == (delayCount + 4U));
        assert(backend.delays[delayCount] == 100U);
        assert(backend.delays[delayCount + 3U] == 100U);

        delayCount = backend.delays.size();
        assert(selfDrive.doAction("forward"));
        assert(backend.delays.size() == (delayCount + 1U));
        assert(backend.delays[delayCount] == 1'000U);

        const agent::stringvector gestures{"shake head",
                                           "nod",
                                           "resist",
                                           "act cute",
                                           "rub hands",
                                           "think",
                                           "twist body",
                                           "celebrate",
                                           "depressed",
                                           "backward"};
        for (const agent::string& gesture : gestures)
        {
            assert(selfDrive.doAction(gesture));
        }
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
        assert(picarx.directionAngleDegrees() == 0.0);
        assert(!backend.delays.empty());

        picarx.forward(20.0);
        assert(selfDrive.doAction("stop"));
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        assert(selfDrive.doAction("honking"));
        assert(selfDrive.doAction("start engine"));
        assert(backend.backgroundFiles[0U] ==
               (agent::filesystempath(XWALK_TEST_SOUND_DIRECTORY) / "car-double-horn.wav").lexically_normal().string());
        assert(backend.backgroundVolumes[0U] == 1.0);
        assert(
            backend.backgroundFiles[1U] ==
            (agent::filesystempath(XWALK_TEST_SOUND_DIRECTORY) / "car-start-engine.wav").lexically_normal().string());
        assert(backend.backgroundVolumes[1U] == 0.5);
        assert(!selfDrive.doAction("unknown"));
        assert(!selfDrive.addAction("unknown"));

        xwalk::agent::XWalkSelfDrive missingSoundSelfDrive(
            picarx, music, &backend, &delayMilliseconds, nullptr, "/xwalk-test-missing-sounds");
        assert(!missingSoundSelfDrive.doAction("honking"));
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        selfDrive.setCancellation(&backend, &continueOperation);
        backend.continueQueryLimit = backend.continueQueries + 2U;
        delayCount = backend.delays.size();
        picarx.clearEmergencyStop();
        assert(selfDrive.doAction("forward"));
        agent::uint32 cancelledDelayMs{};
        for (agent::size index = delayCount; index < backend.delays.size(); ++index)
        {
            assert(backend.delays[index] <= 20U);
            cancelledDelayMs += backend.delays[index];
        }
        assert(cancelledDelayMs <= 40U);
        assert(picarx.emergencyStopRequested());
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
        backend.continueQueryLimit = 1'000'000U;
        picarx.clearEmergencyStop();

        xwalk::hal::test::expectFailure(
            [&]()
            {
                xwalk::agent::XWalkSelfDrive invalid(
                    picarx, music, &backend, nullptr, nullptr, XWALK_TEST_SOUND_DIRECTORY);
                static_cast<void>(invalid);
            });

        selfDrive.start();
        assert(selfDrive.running());
        assert(selfDrive.status() == xwalk::agent::XWalkSelfDriveStatus::Standby);
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
        assert(selfDrive.addAction("honking"));
        assert(selfDrive.addAction("backward"));
        assert(selfDrive.waitActionsDone());
        assert(selfDrive.status() == xwalk::agent::XWalkSelfDriveStatus::Standby);
        selfDrive.stop();
        assert(!selfDrive.running());
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);

        backend.failDelayWhileMoving = true;
        picarx.clearEmergencyStop();
        selfDrive.start();
        assert(selfDrive.addAction("forward"));
        const agent::boolean workerCompleted = selfDrive.waitActionsDone();
        selfDrive.stop();
        backend.failDelayWhileMoving = false;
        assert(!workerCompleted);
        assert(picarx.emergencyStopRequested());
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all self-drive host-test scenarios.
 *
 * @param[in] argumentCount
 * Must equal two so one test-owned configuration path is available.
 *
 * @param[in] arguments
 * Non-owning process argument array whose second entry is the configuration path.
 *
 * @return
 * Zero when every assertion passes; one when the required path is absent.
 */
agent::int32 main(agent::int32 argumentCount, agent::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }

    const agent::filesystempath configPath(arguments[1]);
    agent::filesystempath replacementPath = configPath;
    replacementPath += ".tmp";
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    testSelfDriveBehavior(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath.parent_path()));
    return 0;
}
