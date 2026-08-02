/******************************************************************************
 * @file        xHal_Rpi5CarRgbLedTest.cpp
 * @brief       Verifies RGB LED behavior using in-memory PWM dependencies.
 *
 * @details
 * Checks common-terminal polarity, all supported color representations, state
 * reporting, and invalid color or connection inputs without physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkLed Host Test
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

#include "xHal_Rpi5CarRgbLed.h"

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
 * Constants
 ******************************************************************************/

/** @brief Maximum accepted floating-point difference in percentage points. */
constexpr XWalkHal::float64 PERCENT_TOLERANCE {0.000'001};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records the number of register writes made by simulated PWM objects. */
struct TestBus
{
    /** @brief Number of simulated Robot HAT register writes. */
    XWalkHal::uint32 writeCount{};
};

/** @brief Owns the dependency graph required by one RGB LED test. */
struct RgbFixture
{
    /** @brief In-memory backend state that outlives the I2C callback interface. */
    TestBus bus;
    /** @brief Callback-based I2C interface that outlives all PWM channels. */
    XWalkHal::XWalkI2c i2c;
    /** @brief Shared timer state that outlives all PWM channels. */
    XWalkHal::XWalkPwmTimerState timerState;
    /** @brief PWM output assigned to the red channel. */
    XWalkHal::XWalkPwm red;
    /** @brief PWM output assigned to the green channel. */
    XWalkHal::XWalkPwm green;
    /** @brief PWM output assigned to the blue channel. */
    XWalkHal::XWalkPwm blue;

    /** @brief Constructs the callback interface and three PWM channels. */
    RgbFixture();
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Reports that every simulated I2C address is available.
 *
 * @param[in] context
 * Opaque test context, unused by this callback.
 *
 * @param[in] address
 * Seven-bit address being probed, unused by this callback.
 *
 * @return
 * Always `true`.
 */
XWalkHal::boolean probe(XWalkHal::contextpointer context, XWalkHal::uint8 address)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return true;
}

/**
 * @brief Records one simulated PWM register write.
 *
 * @param[in,out] context
 * Non-null test-bus context.
 *
 * @param[in] address
 * Seven-bit destination address, unused by this callback.
 *
 * @param[in] reg
 * Eight-bit register address, unused by this callback.
 *
 * @param[in] data
 * Register payload, unused by this callback.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    TestBus& bus = *static_cast<TestBus*>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    ++bus.writeCount;
}

/**
 * @brief Supplies zero-filled bytes for the simulated I2C interface.
 *
 * @param[in] context
 * Opaque test context, unused by this callback.
 *
 * @param[in] address
 * Seven-bit source address, unused by this callback.
 *
 * @param[in] length
 * Requested byte count.
 *
 * @return
 * Zero-filled payload containing `length` bytes.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return XWalkHal::bytevector(length, 0U);
}

/**
 * @brief Constructs one fully caller-owned simulated RGB LED dependency graph.
 */
RgbFixture::RgbFixture():
    i2c(&bus, &probe, &writeRegister, &read),
    red(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState),
    green(i2c, 1U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState),
    blue(i2c, 2U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState)
{
}

/**
 * @brief Reports whether two PWM percentages are equal within test tolerance.
 *
 * @param[in] left
 * First percentage value.
 *
 * @param[in] right
 * Second percentage value.
 *
 * @return
 * `true` when the absolute difference does not exceed `PERCENT_TOLERANCE`.
 */
XWalkHal::boolean nearlyEqual(XWalkHal::float64 left, XWalkHal::float64 right)
{
    return XHAL_ABSOLUTE_VALUE(left - right) <= PERCENT_TOLERANCE;
}

/** @brief Verifies active-high common-cathode component conversion. */
void testCathodeComponents()
{
    RgbFixture fixture;
    XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue,
        XWalkHal::XWalkRgbLedCommon::Cathode);
    const XWalkHal::rgbcolor requestedColor {255U, 128U, 0U};
    led.setColor(requestedColor);

    assert(nearlyEqual(fixture.red.pulseWidthPercent(), 100.0));
    assert(nearlyEqual(fixture.green.pulseWidthPercent(), 50.196'078'431'372'55));
    assert(nearlyEqual(fixture.blue.pulseWidthPercent(), 0.0));
    assert(led.color() == requestedColor);
    assert(led.common() == XWalkHal::XWalkRgbLedCommon::Cathode);
}

/** @brief Verifies common-anode inversion and the Python-compatible default mode. */
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
    XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue,
        XWalkHal::XWalkRgbLedCommon::Cathode);
    led.setColor(0x12'34'56U);
    assert(led.color() == XWalkHal::rgbcolor({0x12U, 0x34U, 0x56U}));

    led.setColor("##A1b2C3##");
    assert(led.color() == XWalkHal::rgbcolor({0xA1U, 0xB2U, 0xC3U}));
}

/** @brief Verifies rejection of invalid modes and encoded color inputs. */
void testValidation()
{
    RgbFixture fixture;

    xwalk::hal::test::expectFailure([&]()
    {
        const XWalkHal::XWalkRgbLedCommon invalidMode =
            static_cast<XWalkHal::XWalkRgbLedCommon>(2U);
        XWalkHal::XWalkRgbLed invalidLed(fixture.red, fixture.green, fixture.blue, invalidMode);
        static_cast<void>(invalidLed);
    });

    XWalkHal::XWalkRgbLed led(fixture.red, fixture.green, fixture.blue);
    xwalk::hal::test::expectFailure([&]()
    {
        led.setColor(0x01'00'00'00U);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        led.setColor("12XZ56");
    });

    xwalk::hal::test::expectFailure([&]()
    {
        led.setColor("#12345");
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side RGB LED tests.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main()
{
    testCathodeComponents();
    testAnodeComponents();
    testEncodedColors();
    testValidation();
    return 0;
}
