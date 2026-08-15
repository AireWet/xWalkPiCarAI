/******************************************************************************
 * @file        xHal_Rpi5CarMotorHardwareTest.cpp
 * @brief       Provides the physical Robot HAT motor stop smoke test.
 *
 * @details
 * Constructs one PWM-and-direction motor and commands zero speed. This executable is opt-in and must only
 * run with mechanically safe hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkMotor Hardware Test
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

#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"
#include "xHal_Rpi5CarMotor.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Configures motor one and commands zero-percent speed.
 *
 * @return
 * Zero after the stop command completes; an exception reports hardware failure.
 *
 * @warning
 * Running this function accesses physical I2C and GPIO devices and may affect connected motor hardware.
 */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkI2cLinux i2cBackend;
    xwalk::hal::XWalkI2c i2c(&i2cBackend,
                             XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             nullptr,
                             XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux));
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm pwm(i2c, "P13", {}, timerState);
    xwalk::hal::XWalkGpioLinux gpioBackend;
    const xwalk::hal::XWalkGpioCallbacks callbacks = XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux);
    xwalk::hal::XWalkGpio direction(&gpioBackend, callbacks, "D4");
    xwalk::hal::XWalkMotor motor(pwm, direction);
    motor.stop();
    return 0;
}
