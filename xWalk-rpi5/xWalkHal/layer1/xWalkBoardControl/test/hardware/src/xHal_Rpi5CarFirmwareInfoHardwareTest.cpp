/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfoHardwareTest.cpp
 * @brief       Provides the physical Robot HAT firmware-information smoke test.
 *
 * @details
 * Composes the Linux I2C backend, probes supported Robot HAT addresses, and
 * reads the three-byte firmware version without changing hardware state.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Hardware Test
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarFirmwareInfo.h"
#include "xHal_Rpi5CarI2cLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Reads firmware-version information from a connected Robot HAT.
 *
 * @return
 * Zero after a complete read; an exception reports hardware failure.
 *
 * @warning
 * Running this function accesses `/dev/i2c-1` and requires a connected Robot HAT.
 */
XWalkHal::int32 main()
{
    XWalkHal::XWalkI2cLinux backend;
    XWalkHal::XWalkI2c i2c(&backend,
                           XHAL_I2C_PROBE_CALLBACK(XWalkHal::XWalkI2cLinux),
                           XHAL_I2C_WRITE_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux),
                           XHAL_I2C_READ_CALLBACK(XWalkHal::XWalkI2cLinux),
                           XHAL_I2C_READ_REGISTER_CALLBACK(XWalkHal::XWalkI2cLinux));
    XWalkHal::XWalkFirmwareInfo information(i2c);
    static_cast<void>(information.read());
    return 0;
}
