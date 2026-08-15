/******************************************************************************
 * @file        xTreasureHuntSequenceTest.cpp
 * @brief       Verifies the example-20 CLI sequence through simulated HAL.
 *
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 *
 * @author      Joxy John
 * @date        2026-08-05
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

#include "xControllerCommandTestSupport.h"
#include "xControllerSequence.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the device-free treasure-hunt scenario. */
namespace
{

    /**
     * @brief Verifies target selection, success, movement, repeat, quit, and cleanup.
     * @param[in,out] context Complete in-memory Controller-to-HAL composition.
     */
    void testTreasureHunt(xwalk::agent::test::ControllerCommandTestContext& context)
    {
        context.state->inputLines = {"w", "space", "quit"};
        context.state->visionColorWidths = {120U, 40U, 40U};
        context.state->treasureColorNames = {"red", "blue"};
        xwalk::agent::test::XWalkControllerSequence sequence(*context.treasureHuntController);
        assert(sequence.run({{"treasure-hunt"}}) == 0);
        assert(context.state->visionStarted == false);
        assert(context.state->treasureColorIndex == 2U);
        assert(context.state->visionObservationCount == 3U);
        assert(context.state->inputIndex == 3U);
        assert(context.state->spokenText ==
               ctrl::stringvector(
                   {"Game start!", "Look for red!", "Well done!", "Look for blue!", "Look for blue!", "Goodbye!"}));

        assert(xwalk::agent::test::containsOrderedEvents(
            context.state->eventLog,
            {"vision.start",     "controller.delay", "hal.speech.speak", "vision.color",     "hal.speech.speak",
             "controller.input", "vision.observe",   "hal.speech.speak", "vision.color",     "hal.speech.speak",
             "hal.i2c.write",    "controller.delay", "hal.i2c.write",    "controller.input", "vision.observe",
             "hal.speech.speak", "controller.input", "vision.observe",   "vision.stop",      "hal.speech.speak"}));
    }

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the treasure-hunt controller-to-HAL host sequence.
 * @param[in] argc Process argument count.
 * @param[in,out] argv Process argument vector.
 * @return Zero after all assertions pass; one for invalid runner arguments.
 */
int xWalkTreasureHuntCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(argc, argv, &testTreasureHunt);
}
