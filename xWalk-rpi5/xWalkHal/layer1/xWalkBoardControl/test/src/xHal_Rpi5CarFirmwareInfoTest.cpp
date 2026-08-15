/******************************************************************************
 * @file        xHal_Rpi5CarFirmwareInfoTest.cpp
 * @brief       Verifies firmware information through a named in-memory bus.
 * @project     xWalk Firmware
 * @module      xWalkBoardControl Host Test
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBoardControlTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include <cassert>
namespace
{
    using namespace xwalk::hal;
    using namespace xwalk::hal::test::boardcontrol;
    XWalkI2c createI2c(TestFirmwareBus& bus)
    {
        return XWalkI2c(&bus, &probeFirmware, &writeFirmware, &readFirmware, &readFirmwareRegister);
    }
    void testReadAndFormat()
    {
        TestFirmwareBus bus;
        bus.respondingAddresses.insert(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1);
        XWalkI2c i2c = createI2c(bus);
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
    void testSecondAddressSelection()
    {
        TestFirmwareBus bus;
        bus.respondingAddresses.insert(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2);
        XWalkI2c i2c = createI2c(bus);
        XWalkFirmwareInfo information(i2c);
        assert(information.address() == XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2);
        assert(bus.probes == bytevector({XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1, XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2}));
    }
    void testFailures()
    {
        TestFirmwareBus missingBus;
        XWalkI2c missingI2c = createI2c(missingBus);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkFirmwareInfo information(missingI2c);
                static_cast<void>(information);
            });
        TestFirmwareBus shortBus;
        shortBus.respondingAddresses.insert(XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1);
        shortBus.versionBytes = {2U, 5U};
        XWalkI2c shortI2c = createI2c(shortBus);
        XWalkFirmwareInfo information(shortI2c);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(information.read());
            });
    }
} /* namespace */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_BOARD_CONTROL_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_BOARD_CONTROL_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .337, "xWalkFirmwareInfo host tests started");
    testReadAndFormat();
    testSecondAddressSelection();
    testFailures();
    XWALK_HAL_TRACE_UID0(RPI .338, "xWalkFirmwareInfo host tests completed");
    return 0;
}
