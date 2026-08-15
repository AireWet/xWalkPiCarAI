/******************************************************************************
 * @file        xHal_Rpi5CarRgbLedTest.cpp
 * @brief       Verifies RGB LED behavior using named in-memory PWM support.
 * @project     xWalk Firmware
 * @module      xWalkLed Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarRgbLed.h"
#include "xHal_Rpi5CarLedSimulationConfig.h"
#include "xHal_Rpi5CarLedTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
namespace
{
    using namespace xwalk::hal::test::led;
    /** @brief Verifies active-high common-cathode component conversion. */
    void testCathodeComponents()
    {
        RgbFixture fixture;
        XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue, XWalkHal::XWalkRgbLedCommon::Cathode);
        const XWalkHal::rgbcolor requestedColor{255U, 128U, 0U};
        led.setColor(requestedColor);
        assert(nearlyEqual(fixture.red.pulseWidthPercent(), 100.0));
        assert(nearlyEqual(fixture.green.pulseWidthPercent(), 50.196'078'431'372'55));
        assert(nearlyEqual(fixture.blue.pulseWidthPercent(), 0.0));
        assert(led.color() == requestedColor);
        assert(led.common() == XWalkHal::XWalkRgbLedCommon::Cathode);
    }
    /** @brief Verifies common-anode inversion and the default mode. */
    void testAnodeComponents()
    {
        RgbFixture fixture;
        XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue);
        led.setColor(XWalkHal::rgbcolor({255U, 128U, 0U}));
        assert(nearlyEqual(fixture.red.pulseWidthPercent(), 0.0));
        assert(nearlyEqual(fixture.green.pulseWidthPercent(), 49.803'921'568'627'45));
        assert(nearlyEqual(fixture.blue.pulseWidthPercent(), 100.0));
        assert(led.common() == XWalkHal::XWalkRgbLedCommon::Anode);
    }
    /** @brief Verifies packed and hexadecimal color decoding. */
    void testEncodedColors()
    {
        RgbFixture fixture;
        XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue, XWalkHal::XWalkRgbLedCommon::Cathode);
        led.setColor(0x12'34'56U);
        assert(led.color() == XWalkHal::rgbcolor({0x12U, 0x34U, 0x56U}));
        led.setColor("##A1b2C3##");
        assert(led.color() == XWalkHal::rgbcolor({0xA1U, 0xB2U, 0xC3U}));
    }
    /** @brief Verifies rejection of invalid modes and encoded color inputs. */
    void testValidation()
    {
        RgbFixture fixture;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                const XWalkHal::XWalkRgbLedCommon invalidMode = static_cast<XWalkHal::XWalkRgbLedCommon>(2U);
                XWalkHal::XWalkRgbLed invalidLed(fixture.red, fixture.green, fixture.blue, invalidMode);
                static_cast<void>(invalidLed);
            });
        XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                led.setColor(0x01'00'00'00U);
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                led.setColor("12XZ56");
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                led.setColor("#12345");
            });
    }
} /* namespace */
/** @brief Runs all host-side RGB LED tests. */
XWalkHal::int32 main()
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_LED_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_LED_SIMULATION_TRACE_LOG_PATH);
    XWALK_HAL_TRACE_UID0(RPI .273, "xWalkLed RGB host tests started");
    testCathodeComponents();
    testAnodeComponents();
    testEncodedColors();
    testValidation();
    XWALK_HAL_TRACE_UID0(RPI .274, "xWalkLed RGB host tests completed");
    return 0;
}
