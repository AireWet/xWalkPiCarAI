/******************************************************************************
 * @file        xAgent_Rpi5CarSpiTransferTest.cpp
 * @brief       Verifies device-free SPI Agent coordination.
 *
 * @details
 * Uses an in-memory HAL callback to confirm exact request and response
 * forwarding without opening a Linux SPI device.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpiTransfer Host Test
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

#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xAgent_Rpi5CarSpiTransferTestTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using TestBackend = ::xwalk::source_types::xagent_rpi5carspitransfertest::TestBackend;

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state private to this translation unit. */
namespace
{

    /** @brief Returns each transmitted byte with every bit inverted. */
    agent::bytevector transfer(agent::contextpointer context, const agent::bytevector& transmitData)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        backend.request = transmitData;
        agent::bytevector response;
        response.reserve(transmitData.size());
        for (const agent::uint8 value : transmitData)
        {
            response.push_back(static_cast<agent::uint8>(value ^ 0xFFU));
        }
        return response;
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs the deterministic SPI Agent scenario. */
agent::int32 main()
{
    TestBackend backend;
    XWalkHal::XWalkSpi spi(&backend, &transfer);
    xwalk::agent::XWalkSpiTransfer agent(spi);
    const agent::bytevector request{0x00U, 0x55U, 0xAAU, 0xFFU};
    const agent::bytevector expected{0xFFU, 0xAAU, 0x55U, 0x00U};
    assert(agent.transfer(request) == expected);
    assert(backend.request == request);
    return 0;
}
