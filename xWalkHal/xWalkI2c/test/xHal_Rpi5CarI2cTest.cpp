/******************************************************************************
 * @file        xHal_Rpi5CarI2cTest.cpp
 * @brief       Tests the hardware-independent I2C callback interface.
 *
 * @details
 * Uses an in-memory recording context to verify address validation, probe
 * forwarding, and register-write forwarding without physical I2C access.
 *
 * @project     xWalk Firmware
 * @module      xWalkI2c Host Test
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

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test data and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records the most recent operation received by the test callbacks. */
struct RecordingData
{
    /** @brief Most recently probed or written seven-bit I2C address. */
    XWalkHal::uint8 lastAddress{};
    /** @brief Most recently written eight-bit register address. */
    XWalkHal::uint8 lastRegister{};
    /** @brief Copy of the most recently written byte payload. */
    XWalkHal::bytevector lastData;
    /** @brief Most recent requested read length in bytes. */
    XWalkHal::size lastReadLength{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records an address probe and simulates one present device.
 *
 * @param[in,out] context
 * Non-null pointer to the `RecordingData` instance updated by the callback.
 *
 * @param[in] address
 * Seven-bit I2C address to record and evaluate.
 *
 * @return
 * `true` only for address `0x14`; otherwise `false`.
 */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    RecordingData& data = *static_cast<RecordingData*>(context);
    data.lastAddress = address;
    return address == 0x14U;
}

/**
 * @brief Records one simulated I2C register write.
 *
 * @param[in,out] context
 * Non-null pointer to the `RecordingData` instance updated by the callback.
 *
 * @param[in] address
 * Seven-bit I2C destination address.
 *
 * @param[in] reg
 * Eight-bit destination register address.
 *
 * @param[in] bytes
 * Payload bytes copied into the recording context.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::uint8 reg,
    const XWalkHal::bytevector& bytes)
{
    RecordingData& data = *static_cast<RecordingData*>(context);
    data.lastAddress = address;
    data.lastRegister = reg;
    data.lastData = bytes;
}

/**
 * @brief Records a read request and returns deterministic test bytes.
 *
 * @param[in,out] context
 * Non-null pointer to the `RecordingData` instance updated by the callback.
 *
 * @param[in] address
 * Seven-bit I2C source address.
 *
 * @param[in] length
 * Number of bytes requested.
 *
 * @return
 * The bytes `0xAB` and `0xCD` for the two-byte test request.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    RecordingData& data = *static_cast<RecordingData*>(context);
    data.lastAddress = address;
    data.lastReadLength = length;
    return {0xABU, 0xCDU};
}

/**
 * @brief Records a register-read request and returns deterministic test bytes.
 *
 * @param[in,out] context
 * Non-null pointer to the `RecordingData` instance updated by the callback.
 *
 * @param[in] address
 * Seven-bit I2C source address.
 *
 * @param[in] reg
 * First eight-bit register address requested.
 *
 * @param[in] length
 * Number of bytes requested.
 *
 * @return
 * The bytes `0x34` and `0x12` for the two-byte test request.
 */
XWalkHal::bytevector readRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, XWalkHal::size length)
{
    RecordingData& data = *static_cast<RecordingData*>(context);
    data.lastAddress = address;
    data.lastRegister = reg;
    data.lastReadLength = length;
    return {0x34U, 0x12U};
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the callback-forwarding host test.
 *
 * @return
 * Zero when all assertions pass. A failed assertion terminates the process.
 */
XWalkHal::int32 main()
{
    RecordingData data;
    xwalk::hal::XWalkI2c i2c(&data, &probe, &writeRegister, &read, &readRegister);

    const XWalkHal::boolean probeResult = i2c.probe(0x14U);
    assert(probeResult);

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(i2c.probe(0x80U));
    });

    const XWalkHal::bytevector bytes{0x12U, 0x34U};
    i2c.writeRegister(0x14U, 0x20U, bytes);
    const XWalkHal::bytevector expectedBytes{0x12U, 0x34U};
    assert(data.lastAddress == 0x14U);
    assert(data.lastRegister == 0x20U);
    assert(data.lastData == expectedBytes);

    const XWalkHal::bytevector readBytes = i2c.read(0x15U, 2U);
    assert(data.lastAddress == 0x15U);
    assert(data.lastReadLength == 2U);
    assert(readBytes == XWalkHal::bytevector({0xABU, 0xCDU}));

    const XWalkHal::bytevector registerBytes = i2c.readRegister(0x53U, 0x32U, 2U);
    assert(data.lastAddress == 0x53U);
    assert(data.lastRegister == 0x32U);
    assert(data.lastReadLength == 2U);
    assert(registerBytes == XWalkHal::bytevector({0x34U, 0x12U}));

    xwalk::hal::XWalkI2c sequentialOnlyI2c(&data, &probe, &writeRegister, &read);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(sequentialOnlyI2c.readRegister(0x53U, 0x32U, 2U));
    });

    return 0;
}
