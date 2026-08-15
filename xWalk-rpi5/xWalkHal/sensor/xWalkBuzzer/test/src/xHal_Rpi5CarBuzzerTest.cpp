/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerTest.cpp
 * @brief       Verifies buzzer behavior using named in-memory support.
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarBuzzer.h"
#include "xHal_Rpi5CarBuzzerSimulationArguments.h"
#include "xHal_Rpi5CarBuzzerSimulationConfig.h"
#include "xHal_Rpi5CarBuzzerTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
namespace
{
    using namespace xwalk::hal::test::buzzer;
    /** @brief Verifies passive-buzzer construction, activation, and deactivation.
     */
    void testPassiveControl()
    {
        PassiveFixture fixture;
        XWalkHal::XWalkBuzzer buzzer(fixture.pwm);
        assert(buzzer.isPassive());
        assert(buzzer.isOn() == false);
        assert(fixture.pwm.pulseWidthPercent() == 0.0);
        buzzer.on();
        assert(buzzer.isOn());
        assert(fixture.pwm.pulseWidthPercent() == 50.0);
        buzzer.off();
        assert(buzzer.isOn() == false);
        assert(fixture.pwm.pulseWidthPercent() == 0.0);
    }
    /** @brief Verifies continuous and zero-duration passive playback. */
    void testPassivePlayback()
    {
        PassiveFixture fixture;
        XWalkHal::XWalkBuzzer buzzer(fixture.pwm);
        buzzer.play(440.0);
        assert(buzzer.isOn());
        assert(fixture.pwm.frequency() > 0.0);
        buzzer.play(880.0, 0.0);
        assert(buzzer.isOn() == false);
        assert(fixture.pwm.pulseWidthPercent() == 0.0);
    }
    /** @brief Verifies active-buzzer GPIO output and passive-only restrictions. */
    void testActiveControl()
    {
        TestBackend backend;
        const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
        XWalkHal::XWalkGpio gpio(&backend, callbacks, "D4");
        XWalkHal::XWalkBuzzer buzzer(gpio);
        assert(buzzer.isPassive() == false);
        assert(buzzer.isOn() == false);
        assert(backend.gpioValue == false);
        buzzer.on();
        assert(buzzer.isOn());
        assert(backend.gpioValue);
        buzzer.off();
        assert(buzzer.isOn() == false);
        assert(backend.gpioValue == false);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                buzzer.setFrequency(440.0);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                buzzer.play(440.0);
            });
    }
    /** @brief Verifies finite and non-negative playback-duration requirements. */
    void testDurationValidation()
    {
        PassiveFixture fixture;
        XWalkHal::XWalkBuzzer buzzer(fixture.pwm);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                buzzer.play(440.0, -1.0);
            });
        assert(buzzer.isOn() == false);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                buzzer.play(440.0, XHAL_POSITIVE_INFINITY(XWalkHal::float64));
            });
        assert(buzzer.isOn() == false);
    }
    /** @brief Verifies persistent Buzzer trace-selector behavior. */
    void testTraceSelection()
    {
        char executable[] = "xWalkBuzzerTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.283.enable";
        char disableSelector[] = "RPI.283.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
        xwalk::hal::sim::XWalkBuzzerSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
        xwalk::hal::sim::XWalkBuzzerSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        XWalkHal::charpointer malformedArguments[]{executable, option, malformedSelector};
        const xwalk::hal::sim::XWalkBuzzerSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace */
/** @brief Runs all host-side buzzer tests. */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_BUZZER_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_BUZZER_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .286, "xWalkBuzzer host tests started");
    testPassiveControl();
    testPassivePlayback();
    testActiveControl();
    testDurationValidation();
    testTraceSelection();
    XWALK_HAL_TRACE_UID0(RPI .287, "xWalkBuzzer host tests completed");
    return 0;
}
