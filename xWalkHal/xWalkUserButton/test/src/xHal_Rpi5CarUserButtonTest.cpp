/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonTest.cpp
 * @brief       Verifies user-button behavior using an in-memory GPIO backend.
 *
 * @details
 * Checks short and long presses, callback ordering data, state duration,
 * threshold validation, worker lifecycle, and fatal worker failures.
 *
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarUserButton.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Supplies an atomic simulated input level and optional read failure. */
struct TestBackend
{
    /** @brief Simulated pull-up input level; `true` represents released. */
    XWalkHal::atomicboolean inputLevel{true};
    /** @brief `true` when subsequent reads must report a hardware failure. */
    XWalkHal::atomicboolean failReads{false};
};

/** @brief Counts each user-button callback observation. */
struct EventCounts
{
    /** @brief Number of short-click callbacks. */
    XWalkHal::uint32 clicks{};
    /** @brief Number of press callbacks. */
    XWalkHal::uint32 presses{};
    /** @brief Number of release callbacks. */
    XWalkHal::uint32 releases{};
    /** @brief Number of state callbacks carrying `true`. */
    XWalkHal::uint32 pressedStates{};
    /** @brief Number of state callbacks carrying `false`. */
    XWalkHal::uint32 releasedStates{};
    /** @brief Number of long-press callbacks. */
    XWalkHal::uint32 longPresses{};
    /** @brief Number of long-press-release callbacks. */
    XWalkHal::uint32 longReleases{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Accepts one simulated GPIO configuration. */
void configure(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
    XWalkHal::boolean initialValue)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    static_cast<void>(initialValue);
}

/**
 * @brief Returns the simulated input level or reports a configured failure.
 *
 * @return
 * Current active-low button input level.
 *
 * @throws std::runtime_error
 * If simulated reads are configured to fail.
 */
XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    const hal::boolean simulatedReadFailure =
        static_cast<hal::boolean>(
            backend.failReads.load());
    if (simulatedReadFailure)
    {
        XHAL_THROW_RUNTIME_ERROR("Simulated user-button GPIO failure");
    }
    return backend.inputLevel.load();
}

/** @brief Accepts an unused simulated GPIO output operation. */
void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::boolean value)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(value);
}

/** @brief Accepts an unused simulated interrupt registration. */
void registerInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioEdge edge, XWalkHal::uint32 debounceMs,
    XWalkHal::contextpointer handlerContext, XWalkHal::gpiointerrupthandler handler)
{
    static_cast<void>(context);
    static_cast<void>(pin);
    static_cast<void>(edge);
    static_cast<void>(debounceMs);
    static_cast<void>(handlerContext);
    static_cast<void>(handler);
}

/** @brief Accepts an unused simulated interrupt cancellation. */
void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/** @brief Returns the complete callback set used by every test scenario. */
XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configure, &read, &write, &registerInterrupt, &cancelInterrupt};
}

/** @brief Counts one short-click callback. */
void countClick(XWalkHal::contextpointer context)
{
    ++static_cast<EventCounts*>(context)->clicks;
}

/** @brief Counts one press callback. */
void countPress(XWalkHal::contextpointer context)
{
    ++static_cast<EventCounts*>(context)->presses;
}

/** @brief Counts one release callback. */
void countRelease(XWalkHal::contextpointer context)
{
    ++static_cast<EventCounts*>(context)->releases;
}

/** @brief Counts one long-press callback. */
void countLongPress(XWalkHal::contextpointer context)
{
    ++static_cast<EventCounts*>(context)->longPresses;
}

/** @brief Counts one long-press-release callback. */
void countLongRelease(XWalkHal::contextpointer context)
{
    ++static_cast<EventCounts*>(context)->longReleases;
}

/** @brief Counts one press or release state callback. */
void countState(XWalkHal::contextpointer context, XWalkHal::boolean pressed)
{
    EventCounts& counts = *static_cast<EventCounts*>(context);
    if (pressed)
    {
        ++counts.pressedStates;
    }
    else
    {
        ++counts.releasedStates;
    }
}

/** @brief Verifies short-press state, duration, and callback dispatch. */
void testShortPress()
{
    TestBackend backend;
    EventCounts counts;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input,
        XWalkHal::XWalkGpioPull::Up);
    XWalkHal::XWalkUserButton button(gpio);
    button.setOnClick(&counts, &countClick);
    button.setOnPress(&counts, &countPress);
    button.setOnRelease(&counts, &countRelease);
    button.setOnPressReleased(&counts, &countState);
    button.start();
    XWalkHal::common::sleepMilliseconds(100U);

    backend.inputLevel.store(false);
    XWalkHal::common::sleepMilliseconds(150U);
    assert(button.isPressed());
    assert(button.pressedForSeconds() > 0.0);
    backend.inputLevel.store(true);
    XWalkHal::common::sleepMilliseconds(150U);
    button.stop();

    assert(!button.isPressed());
    assert(button.pressedForSeconds() > 0.0);
    assert(counts.clicks == 1U);
    assert(counts.presses == 1U);
    assert(counts.releases == 1U);
    assert(counts.pressedStates == 1U);
    assert(counts.releasedStates == 1U);
}

/** @brief Verifies one-shot long-press recognition and click suppression. */
void testLongPress()
{
    TestBackend backend;
    EventCounts counts;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input,
        XWalkHal::XWalkGpioPull::Up);
    XWalkHal::XWalkUserButton button(gpio);
    button.setOnClick(&counts, &countClick);
    button.setOnLongPress(&counts, &countLongPress, 1.0);
    button.setOnLongPressReleased(&counts, &countLongRelease, 1.0);
    assert(button.longPressDurationSeconds() == 2.0);
    button.start();
    XWalkHal::common::sleepMilliseconds(100U);
    backend.inputLevel.store(false);
    XWalkHal::common::sleepMilliseconds(2'300U);
    backend.inputLevel.store(true);
    XWalkHal::common::sleepMilliseconds(150U);
    button.stop();

    assert(counts.longPresses == 1U);
    assert(counts.longReleases == 1U);
    assert(counts.clicks == 0U);
}

/** @brief Verifies threshold clamping and non-finite rejection. */
void testThresholdValidation()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input,
        XWalkHal::XWalkGpioPull::Up);
    XWalkHal::XWalkUserButton button(gpio);
    button.setOnLongPress(nullptr, nullptr, 10.0);
    assert(button.longPressDurationSeconds() == 5.0);

    xwalk::hal::test::expectFailure([&]()
    {
        button.setOnLongPress(nullptr, nullptr,
            XHAL_POSITIVE_INFINITY(XWalkHal::float64));
    });
}

/** @brief Verifies that a worker GPIO failure terminates the process. */
void testWorkerFailure()
{
    xwalk::hal::test::expectFailure([&]()
    {
        TestBackend backend;
        backend.failReads.store(true);
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(&backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input,
            XWalkHal::XWalkGpioPull::Up);
        XWalkHal::XWalkUserButton button(gpio);
        button.start();
        XWalkHal::common::sleepMilliseconds(100U);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side user-button tests.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main()
{
    testShortPress();
    testLongPress();
    testThresholdValidation();
    testWorkerFailure();
    return 0;
}
