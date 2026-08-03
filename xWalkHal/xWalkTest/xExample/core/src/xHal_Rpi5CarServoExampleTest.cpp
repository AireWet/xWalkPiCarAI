/******************************************************************************
 * @file        xHal_Rpi5CarServoExampleTest.cpp
 * @brief       Verifies servo sweeping without physical movement.
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

#include "xHal_Rpi5CarServoExample.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains the in-memory servo adapter and host scenarios. */
namespace
{

struct ServoExampleState
{
    /** @brief Ordered angles commanded by the example. */
    XWalkHal::float64vector angles;
    /** @brief Ordered wait durations requested by the example. */
    XWalkHal::uint32vector waits;
    /** @brief Ordered angle values reported by the example. */
    XWalkHal::float64vector reports;
};

void setAngle(XWalkHal::contextpointer context, XWalkHal::float64 angleDegrees)
{
    static_cast<ServoExampleState*>(context)->angles.push_back(angleDegrees);
}

void wait(XWalkHal::contextpointer context,
    XWalkHal::uint32 durationMilliseconds)
{
    static_cast<ServoExampleState*>(context)->waits.push_back(
        durationMilliseconds);
}

void report(XWalkHal::contextpointer context, XWalkHal::int32 angleDegrees)
{
    static_cast<ServoExampleState*>(context)->reports.push_back(
        static_cast<XWalkHal::float64>(angleDegrees));
}

xwalk::hal::example::XWalkServoExampleCallbacks callbacks()
{
    return {&setAngle, &wait, &report};
}

/** @brief Verifies every range boundary, command count, and source delay. */
void testSweep()
{
    ServoExampleState state;
    xwalk::hal::example::XWalkServoExample example(&state, callbacks());

    example.run(1U);

    assert(state.angles.size() == 360U);
    assert(state.reports.size() == 360U);
    assert(state.waits.size() == 362U);
    assert(state.angles.front() == -90.0);
    assert(state.angles[179U] == 89.0);
    assert(state.angles[180U] == 90.0);
    assert(state.angles.back() == -89.0);
    assert(state.reports.front() == -90.0);
    assert(state.reports.back() == -89.0);
    assert(state.waits[179U] == 10U);
    assert(state.waits[180U] == 1'000U);
    assert(state.waits.back() == 1'000U);
}

/** @brief Verifies callback completeness and bounded cycle validation. */
void testValidation()
{
    ServoExampleState state;
    xwalk::hal::example::XWalkServoExampleCallbacks incomplete = callbacks();
    incomplete.setAngle = nullptr;
    XWalkHal::boolean rejectedCallbacks = false;
    try
    {
        xwalk::hal::example::XWalkServoExample invalidExample(&state, incomplete);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedCallbacks = true;
    }
    assert(rejectedCallbacks);

    xwalk::hal::example::XWalkServoExample example(&state, callbacks());
    XWalkHal::boolean rejectedCount = false;
    try
    {
        example.run(0U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedCount = true;
    }
    assert(rejectedCount);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/** @brief Runs the host-safe servo example verification. */
int xWalkServoExampleHostTest()
{
    testSweep();
    testValidation();
    return 0;
}
