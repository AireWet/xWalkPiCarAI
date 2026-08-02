/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345HardwareTest.cpp
 * @brief       Provides an opt-in ADXL345 hardware smoke test.
 *
 * @details
 * Composes the Linux I2C backend, callback interface, and ADXL345 driver, then
 * performs one three-axis read from a physically connected accelerometer.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Hardware Test
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

#include "xHal_Rpi5CarAdxl345.h"
#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Reads all three axes from a connected ADXL345.
 *
 * @return
 * Zero after a successful hardware read.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and requires an ADXL345 at `0x53`.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkI2cLinux backend;
    XWalkHal::XWalkI2c i2c(&backend, XHAL_I2C_PROBE_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(XWalkHal::XWalkI2cLinux),
        XHAL_I2C_READ_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux));
    XWalkHal::XWalkAdxl345 accelerometer(i2c);
    static_cast<void>(accelerometer.read());
    return 0;
}
