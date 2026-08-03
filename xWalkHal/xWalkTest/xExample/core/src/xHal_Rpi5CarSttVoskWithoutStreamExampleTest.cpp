/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWithoutStreamExampleTest.cpp
 * @brief       Verifies non-streaming speech flow without microphone access.
 *
 * @details
 * Uses deterministic transcripts to verify prompts, final-result output,
 * session and timeout propagation, and bounded validation behavior.
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

#include "xHal_Rpi5CarSttVoskWithoutStreamExample.h"

#include <cassert>

namespace
{

/** @brief Records deterministic recognition and reporting activity. */
struct WithoutStreamExampleState
{
    XWalkHal::stringvector transcripts{"hello", "robot ready"};
    XWalkHal::stringvector reports;
    XWalkHal::uint32vector timeouts;
    XWalkHal::size transcriptIndex{};
};

/** @brief Returns one configured final transcript. */
XWalkHal::string listen(
    XWalkHal::contextpointer context, XWalkHal::uint32 timeoutMs)
{
    WithoutStreamExampleState& state =
        *static_cast<WithoutStreamExampleState*>(context);
    state.timeouts.push_back(timeoutMs);
    const XWalkHal::string transcript = state.transcripts[state.transcriptIndex];
    ++state.transcriptIndex;
    return transcript;
}

/** @brief Records one literal prompt or transcript. */
void report(XWalkHal::contextpointer context, XWalkHal::stringview text)
{
    static_cast<WithoutStreamExampleState*>(context)->reports.emplace_back(text);
}

/** @brief Returns the complete deterministic callback table. */
xwalk::hal::example::XWalkSttVoskWithoutStreamExampleCallbacks callbacks()
{
    return {&listen, &report};
}

/** @brief Verifies exact prompt and unlabeled final-result ordering. */
void testRecognitionFlow()
{
    WithoutStreamExampleState state;
    xwalk::hal::example::XWalkSttVoskWithoutStreamExample example(&state, callbacks());

    example.run(2U, 5'000U);

    assert(state.transcriptIndex == 2U);
    assert(state.timeouts == XWalkHal::uint32vector({5'000U, 5'000U}));
    assert(state.reports == XWalkHal::stringvector({
        "Say something", "hello", "Say something", "robot ready"}));
}

/** @brief Verifies callback, session-count, and timeout validation. */
void testValidation()
{
    WithoutStreamExampleState state;
    auto incomplete = callbacks();
    incomplete.report = nullptr;
    XWalkHal::boolean rejectedCallbacks = false;
    try
    {
        xwalk::hal::example::XWalkSttVoskWithoutStreamExample invalid(
            &state, incomplete);
    }
    catch (const XWalkHal::invalidargument&)
    {
        rejectedCallbacks = true;
    }
    assert(rejectedCallbacks);

    xwalk::hal::example::XWalkSttVoskWithoutStreamExample example(&state, callbacks());
    XWalkHal::boolean rejectedCount = false;
    XWalkHal::boolean rejectedTimeout = false;
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
        example.run(1U, XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS + 1U);
    }
    catch (const XWalkHal::outofrange&)
    {
        rejectedTimeout = true;
    }
    assert(rejectedCount);
    assert(rejectedTimeout);
}

} /* namespace */

/** @brief Runs the host-safe non-streaming Vosk example verification. */
int xWalkSttVoskWithoutStreamExampleHostTest()
{
    testRecognitionFlow();
    testValidation();
    return 0;
}
