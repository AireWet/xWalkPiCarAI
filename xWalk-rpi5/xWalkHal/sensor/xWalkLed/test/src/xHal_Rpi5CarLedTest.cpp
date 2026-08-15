/******************************************************************************
 * @file        xHal_Rpi5CarLedTest.cpp
 * @brief       Verifies LED behavior using named in-memory GPIO support.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarLedSimulationArguments.h"
#include "xHal_Rpi5CarLedSimulationConfig.h"
#include "xHal_Rpi5CarLedTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
namespace
{
    using namespace xwalk::hal::test::led;
    /** @brief Verifies construction, direct output operations, and toggling. */
    void testDirectControl()
    {
        GpioBackend backend;
        const XWalkHal::XWalkGpioCallbacks callbackSet = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
        XWalkHal::XWalkLed led(gpio);
        assert(led.isOn() == false);
        assert(backend.value == false);
        led.on();
        assert(led.isOn());
        assert(backend.value);
        led.toggle();
        assert(led.isOn() == false);
        assert(backend.value == false);
        led.toggle();
        assert(led.isOn());
        led.close();
        assert(led.isOn() == false);
        assert(backend.value == false);
    }
    /** @brief Verifies background blinking, responsive stopping, and inactive
     * completion. */
    void testBlinking()
    {
        GpioBackend backend;
        const XWalkHal::XWalkGpioCallbacks callbackSet = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
        XWalkHal::XWalkLed led(gpio);
        led.blink(1U, 0.001, 0.0);
        XWalkHal::common::sleepMilliseconds(40U);
        led.stopBlinking();
        assert(backend.writeCount > 1U);
        assert(led.isBlinking() == false);
        assert(led.isOn() == false);
        assert(backend.value == false);
    }
    /** @brief Verifies rejection of invalid blink count and timing values. */
    void testValidation()
    {
        GpioBackend backend;
        const XWalkHal::XWalkGpioCallbacks callbackSet = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
        XWalkHal::XWalkLed led(gpio);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                led.blink(0U);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                led.blink(1U, -0.1);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                led.blink(1U, 0.1, XHAL_POSITIVE_INFINITY(XWalkHal::float64));
            });
    }
    /** @brief Verifies that a worker hardware failure terminates the child process.
     */
    void testWorkerFailure()
    {
        xwalk::hal::test::expectFailure(
            [&]()
            {
                GpioBackend backend;
                const XWalkHal::XWalkGpioCallbacks callbackSet = gpioCallbacks();
                XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
                XWalkHal::XWalkLed led(gpio);
                backend.failWrites.store(true);
                led.blink(1U, 0.001, 0.0);
                XWalkHal::common::sleepMilliseconds(20U);
            });
    }
    /** @brief Verifies persistent LED trace-selector behavior. */
    void testTraceSelection()
    {
        char executable[] = "xWalkLedTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.268.enable";
        char disableSelector[] = "RPI.268.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
        xwalk::hal::sim::XWalkLedSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
        xwalk::hal::sim::XWalkLedSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        XWalkHal::charpointer malformedArguments[]{executable, option, malformedSelector};
        const xwalk::hal::sim::XWalkLedSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace */
/** @brief Runs all host-side single-color LED tests. */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_LED_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_LED_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .271, "xWalkLed single-color host tests started");
    testDirectControl();
    testBlinking();
    testValidation();
    testWorkerFailure();
    testTraceSelection();
    XWALK_HAL_TRACE_UID0(RPI .272, "xWalkLed single-color host tests completed");
    return 0;
}
