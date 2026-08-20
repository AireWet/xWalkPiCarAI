/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveTest.cpp
 * @brief       Verifies preset actions with in-memory hardware and audio.
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive Host Test
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarSelfDriveTestSupport.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

namespace
{

    namespace support = xwalk::agent::test::selfdrive;

    /** @brief Exercises the preset catalog, watchdog refresh, and background queue. */
    void testSelfDriveBehavior(agent::stringview configPath)
    {
        support::TestBackend backend;
        support::TestBus bus;
        xwalk::hal::XWalkI2c i2c(
            &bus, &support::probe, &support::writeRegister, &support::readBus, nullptr, &support::tryWriteRegister);
        xwalk::hal::XWalkPwmTimerState timerState;
        xwalk::hal::XWalkPwm leftPwm(i2c, "P13", 0x14U, timerState);
        xwalk::hal::XWalkPwm rightPwm(i2c, "P12", 0x14U, timerState);
        xwalk::hal::XWalkPwm directionPwm(i2c, "P2", 0x14U, timerState);
        xwalk::hal::XWalkPwm panPwm(i2c, "P0", 0x14U, timerState);
        xwalk::hal::XWalkPwm tiltPwm(i2c, "P1", 0x14U, timerState);
        support::TestGpio leftBackend;
        support::TestGpio rightBackend;
        support::TestGpio triggerBackend;
        support::TestGpio echoBackend;
        const XWalkHal::XWalkGpioCallbacks gpioBackendCallbacks = support::gpioCallbacks();
        xwalk::hal::XWalkGpio leftDirection(&leftBackend, gpioBackendCallbacks, "D4");
        xwalk::hal::XWalkGpio rightDirection(&rightBackend, gpioBackendCallbacks, "D5");
        xwalk::hal::XWalkGpio trigger(&triggerBackend, gpioBackendCallbacks, "D2");
        xwalk::hal::XWalkGpio echo(&echoBackend, gpioBackendCallbacks, "D3");
        xwalk::hal::XWalkMotor leftMotor(leftPwm, leftDirection);
        xwalk::hal::XWalkMotor rightMotor(rightPwm, rightDirection);
        xwalk::hal::XWalkMotorsConfiguration motorsConfiguration;
        motorsConfiguration.clockContext = &backend;
        motorsConfiguration.clockMilliseconds = &support::clockMilliseconds;
        motorsConfiguration.watchdogWorkerEnabled = false;
        xwalk::hal::XWalkMotors motors(leftMotor, rightMotor, motorsConfiguration);
        backend.motors = &motors;
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
        xwalk::agent::xAgentContext carCtx{};
        carCtx.config = &config;
        carCtx.motors = &motors;
        carCtx.dirServo = &directionServo;
        carCtx.panServo = &panServo;
        carCtx.tiltServo = &tiltServo;
        carCtx.grayscale = &grayscale;
        carCtx.ultrasonic = &ultrasonic;
        xwalk::agent::XWalkPicarx picarx(carCtx);
        static_cast<void>(picarx.initialize());

        const XWalkHal::XWalkMusicCallbacks musicCallbacks{&support::enableOutput,
                                                           &support::playSound,
                                                           &support::playSoundBackground,
                                                           &support::playMusic,
                                                           &support::setMusicVolume,
                                                           &support::controlMusic,
                                                           &support::controlMusic,
                                                           &support::controlMusic,
                                                           &support::soundLength,
                                                           &support::playTone};
        xwalk::hal::XWalkMusic music(&backend, musicCallbacks);
        xwalk::agent::XWalkSelfDrive selfDrive(picarx,
                                               music,
                                               &backend,
                                               &support::delayMilliseconds,
                                               nullptr,
                                               XWALK_TEST_SOUND_DIRECTORY,
                                               XWALK_TEST_MUSIC_DIRECTORY);
        assert(backend.outputEnabled);

        agent::size delayCount = backend.delays.size();
        assert(selfDrive.doAction("wave hands"));
        assert(backend.delays.size() == (delayCount + 4U));
        assert(backend.delays[delayCount] == 100U);
        assert(backend.delays[delayCount + 3U] == 100U);

        delayCount = backend.delays.size();
        assert(selfDrive.doAction("forward"));
        assert(backend.delays.size() == (delayCount + 10U));
        for (agent::size index = delayCount; index < backend.delays.size(); ++index)
        {
            assert(backend.delays[index] == 100U);
        }
        assert(motors.isArmed());

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
        assert(motors.isArmed());
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
        assert(picarx.directionAngleDegrees() == 0.0);

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
        assert(selfDrive.doAction("play background music"));
        assert(backend.musicFile == (agent::filesystempath(XWALK_TEST_MUSIC_DIRECTORY) / "slow-trail-Ahjay_Stelino.mp3")
                                        .lexically_normal()
                                        .string());
        assert(backend.musicLoops == 1);
        assert(backend.musicStartSeconds == 0.0);
        assert(backend.musicVolume == 0.2);
        assert(selfDrive.doAction("stop background music"));
        assert(backend.musicControlCount == 1U);
        assert(selfDrive.doAction("stop background music"));
        assert(backend.musicControlCount == 1U);
        assert(!selfDrive.doAction("unknown"));
        assert(!selfDrive.addAction("unknown"));

        xwalk::agent::XWalkSelfDrive missingSoundSelfDrive(
            picarx, music, &backend, &support::delayMilliseconds, nullptr, "/xwalk-test-missing-sounds");
        assert(!missingSoundSelfDrive.doAction("honking"));

        selfDrive.setCancellation(&backend, &support::continueOperation);
        backend.continueQueryLimit = backend.continueQueries + 2U;
        delayCount = backend.delays.size();
        picarx.clearEmergencyStop();
        assert(!selfDrive.doAction("forward"));
        agent::uint32 cancelledDelayMs{};
        for (agent::size index = delayCount; index < backend.delays.size(); ++index)
        {
            assert(backend.delays[index] <= 20U);
            cancelledDelayMs += backend.delays[index];
        }
        assert(cancelledDelayMs <= 40U);
        assert(picarx.emergencyStopRequested());
        backend.continueQueryLimit = 1'000'000U;
        picarx.clearEmergencyStop();

        selfDrive.setCancellation(&backend, nullptr);
        backend.expireWatchdogWhileMoving = true;
        assert(!selfDrive.doAction("backward"));
        assert(picarx.emergencyStopRequested());
        assert(!motors.isArmed());
        assert(!motors.heartbeatSafely());
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
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
        assert(selfDrive.addAction("honking"));
        assert(selfDrive.addAction("backward"));
        assert(selfDrive.waitActionsDone());
        assert(selfDrive.status() == xwalk::agent::XWalkSelfDriveStatus::Standby);
        selfDrive.stop();
        assert(!selfDrive.running());

        backend.failDelayWhileMoving = true;
        picarx.clearEmergencyStop();
        selfDrive.start();
        assert(selfDrive.addAction("forward"));
        const agent::boolean workerCompleted = selfDrive.waitActionsDone();
        backend.failDelayWhileMoving = false;
        selfDrive.start();
        assert(selfDrive.running());
        selfDrive.stop();
        assert(!workerCompleted);
        assert(picarx.emergencyStopRequested());
        assert(motors.left().speed() == 0.0);
        assert(motors.right().speed() == 0.0);
    }

} /* namespace */

/** @brief Runs all deterministic SelfDrive host-test scenarios. */
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
