/******************************************************************************
 * @file        xHal_Rpi5CarPwmHardwareTest.cpp
 * @brief       Provides the physical Robot HAT zero-output PWM test.
 *
 * @details
 * Creates the backend, callback interface, timer state, and PWM channel as
 * separate objects, then writes a zero pulse width. This executable must only
 * run on prepared hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Hardware Test
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

#include "xHal_Rpi5CarI2cLinux.h"
#include "xHal_Rpi5CarPwm.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Writes a zero pulse width to Robot HAT PWM channel zero.
 *
 * @return
 * Zero after the write completes. Construction or I2C failures propagate as
 * exceptions.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and writes physical hardware.
 */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkI2cLinux backend;
    xwalk::hal::XWalkI2c i2c(&backend, XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux));
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm pwm(i2c, 0U, {}, timerState);
    pwm.setPulseWidth(0.0);
    return 0;
}
