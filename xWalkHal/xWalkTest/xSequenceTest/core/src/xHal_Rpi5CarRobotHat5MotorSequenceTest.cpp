/******************************************************************************
 * @file        xHal_Rpi5CarRobotHat5MotorSequenceTest.cpp
 * @brief       Verifies the Robot HAT v5 motor sequence in memory.
 *
 * @details
 * Checks all four dual-PWM motors, signed-speed phases, bounded waits, final
 * stop behavior, and cycle-count validation without physical motor movement.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Host Test
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

#include "xHal_Rpi5CarRobotHat5MotorSequence.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

struct TestI2c
{
    XWalkHal::uint32 writeCount{};
};

struct WaitState
{
    XWalkHal::fixedarray<XWalkHal::XWalkMotor*, 4U> motors{};
    XWalkHal::uint32vector durations;
    XWalkHal::float64vector observedSpeeds;
    XWalkHal::boolean failNextWait{};
};

XWalkHal::boolean probe(
    XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    TestI2c& state = *static_cast<TestI2c*>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    ++state.writeCount;
}

XWalkHal::boolean tryWriteRegister(XWalkHal::contextpointer context,
    XWalkHal::uint8 address, XWalkHal::uint8 reg,
    const XWalkHal::bytevector& data) noexcept
{
    TestI2c& state = *static_cast<TestI2c*>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    ++state.writeCount;
    return true;
}

XWalkHal::bytevector read(XWalkHal::contextpointer context,
    XWalkHal::uint8 address, XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return XWalkHal::bytevector(length, 0U);
}

void wait(XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
{
    WaitState& state = *static_cast<WaitState*>(context);
    state.durations.push_back(durationMilliseconds);
    for (const XWalkHal::XWalkMotor* const motor : state.motors)
    {
        state.observedSpeeds.push_back(motor->speed());
    }
    if (state.failNextWait)
    {
        state.failNextWait = false;
        XHAL_THROW_RUNTIME_ERROR("Simulated motor-sequence wait failure");
    }
}

void runTest()
{
    TestI2c i2cState;
    XWalkHal::XWalkI2c i2c(&i2cState, &probe, &writeRegister, &read,
        nullptr, &tryWriteRegister);
    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm pwm12(i2c, 12U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm13(i2c, 13U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm14(i2c, 14U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm15(i2c, 15U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm16(i2c, 16U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm17(i2c, 17U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm18(i2c, 18U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm19(i2c, 19U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkMotor firstMotor(pwm12, pwm13);
    XWalkHal::XWalkMotor secondMotor(pwm14, pwm15);
    XWalkHal::XWalkMotor thirdMotor(pwm16, pwm17);
    XWalkHal::XWalkMotor fourthMotor(pwm18, pwm19);
    WaitState waitState;
    waitState.motors = {{&firstMotor, &secondMotor, &thirdMotor, &fourthMotor}};
    xwalk::hal::test::XWalkRobotHat5MotorSequence sequence(firstMotor,
        secondMotor, thirdMotor, fourthMotor, &waitState, &wait);

    sequence.run(1U);

    assert(waitState.durations == XWalkHal::uint32vector({1'000U, 1'000U, 100U}));
    assert(waitState.observedSpeeds == XWalkHal::float64vector({
        -50.0, -50.0, -50.0, -50.0,
        50.0, 50.0, 50.0, 50.0,
        0.0, 0.0, 0.0, 0.0}));
    assert(firstMotor.speed() == 0.0);
    assert(secondMotor.speed() == 0.0);
    assert(thirdMotor.speed() == 0.0);
    assert(fourthMotor.speed() == 0.0);
    assert(i2cState.writeCount > 0U);

    XWalkHal::boolean rejectedCycles = false;
    try
    {
        sequence.run(0U);
    }
    catch (const std::out_of_range&)
    {
        rejectedCycles = true;
    }
    assert(rejectedCycles);

    waitState.durations.clear();
    waitState.observedSpeeds.clear();
    waitState.failNextWait = true;
    XWalkHal::boolean propagatedFailure = false;
    try
    {
        sequence.run(1U);
    }
    catch (const std::runtime_error&)
    {
        propagatedFailure = true;
    }
    assert(propagatedFailure);
    assert(waitState.durations == XWalkHal::uint32vector({1'000U, 100U}));
    assert(firstMotor.speed() == 0.0);
    assert(secondMotor.speed() == 0.0);
    assert(thirdMotor.speed() == 0.0);
    assert(fourthMotor.speed() == 0.0);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs the host-safe Robot HAT v5 motor-sequence verification. */
int xWalkRobotHat5MotorSequenceHostTest()
{
    runTest();
    return 0;
}
