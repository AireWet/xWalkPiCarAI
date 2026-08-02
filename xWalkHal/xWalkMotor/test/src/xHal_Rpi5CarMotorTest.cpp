/******************************************************************************
 * @file        xHal_Rpi5CarMotorTest.cpp
 * @brief       Verifies single and paired motor behavior using in-memory callbacks.
 *
 * @details
 * Exercises both driver modes, signed speed, reversal, braking, movement commands, and validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkMotor Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarMotor.h"

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarMotors.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Provides the minimal callback state required by host PWM objects. */
struct TestI2c
{
    XWalkHal::uint32 writeCount{}; /**< Number of simulated register writes. */
    XWalkHal::uint32vector failingWrites; /**< One-based write indices that throw a simulated failure. */
};

/** @brief Records the most recent simulated GPIO output. */
struct TestGpio
{
    XWalkHal::boolean value{}; /**< Most recent physical output level. */
    XWalkHal::size writeCount{}; /**< Number of physical output writes. */
};

/******************************************************************************
 * Private callback definitions
 ******************************************************************************/

/**
 * @brief Simulates an I2C probe with no responding device.
 *
 * @param[in] context
 * Non-owning test context; unused by this callback.
 *
 * @param[in] address
 * Seven-bit address; unused by this callback.
 *
 * @return
 * Always `false`.
 */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return false;
}

/**
 * @brief Counts one simulated I2C register write.
 *
 * @param[in,out] context
 * Non-null pointer to `TestI2c`.
 *
 * @param[in] address
 * Seven-bit destination address; unused after interface validation.
 *
 * @param[in] reg
 * Register address; unused by this callback.
 *
 * @param[in] data
 * Register payload; unused by this callback.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::uint8 reg,
    const XWalkHal::bytevector& data)
{
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    TestI2c& bus = *static_cast<TestI2c*>(context);
    ++bus.writeCount;
    for (const XWalkHal::uint32 failingWrite : bus.failingWrites)
    {
        if (bus.writeCount == failingWrite)
        {
            XHAL_THROW_RUNTIME_ERROR("simulated motor PWM write failure");
        }
    }
}

/** @brief Attempts one simulated PWM write without throwing. */
XWalkHal::boolean tryWriteRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data) noexcept
{
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    TestI2c& bus = *static_cast<TestI2c*>(context);
    ++bus.writeCount;
    for (const XWalkHal::uint32 failingWrite : bus.failingWrites)
    {
        if (bus.writeCount == failingWrite)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Returns zero-filled bytes for the unused test read operation.
 *
 * @param[in] context
 * Non-owning test context; unused by this callback.
 *
 * @param[in] address
 * Seven-bit source address; unused by this callback.
 *
 * @param[in] length
 * Number of zero-filled bytes to return.
 *
 * @return
 * Byte vector containing `length` zero values.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return XWalkHal::bytevector(length, 0U);
}

/**
 * @brief Accepts one simulated GPIO configuration.
 *
 * @param[in] context
 * Non-owning GPIO test context; unused by this callback.
 *
 * @param[in] pin
 * GPIO line offset; unused by this callback.
 *
 * @param[in] mode
 * GPIO mode; unused by this callback.
 *
 * @param[in] pull
 * GPIO pull setting; unused by this callback.
 *
 * @param[in] initialValue
 * Initial output value; unused by this callback.
 */
void configureGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull, XWalkHal::boolean initialValue)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<void>(initialValue);
}

/**
 * @brief Returns the stored simulated GPIO level.
 *
 * @param[in,out] context
 * Non-null pointer to `TestGpio`.
 *
 * @param[in] pin
 * GPIO line offset; unused by this callback.
 *
 * @return
 * Most recently stored physical level.
 */
XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(pin);
    return static_cast<TestGpio*>(context)->value;
}

/**
 * @brief Records one simulated GPIO output.
 *
 * @param[in,out] context
 * Non-null pointer to `TestGpio`.
 *
 * @param[in] pin
 * GPIO line offset; unused by this callback.
 *
 * @param[in] value
 * Physical output value to store.
 */
void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
{
    static_cast<void>(pin);
    TestGpio& gpio = *static_cast<TestGpio*>(context);
    gpio.value = value;
    ++gpio.writeCount;
}

