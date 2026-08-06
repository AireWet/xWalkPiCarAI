/******************************************************************************
 * @file        xHal_Rpi5CarI2cLinuxHardwareTest.cpp
 * @brief       Provides the physical Robot HAT I2C address-probe test.
 *
 * @details
 * Creates the Linux backend and callback interface as separate objects, then
 * succeeds when any supported Robot HAT address responds. This executable must
 * only run on prepared hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Hardware Test
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

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Probes the supported Robot HAT addresses on a selected I2C bus.
 *
 * @param[in] argumentCount Required process-argument count.
 * @param[in] arguments Configured I2C device path at index one.
 * @return
 * Zero if any supported address responds, one if none responds, or two for invalid arguments.
 *
 * @warning
 * Running this function opens `/dev/i2c-1` and requires connected hardware.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount,
    XWalkHal::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 2;
    }
    xwalk::hal::XWalkI2cLinux backend(arguments[1U]);
    xwalk::hal::XWalkI2c i2c(&backend, XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
        XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
        XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux));

    const hal::boolean primaryAddressAvailable =
        static_cast<hal::boolean>(
            i2c.probe(XHAL_RPI5CAR_I2C_ADDRESS_1));
    if (primaryAddressAvailable)
    {
        return 0;
    }
    const hal::boolean secondaryAddressAvailable =
        static_cast<hal::boolean>(
            i2c.probe(XHAL_RPI5CAR_I2C_ADDRESS_2));
    if (secondaryAddressAvailable)
    {
        return 0;
    }
    const hal::boolean secondaryAddressAvailable =
        static_cast<hal::boolean>(
            i2c.probe(XHAL_RPI5CAR_I2C_ADDRESS_3));
    if (secondaryAddressAvailable)
    {
        return 0;
    }
    return 1;
}
