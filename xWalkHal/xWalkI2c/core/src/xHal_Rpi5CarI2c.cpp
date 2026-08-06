/******************************************************************************
 * @file        xHal_Rpi5CarI2c.cpp
 * @brief       Implements hardware-independent I2C callback forwarding.
 *
 * @details
 * Forwards device probes and register writes to the callbacks validated during
 * construction of the `XWalkI2c` object.
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
    common::validateI2cAddress(address);
    return probeCallback(contextValue, address);
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
    common::validateI2cAddress(address);
    if (length == 0U)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C read length must not be zero");
    }
    return readCallback(contextValue, address, length);
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
    common::validateI2cAddress(address);
    if (length == 0U)
    {
        XHAL_THROW_INVALID_ARGUMENT("I2C register-read length must not be zero");
    }
    if (readRegisterCallback == nullptr)
    {
        XHAL_THROW_RUNTIME_ERROR("I2C register-read callback is not configured");
    }
    return readRegisterCallback(contextValue, address, reg, length);
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
    common::validateI2cAddress(address);
    writeRegisterCallback(contextValue, address, reg, data);
}

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
 * `true` when the address, payload, callback, and backend write are valid;
 * otherwise `false`.
 *
 * @pre
 * The backend context remains valid when a safe-write callback is configured.
 *
 * @post
 * Invalid input and a missing safe-write callback are reported as `false`.
 */
boolean XWalkI2c::tryWriteRegister(uint8 address, uint8 reg, const bytevector& data) noexcept
{
    const hal::boolean addressTryWriteRegisterCallbackInvalid =
        static_cast<hal::boolean>(
            (address > common::I2C_MAXIMUM_SEVEN_BIT_ADDRESS) || data.empty() ||
        (tryWriteRegisterCallback == nullptr));
    if (addressTryWriteRegisterCallbackInvalid)
    {
        return false;
    }
    return tryWriteRegisterCallback(contextValue, address, reg, data);
}

} /* namespace xwalk::hal */
