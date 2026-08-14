/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordThreadExampleTest.cpp
 * @brief       Verifies wake-word polling without threads or microphone access.
 *
 * @details
 * Uses deterministic callbacks to verify lifecycle order, status messages,
 * three-second waits, bounded counts, and unsuccessful detection behavior.
 *
 * @project     xWalk Firmware
 * @module      xExample Host Test
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordThreadExample.h"

#include <cassert>

namespace
{

/** @brief Records deterministic wake-word lifecycle and reporting activity. */
struct WakeWordExampleState
{
    XWalkHal::uint32 starts{};
    XWalkHal::uint32 stops{};
    XWalkHal::uint32 pollInAttempt{};
    XWalkHal::uint32 wakeAfterPoll{1U};
    XWalkHal::uint32vector waits;
    XWalkHal::stringvector messages;
};

/** @brief Starts one deterministic attempt and resets its poll index. */
void startListening(XWalkHal::contextpointer context)
{
    WakeWordExampleState& state = *static_cast<WakeWordExampleState*>(context);
    ++state.starts;
    state.pollInAttempt = 0U;
}

/** @brief Returns true after the configured number of unsuccessful polls. */
XWalkHal::boolean isWaked(XWalkHal::contextpointer context)
{
    WakeWordExampleState& state = *static_cast<WakeWordExampleState*>(context);
    const XWalkHal::boolean detected = state.pollInAttempt >= state.wakeAfterPoll;
    ++state.pollInAttempt;
    return detected;
}

/** @brief Records one listener stop. */
void stopListening(XWalkHal::contextpointer context)
{
    ++static_cast<WakeWordExampleState*>(context)->stops;
}

/** @brief Records one source polling delay. */
void wait(XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
{
    static_cast<WakeWordExampleState*>(context)->waits.push_back(durationMilliseconds);
}

/** @brief Records one literal status message. */
void report(XWalkHal::contextpointer context, XWalkHal::stringview message)
{
    static_cast<WakeWordExampleState*>(context)->messages.emplace_back(message);
}

/** @brief Returns the complete deterministic callback table. */
xwalk::hal::example::XWalkSttVoskWakeWordThreadExampleCallbacks callbacks()
{
    return {&startListening, &isWaked, &stopListening, &wait, &report};
}

/** @brief Verifies two complete start, poll, stop, and detect cycles. */
void testDetectionFlow()
{
    WakeWordExampleState state;
    xwalk::hal::example::XWalkSttVoskWakeWordThreadExample example(&state, callbacks());

    example.run(2U, 3U);

    assert(state.starts == 2U);
    assert(state.stops == 2U);
    assert(state.waits == XWalkHal::uint32vector({3'000U, 3'000U}));
    assert(state.messages == XWalkHal::stringvector({
        "Waiting for wake word...", "Wake word detected",
        "Waiting for wake word...", "Wake word detected"}));
}

/** @brief Verifies callback and bounded-count rejection. */
void testValidation()
{
    WakeWordExampleState state;
    auto incomplete = callbacks();
    incomplete.startListening = nullptr;
    XWalkHal::boolean rejectedCallbacks = false;
    try
    {
        xwalk::hal::example::XWalkSttVoskWakeWordThreadExample invalid(&state, incomplete);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedCallbacks = true;
    }
    assert(rejectedCallbacks);

    xwalk::hal::example::XWalkSttVoskWakeWordThreadExample example(&state, callbacks());
    XWalkHal::boolean rejectedCount = false;
    XWalkHal::boolean rejectedPolls = false;
    try
    {
        example.run(0U, 1U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedCount = true;
    }
    try
    {
        example.run(1U, 0U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedPolls = true;
    }
    assert(rejectedCount);
    assert(rejectedPolls);
}

/** @brief Verifies listener shutdown when the bounded poll limit expires. */
void testPollLimit()
{
    WakeWordExampleState state;
    state.wakeAfterPoll = 5U;
    xwalk::hal::example::XWalkSttVoskWakeWordThreadExample example(&state, callbacks());
    XWalkHal::boolean rejected = false;
    try
    {
        example.run(1U, 2U);
    }
    catch (const XWalkHal::runtimeerror&)
    {
        rejected = true;
    }
    assert(rejected);
    assert(state.starts == 1U);
    assert(state.stops == 1U);
}

} /* namespace */

/** @brief Runs the host-safe threaded wake-word example verification. */
int xWalkSttVoskWakeWordThreadExampleHostTest()
{
    testDetectionFlow();
    testValidation();
    testPollLimit();
    return 0;
}
