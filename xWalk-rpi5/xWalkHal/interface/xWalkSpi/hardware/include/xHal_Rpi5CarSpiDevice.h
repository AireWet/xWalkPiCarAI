/******************************************************************************
 * @file        xHal_Rpi5CarSpiDevice.h
 * @brief       Declares the injectable Linux SPI device-operation boundary.
 *
 * @details
 * Separates Linux device-node and ioctl access from the validation,
 * configuration, and transfer behavior implemented by `XWalkSpiLinux`.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SPI_DEVICE_H
#define XHAL_RPI5CAR_SPI_DEVICE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkSpiDevice
     * @brief Abstracts the operating-system calls used by the Linux SPI backend.
     *
     * @details
     * Production uses the Linux implementation. Host simulation supplies a mirror
     * that implements the same boundary without opening a physical SPI device.
     */
    class XWalkSpiDevice
    {
        public:
            /**************************************************************************
             * Public destructor
             **************************************************************************/

            /** @brief Allows destruction through the interface. */
            virtual ~XWalkSpiDevice() = default;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /** @brief Opens and returns an owned SPI device descriptor. */
            virtual int32 openDevice(cstring devicePath) = 0;

            /** @brief Applies one standard SPI mode value. */
            virtual boolean configureMode(int32 fileDescriptor, uint8& mode) = 0;

            /** @brief Applies one bits-per-word value. */
            virtual boolean configureBitsPerWord(int32 fileDescriptor, uint8& bitsPerWord) = 0;

            /** @brief Applies one SPI clock frequency in Hertz. */
            virtual boolean configureSpeed(int32 fileDescriptor, uint32& speedHz) = 0;

            /** @brief Executes one Linux SPI request supplied as opaque context. */
            virtual int32 transfer(int32 fileDescriptor, contextpointer request) = 0;

            /** @brief Closes an owned descriptor without throwing. */
            virtual void closeDevice(int32 fileDescriptor) noexcept = 0;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPI_DEVICE_H */
