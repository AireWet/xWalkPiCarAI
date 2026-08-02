/******************************************************************************
 * @file        xHal_Rpi5CarAdcTest.cpp
 * @brief       Verifies the xWalk ADC port with an in-memory I2C backend.
 *
 * @details
 * Checks address selection, channel mapping, bus traffic, sample assembly, voltage scaling, and validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkAdc Host Test
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

#include "xHal_Rpi5CarAdc.h"

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

/** @brief Records observable I2C traffic and supplies deterministic ADC bytes. */
struct TestBus
{
    /** @brief Addresses that respond to probes. */
    XWalkHal::byteset presentAddresses;
    /** @brief Ordered addresses passed to the probe callback. */
    XWalkHal::bytevector probes;
    /** @brief Bytes returned by the next read callback. */
    XWalkHal::bytevector readBytes{0x0AU, 0xBCU};
    /** @brief Most recently written address. */
    XWalkHal::uint8 writeAddress{};
    /** @brief Most recently written register or command. */
    XWalkHal::uint8 writeRegister{};
    /** @brief Most recently written payload. */
    XWalkHal::bytevector writeData;
    /** @brief Most recently read address. */
    XWalkHal::uint8 readAddress{};
    /** @brief Most recently requested read length in bytes. */
    XWalkHal::size readLength{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records a probe and reports configured device presence.
 *
 * @param[in,out] context
 * Non-null pointer to the test bus.
 *
 * @param[in] address
 * Seven-bit address to record.
 *
 * @return
 * `true` when the address is configured as present; otherwise `false`.
 */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.probes.push_back(address);
    return bus.presentAddresses.count(address) != 0U;
}

/**
 * @brief Records an ADC command write.
 *
 * @param[in,out] context
 * Non-null pointer to the test bus.
 *
 * @param[in] address
 * Seven-bit destination address.
 *
 * @param[in] reg
 * ADC command byte carried as the register value.
 *
 * @param[in] data
 * Command payload copied into test-owned storage.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address, XWalkHal::uint8 reg,
    const XWalkHal::bytevector& data)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.writeAddress = address;
    bus.writeRegister = reg;
    bus.writeData = data;
}

/**
 * @brief Records a read request and returns configured sample bytes.
 *
 * @param[in,out] context
 * Non-null pointer to the test bus.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] length
 * Requested sample length in bytes.
 *
 * @return
 * Test-owned configured sample bytes copied by value.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.readAddress = address;
    bus.readLength = length;
    return bus.readBytes;
}

/**
 * @brief Verifies numeric channel mapping and raw sample acquisition.
 */
void testRead()
{
    TestBus bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    xwalk::hal::XWalkAdc adc(i2c, 0U, XHAL_RPI5CAR_ADC_ADDRESS_1);

    const XWalkHal::uint16 sample = adc.read();
    assert(sample == 0x0ABCU);
    assert(adc.channel() == 0U);
    assert(adc.command() == 0x17U);
    assert(bus.writeAddress == XHAL_RPI5CAR_ADC_ADDRESS_1);
    assert(bus.writeRegister == 0x17U);
    assert(bus.writeData == XWalkHal::bytevector({0U, 0U}));
    assert(bus.readAddress == XHAL_RPI5CAR_ADC_ADDRESS_1);
    assert(bus.readLength == 2U);
}

/**
 * @brief Verifies named channels, automatic address selection, and full-scale voltage conversion.
 */
void testSelectionAndVoltage()
{
    TestBus bus;
    bus.presentAddresses.insert(XHAL_RPI5CAR_ADC_ADDRESS_2);
    bus.readBytes = {0x0FU, 0xFFU};
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    xwalk::hal::XWalkAdc adc(i2c, "A7");

    assert(adc.address() == XHAL_RPI5CAR_ADC_ADDRESS_2);
    assert(adc.command() == 0x10U);
    assert(bus.probes == XWalkHal::bytevector({0x14U, 0x15U}));
    const XWalkHal::float64 voltageDifference = XHAL_ABSOLUTE_VALUE(adc.readVoltage() - 3.3);
    assert(voltageDifference < 0.000001);
}

/**
 * @brief Verifies rejection of invalid channels and incomplete reads.
 */
void testValidation()
{
    TestBus bus;
    xwalk::hal::XWalkI2c i2c(&bus, &probe, &writeRegister, &read);
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::XWalkAdc adc(i2c, 8U);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::XWalkAdc adc(i2c, "A8");
    });

    bus.readBytes = {0x01U};
    xwalk::hal::XWalkAdc adc(i2c, 1U, XHAL_RPI5CAR_ADC_ADDRESS_1);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(adc.read());
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all xWalk ADC host-test scenarios.
 *
 * @return
 * Zero when every assertion passes. A failed assertion terminates the process.
 */
XWalkHal::int32 main()
{
    testRead();
    testSelectionAndVoltage();
    testValidation();
    return 0;
}