/**
 * @brief Accepts an unused simulated GPIO interrupt registration.
 *
 * @param[in] context
 * Non-owning test context; unused by this callback.
 *
 * @param[in] pin
 * GPIO line offset; unused by this callback.
 *
 * @param[in] edge
 * Edge selection; unused by this callback.
 *
 * @param[in] debounceMs
 * Debounce interval in milliseconds; unused by this callback.
 *
 * @param[in] handlerContext
 * Non-owning handler context; unused by this callback.
 *
 * @param[in] handler
 * Application handler; unused by this callback.
 */
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

/**
 * @brief Accepts an unused simulated GPIO interrupt cancellation.
 *
 * @param[in] context
 * Non-owning test context; unused by this callback.
 *
 * @param[in] pin
 * GPIO line offset; unused by this callback.
 */
void cancelGpioInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/**
 * @brief Returns the complete GPIO callback set used by host motor tests.
 *
 * @return
 * Callback set bound to translation-unit test functions.
 */
XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &interruptGpio, &cancelGpioInterrupt};
}

/******************************************************************************
 * Test function definitions
 ******************************************************************************/

/** @brief Verifies PWM-and-direction speed, stop, and reversal behavior. */
void testPwmAndDirectionMode()
{
    TestI2c bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm pwm(i2c, 13U, 0x14U, timerState);
    TestGpio gpioBackend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio direction(&gpioBackend, callbacks, "D4");
    xwalk::hal::XWalkMotor motor(pwm, direction);

    assert(motor.mode() == XWalkHal::XWalkMotorMode::PwmAndDirection);
    assert(motor.frequency() == 100.0);
    assert(pwm.pulseWidthPercent() == 0.0);

    motor.setSpeed(50.0);
    assert(motor.speed() == 50.0);
    assert(pwm.pulseWidthPercent() == 50.0);
    assert(gpioBackend.value);

    motor.setSpeed(-25.0);
    assert(motor.speed() == -25.0);
    assert(pwm.pulseWidthPercent() == 25.0);
    assert(!gpioBackend.value);

    motor.setReversed(true);
    motor.setSpeed(10.0);
    assert(!gpioBackend.value);
    motor.stop();
    assert(motor.speed() == 0.0);
    assert(pwm.pulseWidthPercent() == 0.0);
}

/** @brief Verifies dual-PWM speed selection, reversal, stopping, and braking. */
void testDualPwmMode()
{
    TestI2c bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm forwardPwm(i2c, 12U, 0x14U, timerState);
    xwalk::hal::XWalkPwm reversePwm(i2c, 13U, 0x14U, timerState);
    xwalk::hal::XWalkMotor motor(forwardPwm, reversePwm);

    motor.setSpeed(40.0);
    assert(forwardPwm.pulseWidthPercent() == 40.0);
    assert(reversePwm.pulseWidthPercent() == 0.0);
    motor.setSpeed(-30.0);
    assert(forwardPwm.pulseWidthPercent() == 0.0);
    assert(reversePwm.pulseWidthPercent() == 30.0);

    motor.setReversed(true);
    motor.setSpeed(20.0);
    assert(forwardPwm.pulseWidthPercent() == 0.0);
    assert(reversePwm.pulseWidthPercent() == 20.0);
    motor.brake();
    assert(forwardPwm.pulseWidthPercent() == 100.0);
    assert(reversePwm.pulseWidthPercent() == 100.0);
    assert(motor.speed() == 0.0);
}

