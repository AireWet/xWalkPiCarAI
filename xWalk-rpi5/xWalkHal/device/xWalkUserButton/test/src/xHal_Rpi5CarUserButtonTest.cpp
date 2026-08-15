/******************************************************************************
 * @file        xHal_Rpi5CarUserButtonTest.cpp
 * @brief       Verifies user-button behavior using an in-memory GPIO backend.
 * @project     xWalk Firmware
 * @module      xWalkUserButton Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarUserButton.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarUserButtonSimulationArguments.h"
#include "xHal_Rpi5CarUserButtonSimulationConfig.h"
#include "xHal_Rpi5CarUserButtonTestSupport.h"
namespace
{
    using namespace xwalk::hal::test::userbutton;

    /** @brief Verifies short-press state, duration, and callback dispatch. */
    void testShortPress()
    {
        TestBackend backend;
        EventCounts counts;
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(
            &backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up);
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
        assert(button.isPressed() == false);
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
        XWalkHal::XWalkGpio gpio(
            &backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up);
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

    /** @brief Verifies threshold validation and worker GPIO failure behavior. */
    void testFailures()
    {
        TestBackend backend;
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(
            &backend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up);
        XWalkHal::XWalkUserButton button(gpio);
        button.setOnLongPress(nullptr, nullptr, 10.0);
        assert(button.longPressDurationSeconds() == 5.0);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                button.setOnLongPress(nullptr, nullptr, XHAL_POSITIVE_INFINITY(XWalkHal::float64));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                TestBackend failingBackend;
                failingBackend.failReads.store(true);
                XWalkHal::XWalkGpio failingGpio(
                    &failingBackend, callbacks, "USER", XWalkHal::XWalkGpioMode::Input, XWalkHal::XWalkGpioPull::Up);
                XWalkHal::XWalkUserButton failingButton(failingGpio);
                failingButton.start();
                XWalkHal::common::sleepMilliseconds(100U);
            });
    }

    /** @brief Verifies persistent UserButton trace-selector behavior. */
    void testTraceSelection()
    {
        char executable[] = "xWalkUserButtonTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.227.enable";
        char disableSelector[] = "RPI.227.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
        xwalk::hal::sim::XWalkUserButtonSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
        xwalk::hal::sim::XWalkUserButtonSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        XWalkHal::charpointer malformedArguments[]{executable, option, malformedSelector};
        const xwalk::hal::sim::XWalkUserButtonSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace */

/** @brief Runs all host-side user-button tests. */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_USER_BUTTON_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_USER_BUTTON_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .230, "xWalkUserButton host tests started");
    testShortPress();
    testLongPress();
    testFailures();
    testTraceSelection();
    XWALK_HAL_TRACE_UID0(RPI .231, "xWalkUserButton host tests completed");
    return 0;
}
