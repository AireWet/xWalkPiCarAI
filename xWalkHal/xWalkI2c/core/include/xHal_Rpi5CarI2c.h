/******************************************************************************
 * @file        xHal_Rpi5CarI2c.h
 * @brief       Declares the hardware-independent xWalk I2C interface.
 *
 * @details
 * Defines a concrete callback-driven I2C object used by HAL modules without
 * coupling those modules to a platform-specific device implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c
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

#ifndef XHAL_RPI5CAR_I2C_H
#define XHAL_RPI5CAR_I2C_H

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
 * @class XWalkI2c
 * @brief Provides a hardware-independent I2C callback interface.
 *
 * @details
 * Stores a non-owning backend context and validated probe, write, and read
 * callbacks. The backend object and callback targets must outlive this object.
 */
class XWalkI2c
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /**
         * @brief Non-owning opaque pointer passed to each backend callback.
         *
         * @note
         * Null is permitted only when every configured callback supports it.
         */
        contextpointer contextValue{};

        /** @brief Non-null callback used to probe a seven-bit I2C address. */
        i2cprobecallback probeCallback{};

        /** @brief Non-null callback used to write an I2C register payload. */
        i2cwriteregistercallback writeRegisterCallback{};

        /** @brief Non-null callback used to read bytes from an I2C device. */
        i2creadcallback readCallback{};

        /** @brief Nullable callback used for atomic register-addressed reads. */
        i2creadregistercallback readRegisterCallback{};

        /** @brief Nullable non-throwing callback reserved for fail-safe writes. */
        i2ctrywriteregistercallback tryWriteRegisterCallback{};

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs an I2C interface bound to backend callbacks.
         *
         * @param[in] context
         * Non-owning opaque backend context forwarded to every callback.
         *
         * @param[in] probeOperation
         * Callback used to test whether a device responds at an address.
         *
         * @param[in] writeRegisterOperation
         * Callback used to write bytes to a device register.
         *
         * @param[in] readOperation
         * Callback used to read bytes directly from a device.
         *
         * @param[in] readRegisterOperation
         * Optional callback used to read bytes beginning at a register address.
         *
         * @param[in] tryWriteRegisterOperation
         * Optional non-throwing callback used by fail-safe register writes.
         *
         * @pre
         * The context satisfies the requirements of every configured callback.
         *
         * @throws std::invalid_argument
         * If a required probe, write-register, or sequential-read callback is null.
         */
        XWalkI2c(contextpointer context, i2cprobecallback probeOperation,
            i2cwriteregistercallback writeRegisterOperation, i2creadcallback readOperation,
            i2creadregistercallback readRegisterOperation = nullptr,
            i2ctrywriteregistercallback tryWriteRegisterOperation = nullptr);

        /** @brief Destroys the callback interface without owning its backend. */
        ~XWalkI2c();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction to preserve callback context identity. */
        XWalkI2c(XWalkI2c&&) = delete;
        /** @brief Disables copying of the callback binding. */
        XWalkI2c(const XWalkI2c&) = delete;
        /** @brief Disables move assignment to preserve callback context identity. */
        XWalkI2c& operator=(XWalkI2c&&) = delete;
        /** @brief Disables copy assignment of the callback binding. */
        XWalkI2c& operator=(const XWalkI2c&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Probes a device through the configured backend.
         *
         * @param[in] address
         * Seven-bit I2C address to probe.
         *
         * @return
         * `true` when the backend reports a responding device; otherwise
         * `false`.
         *
         * @pre
         * The backend context remains valid.
         *
         * @throws std::out_of_range
         * If `address` exceeds the seven-bit protocol range.
         */
        boolean probe(uint8 address);

        /**
         * @brief Reads a requested number of bytes from an I2C device.
         *
         * @param[in] address
         * Seven-bit I2C source address.
         *
         * @param[in] length
         * Non-zero number of bytes requested.
         *
         * @return
         * Bytes returned by the configured backend in bus order.
         *
         * @throws std::invalid_argument
         * If `length` is zero.
         *
         * @throws std::out_of_range
         * If `address` exceeds the seven-bit protocol range.
         */
        bytevector read(uint8 address, size length);

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
         * Non-zero number of consecutive bytes requested.
         *
         * @return
         * Bytes returned by the backend in ascending register order.
         *
         * @throws std::invalid_argument
         * If `length` is zero.
         *
         * @throws std::out_of_range
         * If `address` exceeds the seven-bit protocol range.
         *
         * @throws std::runtime_error
         * If no register-read callback was supplied during construction.
         */
        bytevector readRegister(uint8 address, uint8 reg, size length);

        /**
         * @brief Writes a byte payload to an I2C device register.
         *
         * @param[in] address
         * Seven-bit I2C destination address.
         *
         * @param[in] reg
         * Eight-bit destination register address.
         *
         * @param[in] data
         * Payload bytes interpreted by the configured backend.
         *
         * @pre
         * The backend context remains valid.
         *
         * @post
         * The callback has completed the write or reported failure according
         * to the backend's error policy.
         *
         * @throws std::out_of_range
         * If `address` exceeds the seven-bit protocol range.
         */
        void writeRegister(uint8 address, uint8 reg, const bytevector& data);

        /**
         * @brief Attempts one register write through the non-throwing fail-safe callback.
         *
         * @param[in] address
         * Seven-bit I2C destination address.
         *
         * @param[in] reg
         * Eight-bit destination register address.
         *
         * @param[in] data
         * Non-empty payload passed to the configured backend.
         *
         * @return
         * `true` when the callback accepts the complete write; otherwise `false`.
         *
         * @pre
         * The backend context remains valid when a safe-write callback is configured.
         *
         * @post
         * Invalid input and a missing safe-write callback are reported as `false`.
         */
        boolean tryWriteRegister(uint8 address, uint8 reg, const bytevector& data) noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_I2C_H */