/** @brief Verifies paired movement commands, roles, and runtime configuration output. */
void testPairedMotors()
{
    TestI2c bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm firstPwm(i2c, 12U, 0x14U, timerState);
    xwalk::hal::XWalkPwm secondPwm(i2c, 13U, 0x14U, timerState);
    TestGpio firstGpioBackend;
    TestGpio secondGpioBackend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio firstDirection(&firstGpioBackend, callbacks, "D4");
    xwalk::hal::XWalkGpio secondDirection(&secondGpioBackend, callbacks, "D5");
    xwalk::hal::XWalkMotor firstMotor(firstPwm, firstDirection);
    xwalk::hal::XWalkMotor secondMotor(secondPwm, secondDirection);
    xwalk::hal::XWalkMotors motors(firstMotor, secondMotor);

    motors.turnLeft(35.0);
    const XWalkHal::float64 leftTurnLeftSpeed = motors.left().speed();
    const XWalkHal::float64 rightTurnLeftSpeed = motors.right().speed();
    assert(leftTurnLeftSpeed == -35.0);
    assert(rightTurnLeftSpeed == 35.0);
    motors.turnRight(20.0);
    const XWalkHal::float64 leftTurnRightSpeed = motors.left().speed();
    const XWalkHal::float64 rightTurnRightSpeed = motors.right().speed();
    assert(leftTurnRightSpeed == 20.0);
    assert(rightTurnRightSpeed == -20.0);
    motors.backward(15.0);
    assert(firstMotor.speed() == -15.0);
    assert(secondMotor.speed() == -15.0);

    motors.setLeftMotorId(2U);
    motors.setRightMotorId(1U);
    motors.forward(10.0);
    assert(secondMotor.speed() == 10.0);
    assert(firstMotor.speed() == 10.0);
    const XWalkHal::boolean leftReversed = motors.toggleLeftReversed();
    assert(leftReversed);
    assert(secondMotor.reversed());
    const XWalkHal::XWalkMotorsConfiguration configuration = motors.configuration();
    assert(configuration.leftMotorId == 2U);
    assert(configuration.rightMotorId == 1U);
    assert(configuration.leftReversed);
}

/** @brief Verifies that fail-safe shutdown attempts every channel after independent write failures. */
void testFailSafeStop()
{
    TestI2c bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm firstForward(i2c, 12U, 0x14U, timerState);
    xwalk::hal::XWalkPwm firstReverse(i2c, 13U, 0x14U, timerState);
    xwalk::hal::XWalkPwm secondForward(i2c, 14U, 0x14U, timerState);
    xwalk::hal::XWalkPwm secondReverse(i2c, 15U, 0x14U, timerState);
    xwalk::hal::XWalkMotor firstMotor(firstForward, firstReverse);
    xwalk::hal::XWalkMotor secondMotor(secondForward, secondReverse);
    xwalk::hal::XWalkMotors motors(firstMotor, secondMotor);
    motors.setSpeed(30.0, 40.0);

    const XWalkHal::uint32 firstStopWrite = bus.writeCount + 1U;
    bus.failingWrites = {firstStopWrite, firstStopWrite + 2U};
    assert(!motors.stopSafely());
    assert(bus.writeCount == (firstStopWrite + 3U));
    assert(firstMotor.speed() == 30.0);
    assert(secondMotor.speed() == 40.0);

    bus.failingWrites.clear();
    assert(motors.stopSafely());
    assert(firstMotor.speed() == 0.0);
    assert(secondMotor.speed() == 0.0);
}

/** @brief Verifies speed, frequency, identifier, and mode validation. */
void testValidation()
{
    TestI2c bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, nullptr,
        &tryWriteRegister);
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm pwm(i2c, 13U, 0x14U, timerState);
    TestGpio gpioBackend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    xwalk::hal::XWalkGpio direction(&gpioBackend, callbacks, "D4");
    xwalk::hal::XWalkMotor motor(pwm, direction);

    xwalk::hal::test::expectFailure([&]()
    {
        motor.setSpeed(101.0);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        motor.setSpeed(XHAL_POSITIVE_INFINITY(XWalkHal::float64));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        motor.brake();
    });

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::XWalkMotor invalidFrequencyMotor(pwm, direction, false, 0.0);
    });

    xwalk::hal::XWalkPwm secondPwm(i2c, 12U, 0x14U, timerState);
    TestGpio secondGpioBackend;
    xwalk::hal::XWalkGpio secondDirection(&secondGpioBackend, callbacks, "D5");
    xwalk::hal::XWalkMotor secondMotor(secondPwm, secondDirection);
    xwalk::hal::XWalkMotors motors(motor, secondMotor);
    xwalk::hal::test::expectFailure([&]()
    {
        motors.setLeftMotorId(0U);
    });

    motors.setSpeed(12.0, 13.0);
    xwalk::hal::test::expectFailure([&]()
    {
        motors.setSpeed(20.0, 101.0);
    });
    assert(motor.speed() == 12.0);
    assert(secondMotor.speed() == 13.0);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all xWalk Motor host-test scenarios.
 *
 * @return
 * Zero when every assertion passes. A failed assertion terminates the process.
 */
XWalkHal::int32 main()
{
    testPwmAndDirectionMode();
    testDualPwmMode();
    testPairedMotors();
    testFailSafeStop();
    testValidation();
    return 0;
}
