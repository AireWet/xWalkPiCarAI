/******************************************************************************
 * @file        xHal_Rpi5CarI2cDevice.h
 * @brief       Declares the injectable Linux I2C device-operation boundary.
 *
 * @details
 * Separates Linux device-node and ioctl access from the retry, validation, and
 * transaction behavior implemented by `XWalkI2cLinux`.
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

#ifndef XHAL_RPI5CAR_I2C_DEVICE_H
#define XHAL_RPI5CAR_I2C_DEVICE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkI2cDevice
     * @brief Abstracts the operating-system calls used by the Linux I2C backend.
     *
     * @details
     * Production uses the Linux implementation. Host simulation supplies a mirror
     * that implements the same boundary without opening a physical device.
     */
    class XWalkI2cDevice
    {
        public:
            /** @brief Allows destruction through the interface. */
            virtual ~XWalkI2cDevice() = default;

            /** @brief Opens and returns an owned I2C device descriptor. */
            virtual int32 openDevice(cstring devicePath) = 0;

            /** @brief Selects the seven-bit address for a subsequent transfer. */
            virtual boolean selectAddress(int32 fileDescriptor, uint8 address) = 0;

            /** @brief Executes one Linux SMBus request supplied as opaque context. */
            virtual boolean transfer(int32 fileDescriptor, contextpointer request) = 0;

            /** @brief Closes an owned descriptor without throwing. */
            virtual void closeDevice(int32 fileDescriptor) noexcept = 0;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_I2C_DEVICE_H */
