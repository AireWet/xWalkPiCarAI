/******************************************************************************
 * @file        xHal_Rpi5CarAdcHardwareTest.cpp
 * @brief       Provides the physical Robot HAT ADC smoke test.
 *
 * @details
 * Constructs the Linux I2C backend and reads ADC channel zero from connected hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdc Hardware Test
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
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Reads one sample from physical ADC channel zero.
 *
 * @return
 * Zero after a complete read; an exception reports hardware failure.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and requires a connected Robot HAT.
 */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkI2cLinux backend;
    xwalk::hal::XWalkI2c i2c(&backend,
                             XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
                             XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux));
    xwalk::hal::XWalkAdc adc(i2c, 0U);
    static_cast<void>(adc.read());
    return 0;
}
