/******************************************************************************
 * @file        xHal_Rpi5CarI2cDeviceLinux.h
 * @brief       Declares the production Linux I2C system-call adapter.
 *
 * @details
 * Implements the injectable device-operation contract used by
 * `XWalkI2cLinux` with Linux device-node and ioctl system calls.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_DEVICE_LINUX_H
#define XHAL_RPI5CAR_I2C_DEVICE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2cDevice.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkI2cDeviceLinux
     * @brief Provides the production Linux system-call implementation.
     */
    class XWalkI2cDeviceLinux final : public XWalkI2cDevice
    {
        public:
            /** @brief Constructs the stateless Linux system-call adapter. */
            XWalkI2cDeviceLinux();

            /** @brief Destroys the stateless Linux system-call adapter. */
            ~XWalkI2cDeviceLinux() override;

            XWalkI2cDeviceLinux(XWalkI2cDeviceLinux&&) = delete;
            XWalkI2cDeviceLinux(const XWalkI2cDeviceLinux&) = delete;
            XWalkI2cDeviceLinux& operator=(XWalkI2cDeviceLinux&&) = delete;
            XWalkI2cDeviceLinux& operator=(const XWalkI2cDeviceLinux&) = delete;

            /** @copydoc XWalkI2cDevice::openDevice */
            int32 openDevice(cstring devicePath) override;

            /** @copydoc XWalkI2cDevice::selectAddress */
            boolean selectAddress(int32 fileDescriptor, uint8 address) override;

            /** @copydoc XWalkI2cDevice::transfer */
            boolean transfer(int32 fileDescriptor, contextpointer request) override;

            /** @copydoc XWalkI2cDevice::closeDevice */
            void closeDevice(int32 fileDescriptor) noexcept override;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_I2C_DEVICE_LINUX_H */
