/******************************************************************************
 * @file        xHal_Rpi5CarSpiTest.cpp
 * @brief       Verifies bounded hardware-independent SPI transactions.
 *
 * @details
 * Uses an in-memory full-duplex callback to cover payload forwarding, received
 * data, null callback validation, payload bounds, and response-length checks.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi Host Test
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xHal_Rpi5CarSpi.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and callbacks private to this translation unit. */
namespace
{

/** @brief Records one request and supplies one configured response. */
struct TestBackend
{
    XWalkHal::bytevector request;
    XWalkHal::bytevector response;
};

/** @brief Records and completes one simulated full-duplex transfer. */
XWalkHal::bytevector transfer(XWalkHal::contextpointer context,
    const XWalkHal::bytevector& transmitData)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    backend.request = transmitData;
    return backend.response;
}

/** @brief Verifies forwarding, response delivery, and validation failures. */
void testTransfers()
{
    TestBackend backend{{}, {0xDEU, 0xADU, 0xBEU, 0xEFU}};
    XWalkHal::XWalkSpi spi(&backend, &transfer);
    const XWalkHal::bytevector request{0x9FU, 0x00U, 0x00U, 0x00U};
    assert(spi.transfer(request) == backend.response);
    assert(backend.request == request);

    xwalk::hal::test::expectFailure([&spi]()
    {
        static_cast<void>(spi.transfer({}));
    });
    xwalk::hal::test::expectFailure([&spi]()
    {
        const XWalkHal::bytevector oversized(
            XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES + 1U, 0U);
        static_cast<void>(spi.transfer(oversized));
    });
    backend.response = {0x00U};
    xwalk::hal::test::expectFailure([&spi, &request]()
    {
        static_cast<void>(spi.transfer(request));
    });
    xwalk::hal::test::expectFailure([]()
    {
        XWalkHal::XWalkSpi invalid(nullptr, nullptr);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs every host-side SPI scenario. */
XWalkHal::int32 main()
{
    testTransfers();
    return 0;
}
