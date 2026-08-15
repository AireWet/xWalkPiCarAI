/******************************************************************************
 * @file        xHal_Rpi5CarSpiDeviceLinux.cpp
 * @brief       Implements the production Linux SPI system-call adapter.
 *
 * @details
 * Forwards device open, SPI configuration, full-duplex transfer, and close
 * operations to the Linux kernel without adding backend policy.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpiDeviceLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /** @brief Constructs the stateless Linux system-call adapter. */
    XWalkSpiDeviceLinux::XWalkSpiDeviceLinux() = default;

    /** @brief Destroys the stateless Linux system-call adapter. */
    XWalkSpiDeviceLinux::~XWalkSpiDeviceLinux() = default;

    /**
     * @brief Opens one Linux SPI device node for read/write access.
     * @param[in] devicePath Non-null path forwarded to the Linux `open` operation.
     * @return Non-negative owned descriptor on success; otherwise a negative value.
     */
    int32 XWalkSpiDeviceLinux::openDevice(cstring devicePath)
    {
        return ::open(devicePath, O_RDWR | O_CLOEXEC);
    }

    /**
     * @brief Applies one standard SPI mode through the Linux driver.
     * @param[in] fileDescriptor Open SPI device descriptor.
     * @param[in,out] mode Requested mode and value retained by the driver call.
     * @return `true` when the ioctl succeeds; otherwise `false`.
     */
    boolean XWalkSpiDeviceLinux::configureMode(int32 fileDescriptor, uint8& mode)
    {
        return ::ioctl(fileDescriptor, SPI_IOC_WR_MODE, &mode) >= 0;
    }

    /**
     * @brief Applies one SPI word width through the Linux driver.
     * @param[in] fileDescriptor Open SPI device descriptor.
     * @param[in,out] bitsPerWord Requested word width in bits.
     * @return `true` when the ioctl succeeds; otherwise `false`.
     */
    boolean XWalkSpiDeviceLinux::configureBitsPerWord(int32 fileDescriptor, uint8& bitsPerWord)
    {
        return ::ioctl(fileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) >= 0;
    }

    /**
     * @brief Applies one SPI clock frequency through the Linux driver.
     * @param[in] fileDescriptor Open SPI device descriptor.
     * @param[in,out] speedHz Requested clock frequency in Hertz.
     * @return `true` when the ioctl succeeds; otherwise `false`.
     */
    boolean XWalkSpiDeviceLinux::configureSpeed(int32 fileDescriptor, uint32& speedHz)
    {
        return ::ioctl(fileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &speedHz) >= 0;
    }

    /**
     * @brief Executes one full-duplex Linux SPI message.
     * @param[in] fileDescriptor Open SPI device descriptor.
     * @param[in,out] request Non-null opaque pointer to one `spi_ioc_transfer` request.
     * @return Transferred byte count on success; otherwise a negative value.
     */
    int32 XWalkSpiDeviceLinux::transfer(int32 fileDescriptor, contextpointer request)
    {
        return ::ioctl(fileDescriptor, SPI_IOC_MESSAGE(1), request);
    }

    /**
     * @brief Closes one owned Linux SPI descriptor without throwing.
     * @param[in] fileDescriptor Descriptor passed to the Linux `close` operation.
     */
    void XWalkSpiDeviceLinux::closeDevice(int32 fileDescriptor) noexcept
    {
        static_cast<void>(::close(fileDescriptor));
    }

} /* namespace xwalk::hal */
