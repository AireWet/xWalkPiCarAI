/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfoTest.cpp
 * @brief       Verifies firmware-information behavior with an in-memory bus.
 *
 * @details
 * Checks address ordering, register acquisition, typed conversion, formatting,
 * library metadata, missing devices, and invalid response lengths.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarFirmwareInfo.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Supplies deterministic probe and firmware-register responses. */
struct TestBus
{
    byteset respondingAddresses{}; /**< Addresses reported as present. */
    bytevector probes{}; /**< Ordered addresses passed to the probe callback. */
    bytevector versionBytes{2U, 5U, 5U}; /**< Next firmware-register response. */
    uint8 readAddress{}; /**< Most recent register-read address. */
    uint8 readRegister{}; /**< Most recent register-read register. */
    size readLength{}; /**< Most recent requested register-read byte count. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records a probe and reports configured presence.
 *
 * @param[in,out] context
 * Non-null pointer to test-bus state.
 *
 * @param[in] address
 * Seven-bit address to record.
 *
 * @return
 * `true` when `address` is configured as responding; otherwise `false`.
 */
boolean probe(contextpointer context, uint8 address)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.probes.push_back(address);
    return bus.respondingAddresses.count(address) != 0U;
}

/**
 * @brief Accepts an unused register-write operation.
 *
 * @param[in,out] context
 * Non-owning test-bus context.
 *
 * @param[in] address
 * Seven-bit destination address.
 *
 * @param[in] reg
 * Eight-bit destination register.
 *
 * @param[in] data
 * Payload bytes supplied by the caller.
 */
void writeRegister(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
}

/**
 * @brief Provides an unused sequential-read callback.
 *
 * @param[in,out] context
 * Non-owning test-bus context.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] length
 * Requested byte count.
 *
 * @return
 * Empty payload because firmware acquisition uses atomic register reads.
 */
bytevector read(contextpointer context, uint8 address, size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    static_cast<void>(length);
    return {};
}

/**
 * @brief Records and services one firmware register read.
 *
 * @param[in,out] context
 * Non-null pointer to test-bus state.
 *
 * @param[in] address
 * Seven-bit source address.
 *
 * @param[in] reg
 * First register requested by the caller.
 *
 * @param[in] length
 * Requested byte count.
 *
 * @return
 * Configured firmware-version bytes.
 */
bytevector readRegister(contextpointer context, uint8 address, uint8 reg, size length)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    bus.readAddress = address;
    bus.readRegister = reg;
    bus.readLength = length;
    return bus.versionBytes;
}

/**
 * @brief Verifies first-address selection, acquisition, and formatted metadata.
 */
void testReadAndFormat()
{
    TestBus bus;
    bus.respondingAddresses.insert(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1);
    XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
    XWalkFirmwareInfo information(i2c);

    assert(information.address() == XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1);
    assert(bus.probes == bytevector({XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1}));
    const XWalkFirmwareVersion version = information.read();
    assert(version.major == 2U);
    assert(version.minor == 5U);
    assert(version.patch == 5U);
    assert(bus.readAddress == XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1);
    assert(bus.readRegister == XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_REGISTER);
    assert(bus.readLength == XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_BYTE_COUNT);
    assert(information.readText() == "2.5.5");
    assert(XWalkFirmwareInfo::libraryVersion() == "2.5.5");
}

/** @brief Verifies fallback to the second supported Robot HAT address. */
void testSecondAddressSelection()
{
    TestBus bus;
    bus.respondingAddresses.insert(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2);
    XWalkI2c i2c(&bus, &probe, &writeRegister, &read, &readRegister);
    XWalkFirmwareInfo information(i2c);

    assert(information.address() == XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2);
    assert(bus.probes == bytevector({XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1,
        XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2}));
}

/** @brief Verifies rejection of missing devices and incomplete responses. */
void testFailures()
{
    TestBus missingBus;
    XWalkI2c missingI2c(&missingBus, &probe, &writeRegister, &read, &readRegister);
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkFirmwareInfo information(missingI2c);
        static_cast<void>(information);
    });

    TestBus shortBus;
    shortBus.respondingAddresses.insert(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1);
    shortBus.versionBytes = {2U, 5U};
    XWalkI2c shortI2c(&shortBus, &probe, &writeRegister, &read, &readRegister);
    XWalkFirmwareInfo information(shortI2c);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(information.read());
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side firmware-information test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int32 main()
{
    testReadAndFormat();
    testSecondAddressSelection();
    testFailures();
    return 0;
}
