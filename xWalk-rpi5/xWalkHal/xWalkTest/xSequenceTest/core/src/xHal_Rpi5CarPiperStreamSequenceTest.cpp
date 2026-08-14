/******************************************************************************
 * @file        xHal_Rpi5CarPiperStreamSequenceTest.cpp
 * @brief       Verifies the Piper stream-comparison sequence in memory.
 *
 * @details
 * Checks the model, exact text, streamed and buffered flags, timing boundaries,
 * report order, callback validation, and backwards-clock rejection.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Host Test
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

#include "xHal_Rpi5CarPiperStreamSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains injected callbacks and the host verification scenario. */
namespace
{

/** @brief Records all observable sequence operations in call order. */
struct TestState
{
    /** @brief Model values received by the simulated Piper provider. */
    XWalkHal::stringvector models;
    /** @brief Text values received by the simulated Piper provider. */
    XWalkHal::stringvector texts;
    /** @brief Ordered literal messages and synthesized reporting events. */
    XWalkHal::stringvector events;
    /** @brief Durations reported for streamed and buffered synthesis. */
    XWalkHal::float64vector durations;
    /** @brief Deterministic monotonic values returned in call order. */
    XWalkHal::fixedarray<XWalkHal::float64, 4U> times{{10.0, 12.5, 20.0, 23.0}};
    /** @brief Stream-mode flags received by the two provider calls. */
    XWalkHal::boolean streamModes[2U]{};
    /** @brief Number of provider calls recorded so far. */
    XWalkHal::uint32 speakCount{};
    /** @brief Index of the next deterministic clock value. */
    XWalkHal::size timeIndex{};
};

/**
 * @brief Records one simulated Piper synthesis request.
 *
 * @param[in,out] context Non-null `TestState` pointer.
 * @param[in] model Piper model name retained only during this call.
 * @param[in] text Speech text retained only during this call.
 * @param[in] stream Requested incremental or buffered playback mode.
 */
void speak(XWalkHal::contextpointer context, XWalkHal::stringview model,
    XWalkHal::stringview text, XWalkHal::boolean stream)
{
    TestState& state = *static_cast<TestState*>(context);
    assert(state.speakCount < 2U);
    state.models.emplace_back(model);
    state.texts.emplace_back(text);
    state.streamModes[state.speakCount] = stream;
    state.events.emplace_back(stream ? "speak:stream" : "speak:buffered");
    ++state.speakCount;
}

/**
 * @brief Returns the next deterministic monotonic value.
 *
 * @param[in,out] context Non-null `TestState` pointer.
 *
 * @return Next configured time value in seconds.
 */
XWalkHal::float64 time(XWalkHal::contextpointer context)
{
    TestState& state = *static_cast<TestState*>(context);
    assert(state.timeIndex < state.times.size());
    const XWalkHal::float64 result = state.times[state.timeIndex];
    ++state.timeIndex;
    return result;
}

/**
 * @brief Records one literal sequence status message.
 *
 * @param[in,out] context Non-null `TestState` pointer.
 * @param[in] message Message view retained only during this call.
 */
void reportMessage(
    XWalkHal::contextpointer context, XWalkHal::stringview message)
{
    TestState& state = *static_cast<TestState*>(context);
    state.events.emplace_back(message);
}

/**
 * @brief Records one measured synthesis duration.
 *
 * @param[in,out] context Non-null `TestState` pointer.
 * @param[in] durationSeconds Non-negative elapsed duration in seconds.
 */
void reportDuration(
    XWalkHal::contextpointer context, XWalkHal::float64 durationSeconds)
{
    TestState& state = *static_cast<TestState*>(context);
    state.durations.push_back(durationSeconds);
    state.events.emplace_back("duration");
}

/**
 * @brief Exercises the complete sequence with deterministic callbacks.
 *
 * @post Every assertion covering calls, data, timing, and validation passes.
 */
void runTest()
{
    TestState state;
    xwalk::hal::test::XWalkPiperStreamSequence sequence(&state, &speak,
        &time, &reportMessage, &reportDuration);

    sequence.run();

    assert(state.models == XWalkHal::stringvector({
        "en_US-amy-low", "en_US-amy-low"}));
    assert(state.texts == XWalkHal::stringvector({
        XWalkHal::string(xwalk::hal::test::XHAL_RPI5CAR_PIPER_STREAM_TEXT),
        XWalkHal::string(xwalk::hal::test::XHAL_RPI5CAR_PIPER_STREAM_TEXT)}));
    assert(state.streamModes[0U]);
    assert(!state.streamModes[1U]);
    assert(state.durations == XWalkHal::float64vector({2.5, 3.0}));
    assert(state.events == XWalkHal::stringvector({
        "Say with stream", "speak:stream", "duration",
        "====================================================", "Say without stream",
        "speak:buffered", "duration"}));

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::test::XWalkPiperStreamSequence invalidSequence(
            &state, nullptr, &time, &reportMessage, &reportDuration);
        static_cast<void>(invalidSequence);
    });

    TestState backwardsState;
    backwardsState.times = {{5.0, 4.0, 0.0, 0.0}};
    xwalk::hal::test::XWalkPiperStreamSequence backwardsSequence(
        &backwardsState, &speak, &time, &reportMessage, &reportDuration);
    xwalk::hal::test::expectFailure([&]()
    {
        backwardsSequence.run();
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe Piper stream-comparison verification.
 *
 * @return Zero after every assertion passes.
 */
int xWalkPiperStreamSequenceHostTest()
{
    runTest();
    return 0;
}
