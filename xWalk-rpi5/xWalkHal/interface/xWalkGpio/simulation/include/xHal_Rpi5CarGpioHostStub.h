/******************************************************************************
 * @file        xHal_Rpi5CarGpioHostStub.h
 * @brief       Declares the device-free xWalkGpio host stub.
 *
 * @details
 * Mirrors Linux GPIO chip, line, value, and event requests without opening a
 * physical GPIO character device.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Simulation
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

#ifndef XHAL_RPI5CAR_GPIO_HOST_STUB_H
#define XHAL_RPI5CAR_GPIO_HOST_STUB_H

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarGpioDevice.h"

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkGpio simulation support.
 */
namespace xwalk::hal::sim
{

/**
 * @class XWalkGpioHostStub
 * @brief Mirrors Linux GPIO device traffic for host execution.
 *
 * @details
 * Implements the injected device-operation boundary so host execution reaches
 * production chip validation, line requests, and digital-I/O logic.
 */
class XWalkGpioHostStub final: public XWalkGpioDevice
{
    private:
        /** @brief Most recently claimed Linux GPIO line offset. */
        uint8 pinValue{};
        /** @brief Most recently requested GPIO direction. */
        XWalkGpioMode modeValue{XWalkGpioMode::Output};
        /** @brief Most recently requested GPIO bias. */
        XWalkGpioPull pullValue{XWalkGpioPull::None};
        /** @brief Mirrored physical input or output level. */
        boolean lineValue{};
        /** @brief Number of successful mirrored line configurations. */
        size configureCountValue{};
        /** @brief Number of successful mirrored line reads. */
        size readCountValue{};
        /** @brief Number of successful mirrored line writes. */
        size writeCountValue{};

    public:
        /** @brief Constructs an empty device-free GPIO mirror. */
        XWalkGpioHostStub();
        /** @brief Destroys the device-free GPIO mirror. */
        ~XWalkGpioHostStub() override;

        XWalkGpioHostStub(XWalkGpioHostStub&&) = delete;
        XWalkGpioHostStub(const XWalkGpioHostStub&) = delete;
        XWalkGpioHostStub& operator=(XWalkGpioHostStub&&) = delete;
        XWalkGpioHostStub& operator=(const XWalkGpioHostStub&) = delete;

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

        /** @brief Selects the physical level returned by the next mirrored read. */
        void setInputValue(boolean value) noexcept;

        /** @brief Returns the most recently claimed GPIO line offset. */
        uint8 pin() const noexcept;
        /** @brief Returns the most recently requested GPIO direction. */
        XWalkGpioMode mode() const noexcept;
        /** @brief Returns the most recently requested GPIO bias. */
        XWalkGpioPull pull() const noexcept;
        /** @brief Returns the current mirrored physical line level. */
        boolean value() const noexcept;
        /** @brief Returns the number of mirrored line configurations. */
        size configureCount() const noexcept;
        /** @brief Returns the number of mirrored reads. */
        size readCount() const noexcept;
        /** @brief Returns the number of mirrored writes. */
        size writeCount() const noexcept;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_GPIO_HOST_STUB_H */
