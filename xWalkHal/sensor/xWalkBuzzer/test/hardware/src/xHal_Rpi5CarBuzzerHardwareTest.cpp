/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerHardwareTest.cpp
 * @brief       Provides an opt-in passive-buzzer hardware smoke test.
 *
 * @details
 * Composes Linux I2C, shared PWM state, one PWM channel, and a passive buzzer
 * controller before explicitly requesting the inactive output state.
 *
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Hardware Test
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

#include "xHal_Rpi5CarBuzzer.h"
#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Places a passive buzzer on PWM channel zero into its inactive state.
 *
 * @return
 * Zero after all PWM writes complete successfully.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and changes PWM channel zero.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkI2cLinux backend;
    XWalkHal::XWalkI2c i2c(&backend, XHAL_I2C_PROBE_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_READ_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux));
    XWalkHal::XWalkPwmTimerState timerState;
    XWalkHal::XWalkPwm pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState);
    XWalkHal::XWalkBuzzer buzzer(pwm);
    buzzer.off();
    return 0;
}
