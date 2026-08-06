/******************************************************************************
 * @file        xAgent_Rpi5CarObstacleAvoidanceTest.cpp
 * @brief       Verifies obstacle avoidance through an in-memory HAL graph.
 *
 * @details
 * Exercises every distance boundary, failed-sensor handling, cancellation,
 * bounded delays, and motor cleanup without opening physical devices.
 *
 * @project     xWalk Firmware
 * @module      xWalkObstacleAvoidance Host Test
 *
 * @author      Joxy John
 * @date        2026-08-04
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

#include "xAgent_Rpi5CarObstacleAvoidance.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMotors.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarUltrasonic.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains deterministic host-test state and callbacks. */
namespace
{

/** @brief Provides one constant ADC sample. */
struct TestBus
{
    agent::bytevector sample{0x03U, 0xE8U};
};

/** @brief Stores one simulated GPIO level. */
struct TestGpio
{
    agent::boolean value{};
};

/** @brief Records injected delays and cancellation queries. */
struct TestSchedule
{
    agent::uint32vector delays{};
    agent::uint32 queryCount{};
    agent::uint32 queryLimit{10'000U};
};

/** @brief Accepts every simulated I2C probe. */
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

/** @brief Accepts one non-throwing simulated I2C register write. */
agent::boolean tryWriteRegister(agent::contextpointer context,
    agent::uint8 address, agent::uint8 reg,
    const agent::bytevector& data) noexcept
{
    writeRegister(context, address, reg, data);
    return true;
}

/** @brief Returns one constant simulated ADC sample. */
agent::bytevector readBus(agent::contextpointer context,
    agent::uint8 address, agent::size length)
{
    static_cast<void>(address);
    static_cast<void>(length);
    return static_cast<TestBus*>(context)->sample;
}

/** @brief Configures one simulated GPIO level. */
void configureGpio(agent::contextpointer context, agent::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
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

/** @brief Writes one simulated GPIO level. */
void writeGpio(agent::contextpointer context, agent::uint8 pin,
    agent::boolean value)
{
    static_cast<void>(pin);
    static_cast<TestGpio*>(context)->value = value;
}

/** @brief Accepts one unused GPIO interrupt registration. */
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

/** @brief Accepts one unused GPIO interrupt cancellation. */
void cancelInterrupt(agent::contextpointer context, agent::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/** @brief Returns the complete simulated GPIO operation table. */
XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelInterrupt};
}

/** @brief Records one bounded obstacle-avoidance delay. */
void delay(agent::contextpointer context, agent::uint32 durationMs)
{
    static_cast<TestSchedule*>(context)->delays.push_back(durationMs);
}

/** @brief Allows operation until the configured query limit is reached. */
agent::boolean continueOperation(agent::contextpointer context)
{
    TestSchedule& schedule = *static_cast<TestSchedule*>(context);
    const agent::boolean result = schedule.queryCount < schedule.queryLimit;
    ++schedule.queryCount;
    return result;
}

/** @brief Exercises distance bands, invalid samples, cancellation, and cleanup. */
void testObstacleAvoidance(agent::stringview configurationPath)
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
    xwalk::hal::XWalkConfigStore configuration(configurationPath);
    configuration.set("picarx_max_motor_output_percent", "100");
    configuration.set("picarx_calibration_verified", "true");
    xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo, tiltServo,
        grayscale, ultrasonic, configuration);
    TestSchedule schedule;
    xwalk::agent::XWalkObstacleAvoidance avoidance(
        picarx, &schedule, &delay, &continueOperation);

    assert(avoidance.step(40.0) ==
        xwalk::agent::XWalkObstacleAvoidanceResult::Forward);
    assert(picarx.directionAngleDegrees() == 0.0);
    assert(motors.left().speed() != 0.0);
    assert(motors.right().speed() != 0.0);

    assert(avoidance.step(20.0) ==
        xwalk::agent::XWalkObstacleAvoidanceResult::TurnRight);
    assert(picarx.directionAngleDegrees() == 30.0);
    assert(schedule.delays.size() == 5U);

    assert(avoidance.step(19.99) ==
        xwalk::agent::XWalkObstacleAvoidanceResult::ReverseLeft);
    assert(picarx.directionAngleDegrees() == -30.0);
    assert(schedule.delays.size() == 30U);

    assert(avoidance.step(-1.0) ==
        xwalk::agent::XWalkObstacleAvoidanceResult::SensorInvalid);
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);

    schedule.queryLimit = schedule.queryCount;
    assert(avoidance.step(50.0) ==
        xwalk::agent::XWalkObstacleAvoidanceResult::Cancelled);
    avoidance.stop();
    assert(motors.left().speed() == 0.0);
    assert(motors.right().speed() == 0.0);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the device-free obstacle-avoidance host verification.
 * @param[in] argumentCount Must be two.
 * @param[in] argumentValues Test executable and writable configuration path.
 * @return Zero after successful assertions or one for invalid arguments.
 */
int main(int argumentCount, char* argumentValues[])
{
    if (argumentCount != 2)
    {
        return 1;
    }
    const agent::filesystempath configurationPath(argumentValues[1U]);
    agent::filesystempath replacementPath = configurationPath;
    replacementPath += ".tmp";
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configurationPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    testObstacleAvoidance(configurationPath.string());
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configurationPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(replacementPath));
    static_cast<void>(xwalk::hal::removeFilesystemEntry(configurationPath.parent_path()));
    return 0;
}
