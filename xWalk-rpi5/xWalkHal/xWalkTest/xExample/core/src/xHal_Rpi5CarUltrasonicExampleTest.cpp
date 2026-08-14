/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicExampleTest.cpp
 * @brief       Verifies ultrasonic sampling without physical GPIO access.
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

#include "xHal_Rpi5CarUltrasonicExample.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory ranging adapter and host scenarios. */
namespace
{

/** @brief Records distances, reports, waits, and the next read index. */
struct UltrasonicExampleState
{
    XWalkHal::float64vector values{12.5, 24.75, -1.0};
    XWalkHal::float64vector reports;
    XWalkHal::uint32vector waits;
    XWalkHal::size readIndex{};
};

/** @brief Returns the next configured distance in centimeters. */
XWalkHal::float64 read(XWalkHal::contextpointer context)
{
    UltrasonicExampleState& state =
        *static_cast<UltrasonicExampleState*>(context);
    const XWalkHal::float64 value = state.values[state.readIndex];
    ++state.readIndex;
    return value;
}

/** @brief Records one source wait duration. */
void wait(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMilliseconds)
{
    static_cast<UltrasonicExampleState*>(context)->waits.push_back(
        durationMilliseconds);
}

/** @brief Records one distance reported by the example. */
void report(XWalkHal::contextpointer context,
    XWalkHal::float64 distanceCentimeters)
{
    static_cast<UltrasonicExampleState*>(context)->reports.push_back(
        distanceCentimeters);
}

/** @brief Returns the complete in-memory operation table. */
xwalk::hal::example::XWalkUltrasonicExampleCallbacks callbacks()
{
    return {&read, &wait, &report};
}

/** @brief Verifies bounded read, report, and 200-millisecond wait ordering. */
void testSampling()
{
    UltrasonicExampleState state;
    xwalk::hal::example::XWalkUltrasonicExample example(&state, callbacks());

    example.run(3U);

    assert(state.readIndex == 3U);
    assert(state.reports == XWalkHal::float64vector({12.5, 24.75, -1.0}));
    assert(state.waits == XWalkHal::uint32vector({200U, 200U, 200U}));
}

/** @brief Verifies callback completeness and bounded sample validation. */
void testValidation()
{
    UltrasonicExampleState state;
    xwalk::hal::example::XWalkUltrasonicExampleCallbacks incomplete = callbacks();
    incomplete.read = nullptr;
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::example::XWalkUltrasonicExample invalidExample(
            &state, incomplete);
    });

    xwalk::hal::example::XWalkUltrasonicExample example(&state, callbacks());
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
 * @brief Runs the host-safe ultrasonic example verification.
 * @return Zero after every assertion passes.
 */
int xWalkUltrasonicExampleHostTest()
{
    testSampling();
    testValidation();
    return 0;
}
