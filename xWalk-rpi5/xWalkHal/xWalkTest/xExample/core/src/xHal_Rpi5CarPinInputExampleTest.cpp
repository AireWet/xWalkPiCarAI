/******************************************************************************
 * @file        xHal_Rpi5CarPinInputExampleTest.cpp
 * @brief       Verifies pin-input sampling without physical GPIO access.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarPinInputExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory input adapter and host scenarios. */
namespace
{

/** @brief Records pin values, reports, waits, and the next read index. */
struct PinInputExampleState
{
    XWalkHal::uint32vector values{1U, 0U, 1U};
    XWalkHal::uint32vector reports;
    XWalkHal::uint32vector waits;
    XWalkHal::size readIndex{};
};

/** @brief Returns the next configured logical pin value. */
XWalkHal::boolean read(XWalkHal::contextpointer context)
{
    PinInputExampleState& state =
        *static_cast<PinInputExampleState*>(context);
    const XWalkHal::boolean value = state.values[state.readIndex] != 0U;
    ++state.readIndex;
    return value;
}

/** @brief Records one source wait duration. */
void wait(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMilliseconds)
{
    static_cast<PinInputExampleState*>(context)->waits.push_back(
        durationMilliseconds);
}

/** @brief Records one logical value reported by the example. */
void report(XWalkHal::contextpointer context, XWalkHal::boolean value)
{
    static_cast<PinInputExampleState*>(context)->reports.push_back(
        value ? 1U : 0U);
}

/** @brief Returns the complete in-memory operation table. */
xwalk::hal::example::XWalkPinInputExampleCallbacks callbacks()
{
    return {&read, &wait, &report};
}

/** @brief Verifies bounded read, report, and 100-millisecond wait ordering. */
void testSampling()
{
    PinInputExampleState state;
    xwalk::hal::example::XWalkPinInputExample example(&state, callbacks());

    example.run(3U);

    assert(state.readIndex == 3U);
    assert(state.reports == XWalkHal::uint32vector({1U, 0U, 1U}));
    assert(state.waits == XWalkHal::uint32vector({100U, 100U, 100U}));
}

/** @brief Verifies callback completeness and bounded sample validation. */
void testValidation()
{
    PinInputExampleState state;
    xwalk::hal::example::XWalkPinInputExampleCallbacks incomplete = callbacks();
    incomplete.read = nullptr;
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::example::XWalkPinInputExample invalidExample(
            &state, incomplete);
    });

    xwalk::hal::example::XWalkPinInputExample example(&state, callbacks());
    xwalk::hal::test::expectFailure([&]()
    {
        example.run(0U);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe pin-input example verification.
 * @return Zero after every assertion passes.
 */
int xWalkPinInputExampleHostTest()
{
    testSampling();
    testValidation();
    return 0;
}
