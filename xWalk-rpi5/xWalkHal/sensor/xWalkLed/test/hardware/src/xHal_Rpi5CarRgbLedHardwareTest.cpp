/******************************************************************************
 * @file        xHal_Rpi5CarRgbLedHardwareTest.cpp
 * @brief       Provides an opt-in RGB LED hardware smoke test.
 *
 * @details
 * Composes Linux I2C, shared PWM timer state, three PWM channels, and the RGB
 * controller before requesting logical black from a connected RGB LED.
 *
 * @project     xWalk Firmware
 * @module      xWalkLed Hardware Test
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
#include "xHal_Rpi5CarRgbLed.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Requests logical black from an RGB LED connected to PWM channels zero through two.
 *
 * @return
 * Zero after all PWM writes complete successfully.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and changes physical PWM outputs.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkI2cLinux backend;
    XWalkHal::XWalkI2c i2c(&backend, XHAL_I2C_PROBE_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_READ_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux));
    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm red(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm green(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkPwm blue(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkRgbLed led(red, green, blue);
    led.setColor(0U);
    return 0;
}
