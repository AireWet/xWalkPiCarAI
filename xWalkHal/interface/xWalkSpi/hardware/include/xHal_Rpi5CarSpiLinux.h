/******************************************************************************
 * @file        xHal_Rpi5CarSpiLinux.h
 * @brief       Declares the Linux spidev backend for xWalk SPI.
 *
 * @details
 * Owns one configured Linux SPI device descriptor and serializes bounded
 * full-duplex transfers issued through the hardware-independent interface.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Linux Backend
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

#ifndef XHAL_RPI5CAR_SPI_LINUX_H
#define XHAL_RPI5CAR_SPI_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpi.h"
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
 * @class XWalkSpiLinux
 * @brief Provides serialized Linux `spidev` transactions.
 *
 * @details
 * Opens and owns exactly one deployment-selected device node. The kernel owns
 * chip-select assertion for each ioctl transaction. This object must outlive
 * every `XWalkSpi` callback binding that refers to it.
 */
class XWalkSpiLinux final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning device-operation implementation that must outlive this backend. */
        XWalkSpiDevice& deviceInterfaceValue;

        /** @brief Mutex serializing full-duplex transactions. */
        mutexhandle mutex;
        /** @brief Owned Linux SPI device descriptor, or negative when closed. */
        int32 fileDescriptor{-1};
        /** @brief Validated active clock frequency in Hertz. */
        uint32 speedHzValue{};
        /** @brief Validated standard SPI mode from zero through three. */
        uint8 modeValue{};
        /** @brief Validated word size from one through thirty-two bits. */
        uint8 bitsPerWordValue{};

    protected:
        /**
         * @brief Validates settings and opens one Linux SPI device node.
         * @param[in] devicePath Non-null, non-empty device path.
         * @param[in] configuration Requested Linux SPI configuration.
         * @return Owned non-negative descriptor.
         * @throws std::invalid_argument If the path is empty or speed is zero.
         * @throws std::out_of_range If mode or word size is outside its range.
         * @throws std::runtime_error If the device cannot be opened.
         */
        int32 openValidatedDevice(cstring devicePath,
            const XWalkSpiConfiguration& configuration);

        /**
         * @brief Validates and applies SPI mode, word size, and speed.
         * @param[in] configuration Requested Linux SPI configuration.
         * @throws std::runtime_error If Linux rejects a setting.
         */
        void configure(const XWalkSpiConfiguration& configuration);

    public:
        /**
         * @brief Opens and configures one Linux SPI device.
         * @param[in] devicePath Non-null, non-empty deployment-selected path.
         * @param[in] configuration Clock, mode, and word settings copied by value.
         */
        explicit XWalkSpiLinux(cstring devicePath = XHAL_RPI5CAR_SPI_DEFAULT_DEVICE,
            const XWalkSpiConfiguration& configuration = {});

        /**
         * @brief Constructs the backend with an injected device-operation interface.
         * @param[in,out] deviceInterface Device boundary that must outlive this backend.
         * @param[in] devicePath Non-null, non-empty logical device path.
         * @param[in] configuration Clock, mode, and word settings copied by value.
         */
        XWalkSpiLinux(XWalkSpiDevice& deviceInterface,
            cstring devicePath = XHAL_RPI5CAR_SPI_DEFAULT_DEVICE,
            const XWalkSpiConfiguration& configuration = {});

        /** @brief Closes the owned Linux SPI descriptor. */
        ~XWalkSpiLinux();

        XWalkSpiLinux(const XWalkSpiLinux&) = delete;
        XWalkSpiLinux& operator=(const XWalkSpiLinux&) = delete;
        XWalkSpiLinux(XWalkSpiLinux&&) = delete;
        XWalkSpiLinux& operator=(XWalkSpiLinux&&) = delete;

        /**
         * @brief Performs one chip-select-bounded full-duplex transfer.
         * @param[in] transmitData Non-empty payload containing at most 256 bytes.
         * @return Received bytes with exactly the transmitted length.
         * @throws std::invalid_argument If the payload is empty.
         * @throws std::out_of_range If the payload exceeds 256 bytes.
         * @throws std::runtime_error If the Linux transaction fails or is incomplete.
         */
        bytevector transfer(const bytevector& transmitData);
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPI_LINUX_H */
