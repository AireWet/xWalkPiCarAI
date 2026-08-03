/******************************************************************************
 * @file        xHal_Rpi5CarButtonEventSequenceTest.cpp
 * @brief       Verifies the D0 button-event sequence with an in-memory backend.
 *
 * @details
 * Checks GPIO selection, pull-up interrupt registration, debounce, bounded
 * waiting, press/release reporting, timestamps, cleanup, and validation.
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

#include "xHal_Rpi5CarButtonEventSequence.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

struct TestState
{
    XWalkHal::uint8 pin{};
    XWalkHal::XWalkGpioMode mode{XWalkHal::XWalkGpioMode::Output};
    XWalkHal::XWalkGpioEdge edge{XWalkHal::XWalkGpioEdge::Falling};
    XWalkHal::XWalkGpioPull pull{XWalkHal::XWalkGpioPull::None};
    XWalkHal::uint32 debounceMilliseconds{};
    XWalkHal::uint32 waitedMilliseconds{};
    XWalkHal::uint32 cancelCount{};
    XWalkHal::contextpointer handlerContext{nullptr};
    XWalkHal::gpiointerrupthandler handler{nullptr};
    XWalkHal::boolean events[2U]{};
    XWalkHal::float64 timestamps[2U]{};
    XWalkHal::uint32 eventCount{};
};

void configure(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
    XWalkHal::boolean initialValue)
{
    TestState& state = *static_cast<TestState*>(context);
    state.pin = pin;
    state.mode = mode;
    state.pull = pull;
    static_cast<void>(initialValue);
}

XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    return true;
}

void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::boolean value)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(value);
}

void interrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioEdge edge, XWalkHal::uint32 debounceMilliseconds,
    XWalkHal::contextpointer handlerContext, XWalkHal::gpiointerrupthandler handler)
{
    TestState& state = *static_cast<TestState*>(context);
    state.pin = pin;
    state.edge = edge;
    state.debounceMilliseconds = debounceMilliseconds;
    state.handlerContext = handlerContext;
    state.handler = handler;
}

void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    TestState& state = *static_cast<TestState*>(context);
    state.pin = pin;
    ++state.cancelCount;
}

void wait(XWalkHal::contextpointer context, XWalkHal::uint32 durationMilliseconds)
{
    TestState& state = *static_cast<TestState*>(context);
    state.waitedMilliseconds = durationMilliseconds;
    assert(state.handler != nullptr);
    state.handler(state.handlerContext);
    state.handler(state.handlerContext);
}

XWalkHal::float64 time(XWalkHal::contextpointer context)
{
    const TestState& state = *static_cast<TestState*>(context);
    return 10.0 + static_cast<XWalkHal::float64>(state.eventCount);
}

void event(XWalkHal::contextpointer context, XWalkHal::boolean pressed,
    XWalkHal::float64 timestampSeconds)
{
    TestState& state = *static_cast<TestState*>(context);
    assert(state.eventCount < 2U);
    state.events[state.eventCount] = pressed;
    state.timestamps[state.eventCount] = timestampSeconds;
    ++state.eventCount;
}

XWalkHal::XWalkGpioCallbacks callbacks()
{
    return {&configure, &read, &write, &interrupt, &cancelInterrupt};
}

void testButtonEventSequence()
{
    TestState state;
    const XWalkHal::XWalkGpioCallbacks gpioCallbacks = callbacks();
    XWalkHal::XWalkGpio gpio(&state, gpioCallbacks, "D0",
        XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up);
    xwalk::hal::test::XWalkButtonEventSequence sequence(
        gpio, &state, &wait, &time, &event);
    sequence.run(2U);

    assert(state.pin == 17U);
    assert(state.mode == XWalkHal::XWalkGpioMode::Input);
    assert(state.pull == XWalkHal::XWalkGpioPull::Up);
    assert(state.edge == XWalkHal::XWalkGpioEdge::Both);
    assert(state.debounceMilliseconds == 10U);
    assert(state.waitedMilliseconds == 2'000U);
    assert(state.cancelCount == 1U);
    assert(state.eventCount == 2U);
    assert(state.events[0U]);
    assert(!state.events[1U]);
    assert(state.timestamps[0U] == 10.0);
    assert(state.timestamps[1U] == 11.0);

    XWalkHal::boolean rejectedDuration = false;
    try
    {
        sequence.run(0U);
    }
    catch (const std::out_of_range&)
    {
        rejectedDuration = true;
    }
    assert(rejectedDuration);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs the host-safe button-event sequence verification.
 *
 * @return
 * Zero after every assertion passes.
 */
int xWalkButtonEventSequenceHostTest()
{
    testButtonEventSequence();
    return 0;
}
