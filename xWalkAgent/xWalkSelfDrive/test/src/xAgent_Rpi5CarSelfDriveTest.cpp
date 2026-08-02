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
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarTestFunctions.h"

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

/** @brief Provides deterministic I2C samples and accepts register writes. */
struct TestBus
{
    XWalkHal::bytevector sample{0x03U, 0xE8U};
};

/** @brief Stores one simulated GPIO level. */
struct TestGpio
{
    XWalkHal::boolean value{};
};

/** @brief Records injected delay and audio operations. */
struct TestBackend
{
    XWalkHal::uint32vector delays{};
    XWalkHal::stringvector backgroundFiles{};
    XWalkHal::float64vector backgroundVolumes{};
    XWalkHal::boolean outputEnabled{};
    XWalkHal::uint32 continueQueries{};
    XWalkHal::uint32 continueQueryLimit{1'000'000U};
    xwalk::hal::XWalkMotors* motors{nullptr};
    XWalkHal::boolean failDelayWhileMoving{};
};

/******************************************************************************
 * Private callback definitions
 ******************************************************************************/

/** @brief Accepts every simulated I2C address. */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/** @brief Accepts one simulated I2C register write. */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
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

/** @brief Returns the deterministic two-byte ADC sample. */
XWalkHal::bytevector readBus(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    return static_cast<TestBus*>(context)->sample;
}

/** @brief Stores the configured simulated GPIO level. */
void configureGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull, XWalkHal::boolean initialValue)
{
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<TestGpio*>(context)->value = initialValue;
}

/** @brief Returns one simulated GPIO level. */
XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(pin);
    return static_cast<TestGpio*>(context)->value;
}

/** @brief Stores one simulated GPIO output level. */
void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
{
    static_cast<void>(pin);
    static_cast<TestGpio*>(context)->value = value;
}

/** @brief Accepts one simulated GPIO interrupt registration. */
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

/** @brief Accepts one simulated GPIO interrupt cancellation. */
void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
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
void enableOutput(XWalkHal::contextpointer context)
{
    static_cast<TestBackend*>(context)->outputEnabled = true;
}

/** @brief Accepts one synchronous sound request. */
void playSound(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::optionalfloat64 volume)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    static_cast<void>(volume);
}

/** @brief Records one asynchronous preset sound request. */
void playSoundBackground(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::optionalfloat64 volume)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    backend.backgroundFiles.emplace_back(filename);
    backend.backgroundVolumes.push_back(volume.has_value() ? *volume : -1.0);
}

/** @brief Accepts one unused streamed-music request. */
void playMusic(XWalkHal::contextpointer context, XWalkHal::stringview filename,
    XWalkHal::int32 loops, XWalkHal::float64 startSeconds)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    static_cast<void>(loops);
    static_cast<void>(startSeconds);
}

/** @brief Accepts one unused music-volume request. */
void setMusicVolume(XWalkHal::contextpointer context, XWalkHal::float64 volume)
{
    static_cast<void>(context);
    static_cast<void>(volume);
}

/** @brief Accepts one unused music-control request. */
void controlMusic(XWalkHal::contextpointer context)
{
    static_cast<void>(context);
}

/** @brief Returns a deterministic zero-second sound length. */
XWalkHal::float64 soundLength(XWalkHal::contextpointer context, XWalkHal::stringview filename)
{
    static_cast<void>(context);
    static_cast<void>(filename);
    return 0.0;
}

/** @brief Accepts one unused generated-tone request. */
void playTone(XWalkHal::contextpointer context, const XWalkHal::bytevector& data,
    XWalkHal::uint32 sampleRateHz, XWalkHal::uint8 channelCount)
{
    static_cast<void>(context);
    static_cast<void>(data);
    static_cast<void>(sampleRateHz);
    static_cast<void>(channelCount);
}

/** @brief Records one requested preset-action delay without sleeping. */
XWalkHal::boolean delayMilliseconds(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMs) noexcept
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    backend.delays.push_back(durationMs);
    if (backend.failDelayWhileMoving &&
        ((backend.motors->left().speed() != 0.0) || (backend.motors->right().speed() != 0.0)))
    {
        return false;
    }
    return true;
}

/** @brief Supplies a deterministic cancellation sequence for one preset action. */
XWalkHal::boolean continueOperation(XWalkHal::contextpointer context)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    const XWalkHal::boolean shouldContinue = backend.continueQueries < backend.continueQueryLimit;
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
void testSelfDriveBehavior(XWalkHal::stringview configPath)
{
    TestBus bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &readBus, nullptr,
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

    TestBackend backend;
    backend.motors = &motors;
    const XWalkHal::XWalkMusicCallbacks musicCallbacks{
        &enableOutput,
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
    xwalk::agent::XWalkSelfDrive selfDrive(picarx, music, &backend,
        &delayMilliseconds, nullptr, XWALK_TEST_SOUND_DIRECTORY);
    assert(backend.outputEnabled);

    XWalkHal::size delayCount = backend.delays.size();
    assert(selfDrive.doAction("wave hands"));
    assert(backend.delays.size() == (delayCount + 4U));
    assert(backend.delays[delayCount] == 100U);
    assert(backend.delays[delayCount + 3U] == 100U);

    delayCount = backend.delays.size();
    assert(selfDrive.doAction("forward"));
    assert(backend.delays.size() == (delayCount + 1U));
    assert(backend.delays[delayCount] == 1'000U);

    const XWalkHal::stringvector gestures{
        "shake head", "nod", "resist", "act cute", "rub hands", "think",
        "twist body", "celebrate", "depressed", "backward"};
    for (const XWalkHal::string& gesture : gestures)
    {
        assert(selfDrive.doAction(gesture));
    }
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);
    assert(picarx.directionAngleDegrees() == 0.0);
    assert(!backend.delays.empty());

    assert(selfDrive.doAction("honking"));
    assert(selfDrive.doAction("start engine"));
    assert(backend.backgroundFiles[0U] ==
        (XWalkHal::filesystempath(XWALK_TEST_SOUND_DIRECTORY) /
            "car-double-horn.wav").lexically_normal().string());
    assert(backend.backgroundVolumes[0U] == 1.0);
    assert(backend.backgroundFiles[1U] ==
        (XWalkHal::filesystempath(XWALK_TEST_SOUND_DIRECTORY) /
            "car-start-engine.wav").lexically_normal().string());
    assert(backend.backgroundVolumes[1U] == 0.5);
    assert(!selfDrive.doAction("unknown"));
    assert(!selfDrive.addAction("unknown"));

    xwalk::agent::XWalkSelfDrive missingSoundSelfDrive(picarx, music, &backend,
        &delayMilliseconds, nullptr, "/xwalk-test-missing-sounds");
    assert(!missingSoundSelfDrive.doAction("honking"));
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    selfDrive.setCancellation(&backend, &continueOperation);
    backend.continueQueryLimit = backend.continueQueries + 2U;
    delayCount = backend.delays.size();
    picarx.clearEmergencyStop();
    assert(selfDrive.doAction("forward"));
    XWalkHal::uint32 cancelledDelayMs{};
    for (XWalkHal::size index = delayCount; index < backend.delays.size(); ++index)
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

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::agent::XWalkSelfDrive invalid(picarx, music, &backend,
            nullptr, nullptr, XWALK_TEST_SOUND_DIRECTORY);
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
    const XWalkHal::boolean workerCompleted = selfDrive.waitActionsDone();
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
    testSelfDriveBehavior(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath.parent_path()));
    return 0;
}
