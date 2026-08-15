/******************************************************************************
 * @file        xHal_Rpi5CarSpiDeviceLinux.h
 * @brief       Declares the production Linux SPI system-call adapter.
 *
 * @details
 * Implements the injectable device-operation contract used by `XWalkSpiLinux`
 * with Linux device-node and ioctl system calls.
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

#ifndef XHAL_RPI5CAR_SPI_DEVICE_LINUX_H
#define XHAL_RPI5CAR_SPI_DEVICE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpiDevice.h"

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
     * @class XWalkSpiDeviceLinux
     * @brief Provides the production Linux SPI system-call implementation.
     */
    class XWalkSpiDeviceLinux final : public XWalkSpiDevice
    {
        public:
            /** @brief Constructs the stateless Linux system-call adapter. */
            XWalkSpiDeviceLinux();

            /** @brief Destroys the stateless Linux system-call adapter. */
            ~XWalkSpiDeviceLinux() override;

            XWalkSpiDeviceLinux(XWalkSpiDeviceLinux&&) = delete;
            XWalkSpiDeviceLinux(const XWalkSpiDeviceLinux&) = delete;
            XWalkSpiDeviceLinux& operator=(XWalkSpiDeviceLinux&&) = delete;
            XWalkSpiDeviceLinux& operator=(const XWalkSpiDeviceLinux&) = delete;

            /** @copydoc XWalkSpiDevice::openDevice */
            int32 openDevice(cstring devicePath) override;

            /** @copydoc XWalkSpiDevice::configureMode */
            boolean configureMode(int32 fileDescriptor, uint8& mode) override;

            /** @copydoc XWalkSpiDevice::configureBitsPerWord */
            boolean configureBitsPerWord(int32 fileDescriptor, uint8& bitsPerWord) override;

            /** @copydoc XWalkSpiDevice::configureSpeed */
            boolean configureSpeed(int32 fileDescriptor, uint32& speedHz) override;

            /** @copydoc XWalkSpiDevice::transfer */
            int32 transfer(int32 fileDescriptor, contextpointer request) override;

            /** @copydoc XWalkSpiDevice::closeDevice */
            void closeDevice(int32 fileDescriptor) noexcept override;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPI_DEVICE_LINUX_H */
