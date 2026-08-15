/******************************************************************************
 * @file        xSpiSequenceTest.cpp
 * @brief       Verifies one public CLI command sequence through simulated HAL.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

namespace
{
    void testSpi(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        xwalk::agent::test::XWalkControllerSequence sequence(*context.spiController);
        assert(sequence.run({{"spi", "transfer", "0x9f00a5"}}) == 0);

        assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog, {"hal.spi.transfer"}));
    }
} // namespace

/** @brief Runs the SPI controller-to-HAL host sequence. @return Zero on success. */
int xWalkSpiCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testSpi);
}
