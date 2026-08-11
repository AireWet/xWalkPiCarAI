/******************************************************************************
 * @file        xHal_Rpi5CarI2cLinux.h
 * @brief       Declares the Linux I2C backend for xWalk hardware.
 *
 * @details
 * Owns a Linux I2C device descriptor, serializes bus transactions, and exposes
 * callback-compatible probe, register-write, and sequential-read operations
 * to `XWalkI2c` with xWalk tracing at transaction and failure boundaries.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Linux Backend
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_I2C_LINUX_H
#define XHAL_RPI5CAR_I2C_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarI2cDevice.h"

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
 * @class XWalkI2cLinux
 * @brief Provides serialized Linux `i2c-dev` access for xWalk HAL clients.
 *
 * @details
 * Opens and owns one Linux I2C device descriptor and retries probe, write, and read
 * operations according to the configured attempt count. The application entry
 * point creates the separate `XWalkI2c` object and binds it to this backend.
 * Retry exhaustion and invalid backend state produce xWalk diagnostics.
 */
class XWalkI2cLinux
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning Linux device-operation implementation. */
        XWalkI2cDevice& deviceInterfaceValue;

        /** @brief Mutex serializing address selection and I2C transactions. */
        mutexhandle mutex;

        /** @brief Number of probe, write, or read attempts; always greater than zero. */
        uint32 retryCountValue{};

        /**
         * @brief Owned Linux I2C device descriptor.
         *
         * @note
         * A value of `-1` represents a closed or unavailable descriptor.
         */
        int32 fileDescriptor{-1};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Selects the active seven-bit I2C slave address.
         *
         * @param[in] address
         * Seven-bit I2C address passed to the Linux driver.
         *
         * @return
         * `true` when address selection succeeds; otherwise `false`.
         */
        boolean selectAddress(uint8 address);

        /**
         * @brief Opens a Linux I2C device node.
         *
         * @param[in] devicePath
         * Non-null path to the I2C device node.
         *
         * @return
         * Owned non-negative Linux file descriptor.
         *
         * @throws std::invalid_argument
         * If `devicePath` is null.
         *
         * @throws std::runtime_error
         * If the device cannot be opened.
         */
        int32 openDevice(cstring devicePath);

        /**
         * @brief Validates the configured I2C operation attempt count.
         *
         * @param[in] retryCount
         * Number of permitted attempts.
         *
         * @return
         * The validated non-zero attempt count.
         *
         * @throws std::invalid_argument
         * If `retryCount` is zero.
         */
        static uint32 validateRetryCount(uint32 retryCount);

        /**
         * @brief Performs one SMBus register-write attempt.
         *
         * @param[in] reg
         * Eight-bit destination register address.
         *
         * @param[in] payload
         * Payload containing between 1 and 32 bytes.
         *
         * @return
         * `true` when the Linux ioctl succeeds; otherwise `false`.
         *
         * @pre
         * The caller holds `mutex`, has selected the destination address, and
         * has validated the payload length.
         */
        boolean writeRegisterOnce(uint8 reg, const bytevector& payload);

        /**
         * @brief Reads one byte from the currently selected I2C device.
         *
         * @param[out] value
         * Byte returned by the Linux SMBus request when the operation succeeds.
         *
         * @return
         * `true` when the Linux ioctl succeeds; otherwise `false`.
         *
         * @pre
         * The caller holds `mutex` and has selected the destination address.
         */
        boolean readByteOnce(uint8& value);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a Linux I2C backend and opens its device node.
         *
         * @param[in] devicePath
         * Non-null path to the Linux I2C device node.
         *
         * @param[in] retryCount
         * Number of probe, write, or read attempts; valid range is 1 to `UINT32_MAX`.
         *
         * @post
         * The backend owns an open descriptor and is ready for serialized I2C
         * operations.
         *
         * @throws std::invalid_argument
         * If `devicePath` is null or `retryCount` is zero.
         *
         * @throws std::runtime_error
         * If the Linux device node cannot be opened.
         */
        explicit XWalkI2cLinux(cstring devicePath = XHAL_RPI5CAR_I2C_DEFAULT_DEVICE,
            uint32 retryCount = XHAL_RPI5CAR_I2C_RETRY_COUNT);

        /**
         * @brief Constructs the backend with an injected device-operation interface.
         * @param[in,out] deviceInterface Device boundary that must outlive this backend.
         * @param[in] devicePath Non-null logical device path passed to the interface.
         * @param[in] retryCount Number of permitted transaction attempts.
         */
        XWalkI2cLinux(XWalkI2cDevice& deviceInterface,
            cstring devicePath = XHAL_RPI5CAR_I2C_DEFAULT_DEVICE,
            uint32 retryCount = XHAL_RPI5CAR_I2C_RETRY_COUNT);

        /** @brief Closes the owned Linux I2C device descriptor. */
        ~XWalkI2cLinux();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction because callbacks refer to this object. */
        XWalkI2cLinux(XWalkI2cLinux&&) = delete;
        /** @brief Disables copying of the owned device descriptor. */
        XWalkI2cLinux(const XWalkI2cLinux&) = delete;
        /** @brief Disables move assignment of the owned device descriptor. */
        XWalkI2cLinux& operator=(XWalkI2cLinux&&) = delete;
        /** @brief Disables copy assignment of the owned device descriptor. */
        XWalkI2cLinux& operator=(const XWalkI2cLinux&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Probes an I2C address using bounded retries.
         *
         * @param[in] address
         * Seven-bit I2C address to probe.
         *
         * @return
         * `true` if any attempt receives a response; otherwise `false`.
         *
         * @throws std::out_of_range
         * If `address` exceeds the seven-bit protocol range.
         */
        boolean probeDevice(uint8 address);

        /**
         * @brief Reads bytes from an I2C device using bounded retries.
         *
         * @param[in] address
         * Seven-bit I2C source address.
         *
         * @param[in] length
         * Number of bytes requested in the range 1 to 32.
         *
         * @return
         * Exactly `length` bytes in bus order when the transaction succeeds.
         *
         * @throws std::invalid_argument
         * If `length` is zero.
         *
         * @throws std::out_of_range
         * If the address or length exceeds its supported range.
         *
         * @throws std::runtime_error
         * If every read attempt fails.
         */
        bytevector readDevice(uint8 address, size length);

        /**
         * @brief Reads consecutive bytes beginning at an I2C register.
         *
         * @param[in] address
         * Seven-bit I2C source address.
         *
         * @param[in] reg
         * First eight-bit register address to read.
         *
         * @param[in] length
         * Number of bytes requested in the range 1 to 32.
         *
         * @return
         * Exactly `length` bytes in ascending register order.
         *
         * @throws std::invalid_argument
         * If `length` is zero.
         *
         * @throws std::out_of_range
         * If the address or length exceeds its supported range.
         *
         * @throws std::runtime_error
         * If every register-read attempt fails.
         */
        bytevector readRegisterDevice(uint8 address, uint8 reg, size length);

        /**
         * @brief Writes a payload to an I2C register using bounded retries.
         *
         * @param[in] address
         * Seven-bit I2C destination address.
         *
         * @param[in] reg
         * Eight-bit destination register address.
         *
         * @param[in] payload
         * Payload containing between 1 and 32 bytes.
         *
         * @post
         * A successful return means one complete address-selection and write
         * sequence completed while the bus mutex was held.
         *
         * @throws std::invalid_argument
         * If the payload is empty.
         *
         * @throws std::out_of_range
         * If `address` exceeds the seven-bit protocol range or the payload
         * exceeds the SMBus block limit.
         *
         * @throws std::runtime_error
         * If every write attempt fails.
         */
        void writeRegisterDevice(uint8 address, uint8 reg, const bytevector& payload);

        /**
         * @brief Attempts a validated register write without throwing.
         *
         * @param[in] address
         * Seven-bit I2C destination address.
         *
         * @param[in] reg
         * Eight-bit destination register address.
         *
         * @param[in] payload
         * Payload containing between 1 and 32 bytes.
         *
         * @return
         * `true` when one complete retry attempt succeeds; otherwise `false`.
         *
         * @post
         * Invalid input and exhausted backend retries are reported as `false`.
         *
         * @note
         * This fail-safe path does not invoke trace operations so it remains
         * non-throwing even when trace output is unavailable.
         */
        boolean tryWriteRegisterDevice(uint8 address, uint8 reg,
            const bytevector& payload) noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_I2C_LINUX_H */
