/******************************************************************************
 * @file        xHal_Rpi5CarGpioDeviceLinux.h
 * @brief       Declares the production Linux GPIO system-call adapter.
 *
 * @details
 * Implements `XWalkGpioDevice` with the Linux GPIO character-device ABI.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Linux Backend
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

#ifndef XHAL_RPI5CAR_GPIO_DEVICE_LINUX_H
#define XHAL_RPI5CAR_GPIO_DEVICE_LINUX_H

#include "xHal_Rpi5CarGpioDevice.h"

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /**
     * @class XWalkGpioDeviceLinux
     * @brief Provides the production Linux GPIO system-call implementation.
     */
    class XWalkGpioDeviceLinux final : public XWalkGpioDevice
    {
        public:
            /** @brief Constructs the stateless Linux system-call adapter. */
            XWalkGpioDeviceLinux();

            /** @brief Destroys the stateless Linux system-call adapter. */
            ~XWalkGpioDeviceLinux() override;

            XWalkGpioDeviceLinux(XWalkGpioDeviceLinux&&) = delete;
            XWalkGpioDeviceLinux(const XWalkGpioDeviceLinux&) = delete;
            XWalkGpioDeviceLinux& operator=(XWalkGpioDeviceLinux&&) = delete;
            XWalkGpioDeviceLinux& operator=(const XWalkGpioDeviceLinux&) = delete;

            /** @copydoc XWalkGpioDevice::openDevice */
            int32 openDevice(cstring devicePath) override;
            /** @copydoc XWalkGpioDevice::readChipInformation */
            boolean readChipInformation(int32 chipDescriptor, contextpointer information) override;
            /** @copydoc XWalkGpioDevice::requestLine */
            boolean requestLine(int32 chipDescriptor, contextpointer request) override;
            /** @copydoc XWalkGpioDevice::readLine */
            boolean readLine(int32 lineDescriptor, contextpointer data) override;
            /** @copydoc XWalkGpioDevice::writeLine */
            boolean writeLine(int32 lineDescriptor, contextpointer data) override;
            /** @copydoc XWalkGpioDevice::requestEvent */
            boolean requestEvent(int32 chipDescriptor, contextpointer request) override;
            /** @copydoc XWalkGpioDevice::pollEvent */
            int32 pollEvent(int32 lineDescriptor, int32 timeoutMs) override;
            /** @copydoc XWalkGpioDevice::readEvent */
            int32 readEvent(int32 lineDescriptor, contextpointer eventData, size length) override;
            /** @copydoc XWalkGpioDevice::closeDevice */
            void closeDevice(int32 fileDescriptor) noexcept override;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_GPIO_DEVICE_LINUX_H */
