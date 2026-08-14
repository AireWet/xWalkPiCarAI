/******************************************************************************
 * @file        xHal_Rpi5CarSpiHostStub.h
 * @brief       Declares the device-free xWalkSpi host stub.
 *
 * @details
 * Mirrors Linux SPI configuration and transfers into owned state and returns
 * deterministic data without opening a physical spidev node.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Simulation
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

#ifndef XHAL_RPI5CAR_SPI_HOST_STUB_H
#define XHAL_RPI5CAR_SPI_HOST_STUB_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpiDevice.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::sim
 * @brief Contains device-free and executable-level xWalkSpi simulation support.
 */
namespace xwalk::hal::sim
{

/**
 * @class XWalkSpiHostStub
 * @brief Mirrors SPI traffic for device-free host execution.
 *
 * @details
 * Implements the device-operation boundary injected into `XWalkSpiLinux`, so
 * the host runner executes the same configuration and transaction logic as the
 * Raspberry Pi path.
 */
class XWalkSpiHostStub final: public XWalkSpiDevice
{
    private:
        /** @brief Owned copy of the most recently transmitted bytes. */
        bytevector lastTransmitDataValue;

        /** @brief Deterministic response copied into the next receive buffer. */
        bytevector responseDataValue{0x00U, 0xEFU, 0x40U, 0x18U};

        /** @brief Most recently configured SPI clock frequency in Hertz. */
        uint32 speedHzValue{};

        /** @brief Most recently configured standard SPI mode. */
        uint8 modeValue{};

        /** @brief Most recently configured bits-per-word value. */
        uint8 bitsPerWordValue{};

    public:
        /** @brief Constructs an empty device-free SPI mirror. */
        XWalkSpiHostStub();

        /** @brief Destroys the mirror and its owned transfer record. */
        ~XWalkSpiHostStub() override;

        XWalkSpiHostStub(XWalkSpiHostStub&&) = delete;
        XWalkSpiHostStub(const XWalkSpiHostStub&) = delete;
        XWalkSpiHostStub& operator=(XWalkSpiHostStub&&) = delete;
        XWalkSpiHostStub& operator=(const XWalkSpiHostStub&) = delete;

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

        /** @brief Returns the most recently mirrored transmit payload. */
        const bytevector& lastTransmitData() const noexcept;

        /** @brief Returns the most recently configured SPI clock in Hertz. */
        uint32 speedHz() const noexcept;

        /** @brief Returns the most recently configured standard SPI mode. */
        uint8 mode() const noexcept;

        /** @brief Returns the most recently configured bits-per-word value. */
        uint8 bitsPerWord() const noexcept;
};

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_SPI_HOST_STUB_H */
