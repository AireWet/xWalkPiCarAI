/******************************************************************************
 * @file        xHal_Rpi5CarRobotHat5MotorSequenceLinux.cpp
 * @brief       Implements Linux composition for the Robot HAT v5 motor sequence.
 *
 * @details
 * Creates dual-PWM motors from channel pairs 12/13, 14/15, 16/17, and 18/19
 * on one Linux-backed Robot HAT I2C interface.
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

#include "xHal_Rpi5CarRobotHat5MotorSequenceLinux.h"

#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

    void XWalkRobotHat5MotorSequenceLinux::run(cstring i2cDevice, uint32 cycleCount)
    {
        XWalkI2cLinux i2cBackend(i2cDevice);
        XWalkI2c i2c(&i2cBackend,
                     XHAL_I2C_PROBE_CALLBACK(XWalkI2cLinux),
                     XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkI2cLinux),
                     XHAL_I2C_READ_CALLBACK(XWalkI2cLinux),
                     nullptr,
                     XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(XWalkI2cLinux));
        XWalkPwmTimerState timerState;
        XWalkPwm pwm12(i2c, 12U, {}, timerState);
        XWalkPwm pwm13(i2c, 13U, {}, timerState);
        XWalkPwm pwm14(i2c, 14U, {}, timerState);
        XWalkPwm pwm15(i2c, 15U, {}, timerState);
        XWalkPwm pwm16(i2c, 16U, {}, timerState);
        XWalkPwm pwm17(i2c, 17U, {}, timerState);
        XWalkPwm pwm18(i2c, 18U, {}, timerState);
        XWalkPwm pwm19(i2c, 19U, {}, timerState);
        XWalkMotor firstMotor(pwm12, pwm13);
        XWalkMotor secondMotor(pwm14, pwm15);
        XWalkMotor thirdMotor(pwm16, pwm17);
        XWalkMotor fourthMotor(pwm18, pwm19);
        XWalkRobotHat5MotorSequence sequence(
            firstMotor, secondMotor, thirdMotor, fourthMotor, this, &XWalkRobotHat5MotorSequenceLinux::wait);
        sequence.run(cycleCount);
    }

    void XWalkRobotHat5MotorSequenceLinux::wait(contextpointer context, uint32 durationMilliseconds)
    {
        static_cast<void>(context);
        common::sleepMilliseconds(durationMilliseconds);
    }

} /* namespace xwalk::hal::test */
