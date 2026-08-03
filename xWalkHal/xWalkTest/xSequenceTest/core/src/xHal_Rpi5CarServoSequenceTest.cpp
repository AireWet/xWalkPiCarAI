/******************************************************************************
 * @file        xHal_Rpi5CarServoSequenceTest.cpp
 * @brief       Verifies the 12-channel Robot HAT servo sequence in memory.
 *
 * @details
 * Checks channel order, negative and positive angles, per-command timing,
 * final positions, and cycle validation without moving physical servos.
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

#include "xHal_Rpi5CarServoSequence.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains in-memory I2C callbacks and the host verification scenario. */
namespace
{

/** @brief Counts writes performed through the simulated Robot HAT I2C bus. */
struct I2cState
{
    /** @brief Total number of simulated register writes. */
    XWalkHal::uint32 writeCount{};
};

/** @brief Collects wait durations and complete PWM snapshots. */
struct WaitState
{
    /** @brief Non-owning PWM pointers ordered by channels zero through 11. */
    XWalkHal::fixedarray<XWalkHal::XWalkPwm*, 12U> pwmObjects{};
    /** @brief Ordered callback wait durations in milliseconds. */
    XWalkHal::uint32vector durations;
    /** @brief Flattened PWM snapshots captured after every angle command. */
    XWalkHal::uint32vector pulseWidthSnapshots;
};

/**
 * @brief Reports every simulated Robot HAT I2C address as available.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] address Unused seven-bit I2C address.
 *
 * @return Always `true`.
 */
XWalkHal::boolean probe(
    XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/**
 * @brief Counts a simulated I2C register write.
 *
 * @param[in,out] context Non-null `I2cState` pointer.
 * @param[in] address Unused seven-bit I2C address.
 * @param[in] reg Unused register address.
 * @param[in] data Unused register payload.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    I2cState& state = *static_cast<I2cState*>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    ++state.writeCount;
}

/**
 * @brief Returns a zero-filled simulated I2C payload.
 *
 * @param[in,out] context Unused backend context.
 * @param[in] address Unused seven-bit I2C address.
 * @param[in] length Requested response length in bytes.
 *
 * @return Zero-filled payload containing `length` bytes.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context,
    XWalkHal::uint8 address, XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return XWalkHal::bytevector(length, 0U);
}

/**
 * @brief Records one wait duration and all current PWM counts.
 *
 * @param[in,out] context Non-null `WaitState` pointer.
 * @param[in] durationMilliseconds Requested wait duration in milliseconds.
 */
void wait(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMilliseconds)
{
    WaitState& state = *static_cast<WaitState*>(context);
    state.durations.push_back(durationMilliseconds);
    for (const XWalkHal::XWalkPwm* const pwm : state.pwmObjects)
    {
        state.pulseWidthSnapshots.push_back(pwm->pulseWidth());
    }
}

/**
 * @brief Exercises one complete sweep against an in-memory I2C backend.
 *
 * @post Every assertion covering order, angles, timing, and bounds passes.
 */
void runTest()
{
    I2cState i2cState;
    XWalkHal::XWalkI2c i2c(
        &i2cState, &probe, &writeRegister, &read);
    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm pwm0(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm1(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm2(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm3(i2c, 3U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm4(i2c, 4U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm5(i2c, 5U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm6(i2c, 6U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm7(i2c, 7U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm8(i2c, 8U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm9(i2c, 9U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm10(i2c, 10U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm pwm11(i2c, 11U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkServo servo0(pwm0);
    XWalkHal::XWalkServo servo1(pwm1);
    XWalkHal::XWalkServo servo2(pwm2);
    XWalkHal::XWalkServo servo3(pwm3);
    XWalkHal::XWalkServo servo4(pwm4);
    XWalkHal::XWalkServo servo5(pwm5);
    XWalkHal::XWalkServo servo6(pwm6);
    XWalkHal::XWalkServo servo7(pwm7);
    XWalkHal::XWalkServo servo8(pwm8);
    XWalkHal::XWalkServo servo9(pwm9);
    XWalkHal::XWalkServo servo10(pwm10);
    XWalkHal::XWalkServo servo11(pwm11);
    const xwalk::hal::test::servosequencearray servos{{
        &servo0, &servo1, &servo2, &servo3, &servo4, &servo5,
        &servo6, &servo7, &servo8, &servo9, &servo10, &servo11}};
    WaitState waitState;
    waitState.pwmObjects = {{
        &pwm0, &pwm1, &pwm2, &pwm3, &pwm4, &pwm5,
        &pwm6, &pwm7, &pwm8, &pwm9, &pwm10, &pwm11}};
    xwalk::hal::test::XWalkServoSequence sequence(
        servos, &waitState, &wait);

    sequence.run(1U);

    assert(waitState.durations.size() == 24U);
    assert(waitState.pulseWidthSnapshots.size() == 288U);
    for (XWalkHal::size waitIndex = 0U; waitIndex < 24U; ++waitIndex)
    {
        assert(waitState.durations[waitIndex] == 100U);
        const XWalkHal::size activeChannel = waitIndex % 12U;
        const XWalkHal::boolean positiveSweep = waitIndex >= 12U;
        for (XWalkHal::size channel = 0U; channel < 12U; ++channel)
        {
            XWalkHal::uint32 expectedPulseWidth{};
            if (positiveSweep)
            {
                expectedPulseWidth = (channel <= activeChannel) ? 352U : 261U;
            }
            else if (channel <= activeChannel)
            {
                expectedPulseWidth = 261U;
            }
            const XWalkHal::size snapshotIndex = (waitIndex * 12U) + channel;
            assert(waitState.pulseWidthSnapshots[snapshotIndex] ==
                expectedPulseWidth);
        }
    }
    for (const XWalkHal::XWalkPwm* const pwm : waitState.pwmObjects)
    {
        assert(pwm->pulseWidth() == 352U);
    }
    assert(i2cState.writeCount > 0U);

    XWalkHal::boolean rejectedCycles = false;
    try
    {
        sequence.run(0U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedCycles = true;
    }
    assert(rejectedCycles);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe 12-channel servo-sequence verification.
 *
 * @return Zero after every assertion passes.
 */
int xWalkServoSequenceHostTest()
{
    runTest();
    return 0;
}
