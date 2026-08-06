/******************************************************************************
 * @file        xHal_Rpi5CarAdxl345Test.cpp
 * @brief       Verifies the ADXL345 port using an in-memory I2C backend.
 *
 * @details
 * Checks configuration writes, discarded samples, signed conversion, axis
 * ordering, address forwarding, and validation without physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdxl345 Host Test
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

#include "xHal_Rpi5CarAdxl345.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records simulated ADXL345 traffic and supplies register responses. */
struct TestBus
{
    /** @brief Register-read responses returned in request order. */
    XWalkHal::bytevectorvector responses;
    /** @brief Index of the next response to return. */
    XWalkHal::size responseIndex{};
    /** @brief Number of data-format register writes. */
    XWalkHal::uint32 formatWriteCount{};
    /** @brief Number of power-control register writes. */
    XWalkHal::uint32 powerWriteCount{};
    /** @brief Number of register-read transactions. */
    XWalkHal::uint32 registerReadCount{};
    /** @brief Address used by the most recent transaction. */
    XWalkHal::uint8 lastAddress{};
    /** @brief Register used by the most recent transaction. */
    XWalkHal::uint8 lastRegister{};
    /** @brief Length requested by the most recent register read. */
    XWalkHal::size lastLength{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Reports that the simulated ADXL345 is present.
 *
 * @param[in,out] context
 * Non-null test-bus context.
 *
 * @param[in] address
 * Seven-bit address being probed.
 *
 * @return
 * `true` only for the default ADXL345 address.
 */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.lastAddress = address;
    return address == XHAL_RPI5CAR_ADXL345_ADDRESS;
}

/**
 * @brief Records one ADXL345 configuration write.
 *
 * @param[in,out] context
 * Non-null test-bus context.
 *
 * @param[in] address
 * Seven-bit destination address.
 *
 * @param[in] reg
 * Eight-bit configuration register.
 *
 * @param[in] data
 * One-byte configuration payload.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.lastAddress = address;
    bus.lastRegister = reg;
    const hal::boolean dataFormatWrite =
        static_cast<hal::boolean>(
            (reg == XHAL_RPI5CAR_ADXL345_DATA_FORMAT_REGISTER) &&
        (data == XWalkHal::bytevector({XHAL_RPI5CAR_ADXL345_DATA_FORMAT_VALUE})));
    if (dataFormatWrite)
    {
        ++bus.formatWriteCount;
    }
    const hal::boolean powerControlWrite =
        static_cast<hal::boolean>(
            (reg == XHAL_RPI5CAR_ADXL345_POWER_CONTROL_REGISTER) &&
        (data == XWalkHal::bytevector({XHAL_RPI5CAR_ADXL345_MEASUREMENT_MODE_VALUE})));
    if (powerControlWrite)
    {
        ++bus.powerWriteCount;
    }
}

/**
 * @brief Supplies an unused sequential-read response.
 *
 * @param[in] context
 * Opaque test context, unused by this callback.
 *
 * @param[in] address
 * Seven-bit source address, unused by this callback.
 *
 * @param[in] length
 * Requested byte count, unused by this callback.
 *
 * @return
 * One zero byte.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(length);
    return {0U};
}

/**
 * @brief Returns the next configured register-read response.
 *
 * @param[in,out] context
 * Non-null test-bus context.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] reg
 * First data register requested.
 *
 * @param[in] length
 * Requested byte count, expected to be two.
 *
 * @return
 * Next configured response, or an empty response when none remains.
 */
XWalkHal::bytevector readRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, XWalkHal::size length)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.lastAddress = address;
    bus.lastRegister = reg;
    bus.lastLength = length;
    ++bus.registerReadCount;
    const hal::boolean responseUnavailable =
        static_cast<hal::boolean>(
            bus.responseIndex >= bus.responses.size());
    if (responseUnavailable)
    {
        return {};
    }
    const XWalkHal::bytevector response = bus.responses[bus.responseIndex];
    ++bus.responseIndex;
    return response;
}

/**
 * @brief Verifies single-axis configuration, discarded reads, and signed conversion.
 */
void testSingleAxisRead()
{
    TestBus bus;
    bus.responses = {{0x00U, 0x00U}, {0x00U, 0x01U}};
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
    XWalkHal::XWalkAdxl345 accelerometer(i2c);

    const XWalkHal::float64 positiveValue = accelerometer.read(XWalkHal::XWalkAdxl345Axis::X);
    assert(positiveValue == 1.0);
    assert(accelerometer.address() == XHAL_RPI5CAR_ADXL345_ADDRESS);
    assert(bus.formatWriteCount == 1U);
    assert(bus.powerWriteCount == 1U);
    assert(bus.registerReadCount == 2U);
    assert(bus.lastRegister == XHAL_RPI5CAR_ADXL345_DATA_X_REGISTER);
    assert(bus.lastLength == XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH);

    bus.responses = {{0x00U, 0x00U}, {0x00U, 0xFFU}};
    bus.responseIndex = 0U;
    const XWalkHal::float64 negativeValue = accelerometer.read(XWalkHal::XWalkAdxl345Axis::Y);
    assert(negativeValue == -1.0);
    assert(bus.lastRegister == XHAL_RPI5CAR_ADXL345_DATA_Y_REGISTER);
}

/**
 * @brief Verifies X-, Y-, and Z-axis result ordering and scaling.
 */
void testAllAxesRead()
{
    TestBus bus;
    bus.responses = {
        {0U, 0U}, {0U, 0xFFU},
        {0U, 0U}, {0x80U, 0U},
        {0U, 0U}, {0U, 0x02U}
    };
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
    XWalkHal::XWalkAdxl345 accelerometer(i2c);

    const XWalkHal::adxl345values values = accelerometer.read();
    assert(values == XWalkHal::adxl345values({-1.0, 0.5, 2.0}));
    assert(bus.formatWriteCount == 3U);
    assert(bus.powerWriteCount == 3U);
    assert(bus.registerReadCount == 6U);
    assert(bus.lastRegister == XHAL_RPI5CAR_ADXL345_DATA_Z_REGISTER);
}

/**
 * @brief Verifies axis, address, backend-capability, and response validation.
 */
void testValidation()
{
    TestBus bus;
    bus.responses = {{0U}, {0U, 0U}};
    XWalkHal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
    XWalkHal::XWalkAdxl345 accelerometer(i2c);

    xwalk::hal::test::expectFailure([&]()
    {
        const XWalkHal::XWalkAdxl345Axis invalidAxis =
            static_cast<XWalkHal::XWalkAdxl345Axis>(XHAL_RPI5CAR_ADXL345_AXIS_COUNT);
        static_cast<void>(accelerometer.read(invalidAxis));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(accelerometer.read(XWalkHal::XWalkAdxl345Axis::Z));
    });

    XWalkHal::XWalkI2c sequentialOnlyI2c(&bus, &probe, &writeRegister, &read);
    XWalkHal::XWalkAdxl345 sequentialOnlyAccelerometer(sequentialOnlyI2c);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(sequentialOnlyAccelerometer.read(XWalkHal::XWalkAdxl345Axis::X));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkAdxl345 invalidAddressAccelerometer(i2c, 0x80U);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every xWalk ADXL345 host-test scenario.
 *
 * @return
 * Zero when every assertion passes.
 */
XWalkHal::int32 main()
{
    testSingleAxisRead();
    testAllAxesRead();
    testValidation();
    return 0;
}
