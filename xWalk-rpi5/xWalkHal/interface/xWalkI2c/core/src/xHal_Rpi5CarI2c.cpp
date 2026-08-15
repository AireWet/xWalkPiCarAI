/******************************************************************************
 * @file        xHal_Rpi5CarI2c.cpp
 * @brief       Implements hardware-independent I2C callback forwarding.
 *
 * @details
 * Forwards device probes and register writes to the callbacks validated during
 * construction of the `XWalkI2c` object. Emits filtered transaction traces and
 * unfiltered validation diagnostics around the callback boundary.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarI2c.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Probes a device through the configured backend callback.
     *
     * @param[in] address
     * Seven-bit I2C address to probe.
     *
     * @return
     * `true` when the backend reports a responding device; otherwise `false`.
     *
     * @pre
     * The non-owning backend context supplied during construction remains valid.
     *
     * @throws std::out_of_range
     * If `address` exceeds the seven-bit protocol range.
     */
    boolean XWalkI2c::probe(uint8 address)
    {
        const boolean addressInvalid = address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS;
        if (addressInvalid)
        {
            XWALK_HAL_ERROR(
                XWALK_EXCEPTION, "I2C probe address exceeds the seven-bit range: %u", static_cast<uint32>(address));
            XWALK_HAL_ASSERT(1201);
        }
        common::validateI2cAddress(address);
        const mutexlock transactionLock(transactionMutexValue);
        XWALK_HAL_TRACE_UID1(RPI .001, "I2C probe requested for address %u", static_cast<uint32>(address));
        const boolean deviceAvailable = probeCallback(contextValue, address);
        XWALK_HAL_TRACE_UID2(RPI .002,
                             "I2C probe completed for address %u with status %u",
                             static_cast<uint32>(address),
                             static_cast<uint32>(deviceAvailable));
        return deviceAvailable;
    }

    /**
     * @brief Reads bytes through the configured backend callback.
     *
     * @param[in] address
     * Seven-bit I2C source address.
     *
     * @param[in] length
     * Non-zero number of bytes requested.
     *
     * @return
     * Bytes returned by the backend in bus order.
     *
     * @throws std::invalid_argument
     * If `length` is zero.
     *
     * @throws std::out_of_range
     * If `address` exceeds the seven-bit protocol range.
     */
    bytevector XWalkI2c::read(uint8 address, size length)
    {
        const boolean addressInvalid = address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS;
        if (addressInvalid)
        {
            XWALK_HAL_ERROR(
                XWALK_EXCEPTION, "I2C read address exceeds the seven-bit range: %u", static_cast<uint32>(address));
            XWALK_HAL_ASSERT(1202);
        }
        common::validateI2cAddress(address);
        if (length == 0U)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "I2C read length must not be zero");
        }
        const mutexlock transactionLock(transactionMutexValue);
        XWALK_HAL_TRACE_UID2(RPI .003,
                             "I2C sequential read requested for address %u and %zu bytes",
                             static_cast<uint32>(address),
                             length);
        bytevector bytes = readCallback(contextValue, address, length);
        XWALK_HAL_TRACE_UID2(RPI .004,
                             "I2C sequential read returned %zu bytes from address %u",
                             bytes.size(),
                             static_cast<uint32>(address));
        return bytes;
    }

    /**
     * @brief Reads consecutive bytes through the configured register-read callback.
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
    bytevector XWalkI2c::readRegister(uint8 address, uint8 reg, size length)
    {
        const boolean addressInvalid = address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS;
        if (addressInvalid)
        {
            XWALK_HAL_ERROR(XWALK_EXCEPTION,
                            "I2C register-read address exceeds the seven-bit range: %u",
                            static_cast<uint32>(address));
            XWALK_HAL_ASSERT(1204);
        }
        common::validateI2cAddress(address);
        if (length == 0U)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "I2C register-read length must not be zero");
        }
        if (readRegisterCallback == nullptr)
        {
            XWALK_HAL_WARNING(XWALK_SYSTEM, "I2C atomic register-read operation is unavailable");
            XWALK_HAL_ERROR(XWALK_RUNTIME, "I2C register-read callback is not configured");
        }
        const mutexlock transactionLock(transactionMutexValue);
        XWALK_HAL_TRACE_UID3(RPI .005,
                             "I2C register read requested for address %u, register %u, and %zu bytes",
                             static_cast<uint32>(address),
                             static_cast<uint32>(reg),
                             length);
        bytevector bytes = readRegisterCallback(contextValue, address, reg, length);
        XWALK_HAL_TRACE_UID3(RPI .006,
                             "I2C register read returned %zu bytes from address %u and register %u",
                             bytes.size(),
                             static_cast<uint32>(address),
                             static_cast<uint32>(reg));
        return bytes;
    }

    /**
     * @brief Writes one command and reads its response under the shared transaction
     * lock.
     *
     * @param[in] address
     * Seven-bit I2C address used for both callbacks.
     *
     * @param[in] reg
     * Eight-bit command register written before reading.
     *
     * @param[in] data
     * Command payload forwarded to the write callback.
     *
     * @param[in] length
     * Non-zero response length in bytes.
     *
     * @return
     * Response bytes returned by the sequential-read callback in bus order.
     *
     * @post
     * No public operation through this object executes between the write and read
     * callbacks.
     *
     * @throws std::invalid_argument
     * If `length` is zero.
     *
     * @throws std::out_of_range
     * If `address` exceeds the seven-bit protocol range.
     */
    bytevector XWalkI2c::writeRegisterThenRead(uint8 address, uint8 reg, const bytevector& data, size length)
    {
        const boolean addressInvalid = address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS;
        if (addressInvalid)
        {
            XWALK_HAL_ERROR(XWALK_EXCEPTION,
                            "I2C combined-transaction address exceeds the seven-bit range: %u",
                            static_cast<uint32>(address));
            XWALK_HAL_ASSERT(1208);
        }
        common::validateI2cAddress(address);
        if (length == 0U)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "I2C combined-transaction read length must not be zero");
        }
        const mutexlock transactionLock(transactionMutexValue);
        XWALK_HAL_TRACE_UID3(RPI .383,
                             "I2C atomic command requested for address %u, register "
                             "%u, and %zu response bytes",
                             static_cast<uint32>(address),
                             static_cast<uint32>(reg),
                             length);
        writeRegisterCallback(contextValue, address, reg, data);
        bytevector bytes = readCallback(contextValue, address, length);
        XWALK_HAL_TRACE_UID2(RPI .384,
                             "I2C atomic command returned %zu bytes from address %u",
                             bytes.size(),
                             static_cast<uint32>(address));
        return bytes;
    }

    /**
     * @brief Writes a byte payload through the configured backend callback.
     *
     * @param[in] address
     * Seven-bit I2C destination address.
     *
     * @param[in] reg
     * Eight-bit destination register address.
     *
     * @param[in] data
     * Payload bytes interpreted and validated by the configured backend.
     *
     * @pre
     * The non-owning backend context supplied during construction remains valid.
     *
     * @post
     * The callback has either completed the requested write or reported failure
     * according to the backend's error policy.
     *
     * @throws std::out_of_range
     * If `address` exceeds the seven-bit protocol range.
     */
    void XWalkI2c::writeRegister(uint8 address, uint8 reg, const bytevector& data)
    {
        const boolean addressInvalid = address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS;
        if (addressInvalid)
        {
            XWALK_HAL_ERROR(XWALK_EXCEPTION,
                            "I2C register-write address exceeds the seven-bit range: %u",
                            static_cast<uint32>(address));
            XWALK_HAL_ASSERT(1207);
        }
        common::validateI2cAddress(address);
        const mutexlock transactionLock(transactionMutexValue);
        XWALK_HAL_TRACE_UID3(RPI .007,
                             "I2C register write requested for address %u, register %u, and %zu bytes",
                             static_cast<uint32>(address),
                             static_cast<uint32>(reg),
                             data.size());
        writeRegisterCallback(contextValue, address, reg, data);
        XWALK_HAL_TRACE_UID2(RPI .008,
                             "I2C register write completed for address %u and register %u",
                             static_cast<uint32>(address),
                             static_cast<uint32>(reg));
    }

    /**
     * @brief Attempts one register write through the non-throwing fail-safe
     * callback.
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
     * `true` when the address, payload, callback, and backend write are valid;
     * otherwise `false`.
     *
     * @pre
     * The backend context remains valid when a safe-write callback is configured.
     *
     * @post
     * Invalid input and a missing safe-write callback are reported as `false`.
     *
     * @note
     * This fail-safe path deliberately avoids tracing so trace initialization or
     * formatting cannot violate its non-throwing contract.
     */
    boolean XWalkI2c::tryWriteRegister(uint8 address, uint8 reg, const bytevector& data) noexcept
    {
        const hal::boolean addressTryWriteRegisterCallbackInvalid = static_cast<hal::boolean>(
            (address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS) || data.empty() || (tryWriteRegisterCallback == nullptr));
        if (addressTryWriteRegisterCallbackInvalid)
        {
            return false;
        }
        const mutexlock transactionLock(transactionMutexValue);
        return tryWriteRegisterCallback(contextValue, address, reg, data);
    }

} /* namespace xwalk::hal */
