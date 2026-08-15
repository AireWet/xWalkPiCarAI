/******************************************************************************
 * @file        xHal_Rpi5CarServoHardwareTest.cpp
 * @brief       Provides the physical Robot HAT Servo construction test.
 *
 * @details
 * Creates each dependency in the composition root and configures PWM channel
 * zero for Servo timing. This executable must only run on prepared hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo Hardware Test
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
#include "xHal_Rpi5CarServo.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Configures Robot HAT PWM channel zero for positional-servo timing.
 *
 * @return
 * Zero after construction succeeds. I2C failures propagate as exceptions.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and writes physical hardware.
 */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkI2cLinux backend;
    xwalk::hal::XWalkI2c i2c(&backend,
                             XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux));
    xwalk::hal::XWalkPwmTimerState timerState;
    xwalk::hal::XWalkPwm pwm(i2c, 0U, {}, timerState);
    xwalk::hal::XWalkServo servo(pwm);
    static_cast<void>(servo.initialize());
    return 0;
}
