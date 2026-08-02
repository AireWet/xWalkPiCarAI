/******************************************************************************
 * @file        xHal_Rpi5CarSpiLinuxHardwareTest.cpp
 * @brief       Provides an opt-in Linux SPI transfer smoke test.
 *
 * @details
 * Opens one explicitly selected spidev node and transmits one zero byte. The
 * executable must only run with a reviewed peripheral and safe chip select.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Hardware Test
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xHal_Rpi5CarSpiLinux.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Opens the selected SPI device and performs one single-byte transaction.
 * @param[in] argumentCount Must equal two.
 * @param[in] arguments Device path at index one.
 * @return Zero when the complete transfer succeeds; one for invalid arguments.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    if (argumentCount != 2)
    {
        return 1;
    }
    XWalkHal::XWalkSpiLinux backend(arguments[1]);
    XWalkHal::XWalkSpi spi(&backend, XHAL_SPI_TRANSFER_CALLBACK(XWalkHal::XWalkSpiLinux));
    static_cast<void>(spi.transfer({0x00U}));
    return 0;
}
