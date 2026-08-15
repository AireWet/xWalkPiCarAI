/******************************************************************************
 * @file        xHal_Rpi5CarServoTestI2c.cpp
 * @brief       Implements the Servo host-test I2C recorder.
 *
 * @details
 * Records register-write callbacks and exposes bounded indexed access for test
 * assertions.
 *
 * @project     xWalk Firmware
 * @module      xWalkServo Host Test
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

#include "xHal_Rpi5CarServoTestI2c.h"

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
     * @brief Simulates an I2C probe with no responding device.
     *
     * @param[in] context
     * Non-owning callback context; unused by this implementation.
     *
     * @param[in] address
     * Seven-bit I2C address; unused by this implementation.
     *
     * @return
     * Always `false`.
     */
    boolean XWalkServoTestI2c::probeCallback(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return false;
    }

    /**
     * @brief Records one simulated register write.
     *
     * @param[in,out] context
     * Non-null pointer to the `XWalkServoTestI2c` recorder.
     *
     * @param[in] address
     * Seven-bit destination address; accepted but not stored.
     *
     * @param[in] reg
     * Eight-bit destination register address.
     *
     * @param[in] data
     * Payload bytes copied into recorder-owned storage.
     *
     * @pre
     * `context` points to a live `XWalkServoTestI2c` instance.
     *
     * @post
     * The register and payload vectors have equal lengths.
     */
    void
    XWalkServoTestI2c::writeRegisterCallback(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(address);
        XWalkServoTestI2c& self = *static_cast<XWalkServoTestI2c*>(context);
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
    bytevector XWalkServoTestI2c::readCallback(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }

    /**
     * @brief Returns the number of recorded register writes.
     *
     * @return
     * Number of writes recorded since construction or the most recent clear.
     */
    size XWalkServoTestI2c::writeCount() const noexcept
    {
        return writeRegisters.size();
    }

    /**
     * @brief Clears every recorded register and payload.
     *
     * @post
     * `writeCount()` is zero.
     */
    void XWalkServoTestI2c::clearWrites() noexcept
    {
        writeRegisters.clear();
        writePayloads.clear();
    }

    /**
     * @brief Returns the register address for one recorded write.
     *
     * @param[in] index
     * Zero-based write index in the range 0 to `writeCount() - 1`.
     *
     * @return
     * Eight-bit register address stored at `index`.
     *
     * @throws std::out_of_range
     * If `index` does not identify a recorded write.
     */
    uint8 XWalkServoTestI2c::writeRegister(size index) const
    {
        return writeRegisters.at(index);
    }

    /**
     * @brief Returns the payload for one recorded write.
     *
     * @param[in] index
     * Zero-based write index in the range 0 to `writeCount() - 1`.
     *
     * @return
     * Read-only reference to the payload stored at `index`.
     *
     * @throws std::out_of_range
     * If `index` does not identify a recorded write.
     */
    const bytevector& XWalkServoTestI2c::writeData(size index) const
    {
        return writePayloads.at(index);
    }

} /* namespace xwalk::hal::test */
