/******************************************************************************
 * @file        xAgent_Rpi5CarLineTrackingTest.cpp
 * @brief       Verifies line tracking with in-memory hardware callbacks.
 *
 * @details
 * Covers classification priority, steering and drive decisions, directional
 * recovery, recovery timeout, stopping, and configuration validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkLineTracking Host Test
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

#include "xAgent_Rpi5CarLineTracking.h"

#include "xHal_Rpi5CarAdc.h"
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

/** @brief Provides an ordered sequence of simulated ADC sample counts. */
struct TestBus
{
    agent::uint32vector samples{};
    agent::size nextSample{};
};

/** @brief Stores one simulated GPIO level. */
struct TestGpio
{
    agent::boolean value{};
};

/** @brief Records requested line-recovery delays. */
struct TestTiming
{
    agent::uint32vector delays{};
};

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
void writeRegister(agent::contextpointer context, agent::uint8 address,
    agent::uint8 reg, const agent::bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

/** @brief Accepts one non-throwing simulated fail-safe write. */
agent::boolean tryWriteRegister(agent::contextpointer context, agent::uint8 address,
    agent::uint8 reg, const agent::bytevector& data) noexcept
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    return true;
}

/** @brief Returns the next simulated big-endian ADC sample. */
agent::bytevector readBus(agent::contextpointer context, agent::uint8 address,
    agent::size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    TestBus& bus = *static_cast<TestBus*>(context);
    const agent::uint32 sample = bus.samples[bus.nextSample];
    ++bus.nextSample;
    const agent::uint8 high = static_cast<agent::uint8>((sample >> 8U) & 0xFFU);
    const agent::uint8 low = static_cast<agent::uint8>(sample & 0xFFU);
    return {high, low};
}

/** @brief Stores the configured simulated GPIO level. */
void configureGpio(agent::contextpointer context, agent::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull, agent::boolean initialValue)
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
void interruptGpio(agent::contextpointer context, agent::uint8 pin,
    XWalkHal::XWalkGpioEdge edge, agent::uint32 debounceMs,
    agent::contextpointer handlerContext, XWalkHal::gpiointerrupthandler handler)
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

/** @brief Appends one left, middle, and right sample frame. */
void appendFrame(TestBus& bus, agent::uint32 left, agent::uint32 middle,
    agent::uint32 right)
{
    bus.samples.push_back(left);
    bus.samples.push_back(middle);
    bus.samples.push_back(right);
}

/** @brief Records one recovery-completion delay without sleeping. */
void delayMilliseconds(agent::contextpointer context, agent::uint32 durationMs)
{
    static_cast<TestTiming*>(context)->delays.push_back(durationMs);
}

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/**
 * @brief Exercises line tracking through a complete in-memory HAL composition.
 *
 * @param[in] configPath
 * Test-owned configuration path below the module build directory.
 */
void testLineTrackingBehavior(agent::stringview configPath)
{
    TestBus bus;
    appendFrame(bus, 1'500U, 500U, 1'500U);
    appendFrame(bus, 500U, 1'500U, 1'500U);
    appendFrame(bus, 1'500U, 1'500U, 500U);
    appendFrame(bus, 1'500U, 1'500U, 1'500U);
    appendFrame(bus, 1'500U, 500U, 1'500U);
    appendFrame(bus, 1'500U, 1'500U, 1'500U);
    appendFrame(bus, 1'500U, 1'500U, 1'500U);
    appendFrame(bus, 1'500U, 1'500U, 1'500U);

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
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio leftDirection(&leftBackend, callbacks, "D4");
    xwalk::hal::XWalkGpio rightDirection(&rightBackend, callbacks, "D5");
    xwalk::hal::XWalkGpio trigger(&triggerBackend, callbacks, "D2");
    xwalk::hal::XWalkGpio echo(&echoBackend, callbacks, "D3");
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
    config.set("line_reference", "[1000,1000,1000]");
    xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo, tiltServo,
        grayscale, ultrasonic, config);
    TestTiming timing;
    xwalk::agent::XWalkLineTracking tracking(picarx, &timing, &delayMilliseconds);

    assert(xwalk::agent::XWalkLineTracking::classify({0U, 0U, 0U}) ==
        xwalk::agent::XWalkLineTrackingState::Stop);
    assert(xwalk::agent::XWalkLineTracking::classify({1U, 1U, 1U}) ==
        xwalk::agent::XWalkLineTrackingState::Forward);
    assert(xwalk::agent::XWalkLineTracking::classify({1U, 0U, 0U}) ==
        xwalk::agent::XWalkLineTrackingState::Right);
    assert(xwalk::agent::XWalkLineTracking::classify({0U, 0U, 1U}) ==
        xwalk::agent::XWalkLineTrackingState::Left);

    xwalk::agent::XWalkLineTrackingResult result = tracking.step();
    assert(result.state == xwalk::agent::XWalkLineTrackingState::Forward);
    assert(!result.recoveryAttempted);
    assert(picarx.directionAngleDegrees() == 0.0);
    assert(motors.left().speed() == 55.0);
    assert(motors.right().speed() == 55.0);

    result = tracking.step();
    assert(result.state == xwalk::agent::XWalkLineTrackingState::Right);
    assert(picarx.directionAngleDegrees() == -20.0);

    result = tracking.step();
    assert(result.state == xwalk::agent::XWalkLineTrackingState::Left);
    assert(picarx.directionAngleDegrees() == 20.0);

    result = tracking.step();
    assert(result.recoveryAttempted);
    assert(!result.recoveryTimedOut);
    assert(result.state == xwalk::agent::XWalkLineTrackingState::Forward);
    assert(picarx.directionAngleDegrees() == -30.0);
    assert(motors.left().speed() < 0.0);
    assert(motors.right().speed() < 0.0);
    assert(timing.delays.back() == 1U);

    tracking.stop();
    assert(tracking.currentState() == xwalk::agent::XWalkLineTrackingState::Stop);
    assert(tracking.lastState() == xwalk::agent::XWalkLineTrackingState::Stop);
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    tracking.finish();
    assert(timing.delays.back() == 100U);
    assert(tracking.currentState() == xwalk::agent::XWalkLineTrackingState::Stop);
    assert(tracking.lastState() == xwalk::agent::XWalkLineTrackingState::Stop);

    xwalk::agent::XWalkLineTrackingConfiguration timeoutConfiguration;
    timeoutConfiguration.maximumRecoverySamples = 2U;
    xwalk::agent::XWalkLineTracking timeoutTracking(
        picarx, &timing, &delayMilliseconds, timeoutConfiguration);
    result = timeoutTracking.step();
    assert(result.recoveryAttempted);
    assert(result.recoveryTimedOut);
    assert(result.state == xwalk::agent::XWalkLineTrackingState::Stop);
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::agent::XWalkLineTrackingConfiguration invalidConfiguration;
        invalidConfiguration.powerPercent = 101.0;
        xwalk::agent::XWalkLineTracking invalid(
            picarx, &timing, &delayMilliseconds, invalidConfiguration);
        static_cast<void>(invalid);
    });
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::agent::XWalkLineTracking invalid(picarx, &timing, nullptr);
        static_cast<void>(invalid);
    });
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::agent::XWalkLineTrackingConfiguration invalidConfiguration;
        invalidConfiguration.maximumRecoverySamples = 0U;
        xwalk::agent::XWalkLineTracking invalid(
            picarx, &timing, &delayMilliseconds, invalidConfiguration);
        static_cast<void>(invalid);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all line-tracking host-test scenarios.
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
    testLineTrackingBehavior(configPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configPath.parent_path()));
    return 0;
}
