/******************************************************************************
 * @file        xHal_Rpi5CarMotorSequenceLinux.cpp
 * @brief       Implements Linux composition for the two-motor Robot HAT sequence.
 *
 * @details
 * Creates PWM-and-direction motors from P13/D4 and P12/D5 using Linux-backed
 * Robot HAT I2C and GPIO interfaces.
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

#include "xHal_Rpi5CarMotorSequenceLinux.h"

#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

    void XWalkMotorSequenceLinux::run(
        cstring i2cDevice, cstring gpioDevice, stringview chipName, stringview chipLabel, uint32 cycleCount)
    {
        XWalkI2cLinux i2cBackend(i2cDevice);
        XWalkI2c i2c(&i2cBackend,
                     XHAL_I2C_PROBE_CALLBACK(XWalkI2cLinux),
                     XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkI2cLinux),
                     XHAL_I2C_READ_CALLBACK(XWalkI2cLinux),
                     nullptr,
                     XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(XWalkI2cLinux));
        XWalkPwmTimerState timerState;
        XWalkPwm pwm13(i2c, 13U, {}, timerState);
        XWalkPwm pwm12(i2c, 12U, {}, timerState);
        XWalkGpioLinux gpioBackend(gpioDevice, chipName, chipLabel, 25U);
        const XWalkGpioCallbacks callbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
        XWalkGpio gpioD4(&gpioBackend, callbacks, "D4");
        XWalkGpio gpioD5(&gpioBackend, callbacks, "D5");
        XWalkMotor firstMotor(pwm13, gpioD4);
        XWalkMotor secondMotor(pwm12, gpioD5);
        XWalkMotorSequence sequence(firstMotor, secondMotor, this, &XWalkMotorSequenceLinux::wait);
        sequence.run(cycleCount);
    }

    void XWalkMotorSequenceLinux::wait(contextpointer context, uint32 durationMilliseconds)
    {
        static_cast<void>(context);
        common::sleepMilliseconds(durationMilliseconds);
    }

} /* namespace xwalk::hal::test */
