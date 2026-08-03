/******************************************************************************
 * @file        xHal_Rpi5CarServoSequenceLinux.cpp
 * @brief       Implements Linux composition for the 12-channel servo sequence.
 *
 * @details
 * Creates PWM and servo objects for channels zero through 11 on one
 * Linux-backed Robot HAT I2C interface.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
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

#include "xHal_Rpi5CarServoSequenceLinux.h"

#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

/**
 * @brief Runs bounded physical negative and positive servo sweeps.
 *
 * @param[in] i2cDevice
 * Linux I2C character-device path.
 *
 * @param[in] cycleCount
 * Complete sweep count in the inclusive range one through 100.
 *
 * @warning
 * This operation physically moves servo channels zero through 11.
 */
void XWalkServoSequenceLinux::run(cstring i2cDevice, uint32 cycleCount)
{
    XWalkI2cLinux i2cBackend(i2cDevice);
    XWalkI2c i2c(&i2cBackend, XHAL_I2C_PROBE_CALLBACK(XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(XWalkI2cLinux));
    XWalkPwmTimerState timerState;
    XWalkPwm pwm0(i2c, 0U, {}, timerState);
    XWalkPwm pwm1(i2c, 1U, {}, timerState);
    XWalkPwm pwm2(i2c, 2U, {}, timerState);
    XWalkPwm pwm3(i2c, 3U, {}, timerState);
    XWalkPwm pwm4(i2c, 4U, {}, timerState);
    XWalkPwm pwm5(i2c, 5U, {}, timerState);
    XWalkPwm pwm6(i2c, 6U, {}, timerState);
    XWalkPwm pwm7(i2c, 7U, {}, timerState);
    XWalkPwm pwm8(i2c, 8U, {}, timerState);
    XWalkPwm pwm9(i2c, 9U, {}, timerState);
    XWalkPwm pwm10(i2c, 10U, {}, timerState);
    XWalkPwm pwm11(i2c, 11U, {}, timerState);
    XWalkServo servo0(pwm0);
    XWalkServo servo1(pwm1);
    XWalkServo servo2(pwm2);
    XWalkServo servo3(pwm3);
    XWalkServo servo4(pwm4);
    XWalkServo servo5(pwm5);
    XWalkServo servo6(pwm6);
    XWalkServo servo7(pwm7);
    XWalkServo servo8(pwm8);
    XWalkServo servo9(pwm9);
    XWalkServo servo10(pwm10);
    XWalkServo servo11(pwm11);
    const servosequencearray servos{{
        &servo0, &servo1, &servo2, &servo3, &servo4, &servo5,
        &servo6, &servo7, &servo8, &servo9, &servo10, &servo11}};
    XWalkServoSequence sequence(
        servos, this, &XWalkServoSequenceLinux::wait);
    sequence.run(cycleCount);
}

/**
 * @brief Waits for the requested duration.
 *
 * @param[in,out] context
 * Unused callback context.
 *
 * @param[in] durationMilliseconds
 * Requested wait duration in milliseconds.
 */
void XWalkServoSequenceLinux::wait(
    contextpointer context, uint32 durationMilliseconds)
{
    static_cast<void>(context);
    common::sleepMilliseconds(durationMilliseconds);
}

} /* namespace xwalk::hal::test */
