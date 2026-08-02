/******************************************************************************
 * @file        xHal_Rpi5CarPwmTestI2c.cpp
 * @brief       Implements the in-memory PWM I2C test double.
 *
 * @details
 * Records interactions from an externally created callback interface and
 * provides indexed access to simulated probe and register-write traffic.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Test
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

#include "xHal_Rpi5CarPwmTestI2c.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-side verification components for the xWalk HAL.
 */
namespace xwalk::hal::test
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Records a probe and reports configured address presence.
 *
 * @param[in,out] context
 * Non-null pointer to the owning `XWalkPwmTestI2c` object.
 *
 * @param[in] address
 * Seven-bit I2C address appended to the probe record.
 *
 * @return
 * `true` when `address` is present in the configured address set; otherwise
 * `false`.
 *
 * @pre
 * `context` points to a live `XWalkPwmTestI2c` instance.
 */
boolean XWalkPwmTestI2c::probeCallback(contextpointer context, uint8 address)
{
    XWalkPwmTestI2c& self = *static_cast<XWalkPwmTestI2c*>(context);
    self.probedAddresses.push_back(address);
    return self.presentAddresses.count(address) != 0U;
}

/**
 * @brief Records one simulated register write.
 *
 * @param[in,out] context
 * Non-null pointer to the owning `XWalkPwmTestI2c` object.
 *
 * @param[in] address
 * Seven-bit destination address appended to the write record.
 *
 * @param[in] reg
 * Eight-bit register address appended to the write record.
 *
 * @param[in] data
 * Payload bytes copied into test-owned storage.
 *
 * @pre
 * `context` points to a live `XWalkPwmTestI2c` instance.
 *
 * @post
 * The address, register, and payload record vectors have equal lengths.
 */
void XWalkPwmTestI2c::writeRegisterCallback(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    XWalkPwmTestI2c& self = *static_cast<XWalkPwmTestI2c*>(context);
    self.writeAddresses.push_back(address);
    self.writeRegisters.push_back(reg);
    self.writePayloads.push_back(data);
}

/**
 * @brief Returns zero-filled bytes for interfaces that require a read callback.
 *
 * @param[in] context
 * Non-owning callback context; unused by this implementation.
 *
 * @param[in] address
 * Seven-bit source address; unused by this implementation.
 *
 * @param[in] length
 * Number of zero-filled bytes to return.
 *
 * @return
 * A byte vector of size `length` containing zero values.
 */
bytevector XWalkPwmTestI2c::readCallback(contextpointer context, uint8 address, size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return bytevector(length, 0U);
}

/**
 * @brief Configures an address to answer subsequent probes.
 *
 * @param[in] address
 * Seven-bit I2C address to mark present.
 *
 * @post
 * Future probe callbacks return `true` for `address`.
 */
void XWalkPwmTestI2c::addPresentAddress(uint8 address)
{
    presentAddresses.insert(address);
}

/**
 * @brief Returns the complete ordered address-probe record.
 *
 * @return
 * Read-only reference valid while this test double remains alive or until
 * subsequent vector mutation invalidates references.
 */
const bytevector& XWalkPwmTestI2c::probes() const noexcept
{
    return probedAddresses;
}

/**
 * @brief Returns the number of recorded register writes.
 *
 * @return
 * Number of write records accumulated since construction or `clearWrites`.
 */
size XWalkPwmTestI2c::writeCount() const noexcept
{
    return writeRegisters.size();
}

/**
 * @brief Returns the destination address for one recorded write.
 *
 * @param[in] index
 * Zero-based index in the range 0 to `writeCount() - 1`.
 *
 * @return
 * Seven-bit destination address stored at `index`.
 *
 * @throws std::out_of_range
 * If `index` does not identify a recorded write.
 */
uint8 XWalkPwmTestI2c::writeAddress(size index) const
{
    return writeAddresses.at(index);
}

/**
 * @brief Returns the register address for one recorded write.
 *
 * @param[in] index
 * Zero-based index in the range 0 to `writeCount() - 1`.
 *
 * @return
 * Eight-bit register address stored at `index`.
 *
 * @throws std::out_of_range
 * If `index` does not identify a recorded write.
 */
uint8 XWalkPwmTestI2c::writeRegister(size index) const
{
    return writeRegisters.at(index);
}

/**
 * @brief Returns the payload for one recorded write.
 *
 * @param[in] index
 * Zero-based index in the range 0 to `writeCount() - 1`.
 *
 * @return
 * Read-only reference to the payload stored at `index`.
 *
 * @throws std::out_of_range
 * If `index` does not identify a recorded write.
 */
const bytevector& XWalkPwmTestI2c::writeData(size index) const
{
    return writePayloads.at(index);
}

/**
 * @brief Clears all recorded register writes.
 *
 * @post
 * `writeCount()` is zero. Probe history and configured present addresses are
 * unchanged.
 */
void XWalkPwmTestI2c::clearWrites() noexcept
{
    writeAddresses.clear();
    writeRegisters.clear();
    writePayloads.clear();
}
} /* namespace xwalk::hal::test */
