/******************************************************************************
 * @file        xHal_Rpi5CarBuzzerTest.cpp
 * @brief       Verifies buzzer behavior using in-memory hardware dependencies.
 *
 * @details
 * Checks passive PWM and active GPIO control, construction state, continuous
 * and finite playback, unsupported operations, and duration validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkBuzzer Host Test
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

#include "xHal_Rpi5CarBuzzer.h"

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

/** @brief Records simulated I2C and GPIO output traffic. */
struct TestBackend
{
    /** @brief Number of simulated I2C register writes. */
    XWalkHal::uint32 i2cWriteCount{};
    /** @brief Number of simulated GPIO configurations. */
    XWalkHal::uint32 gpioConfigureCount{};
    /** @brief Number of simulated GPIO writes. */
    XWalkHal::uint32 gpioWriteCount{};
    /** @brief Most recent physical GPIO output level. */
    XWalkHal::boolean gpioValue{};
};

/** @brief Owns the caller-created dependency graph for a passive buzzer. */
struct PassiveFixture
{
    /** @brief In-memory backend that outlives the callback interface. */
    TestBackend backend;
    /** @brief I2C callback interface that outlives the PWM object. */
    XWalkHal::XWalkI2c i2c;
    /** @brief Shared PWM timer state that outlives the PWM object. */
    XWalkHal::XWalkPwmTimerState timerState;
    /** @brief PWM output supplied to the buzzer controller by reference. */
    XWalkHal::XWalkPwm pwm;

    /** @brief Constructs a simulated passive-buzzer dependency graph. */
    PassiveFixture();
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Reports that every simulated I2C device is available.
 *
 * @param[in] context
 * Opaque test context, unused by this callback.
 *
 * @param[in] address
 * Seven-bit address, unused by this callback.
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
 * @brief Records one simulated I2C register write.
 *
 * @param[in,out] context
 * Non-null test backend context.
 *
 * @param[in] address
 * Seven-bit address, unused by this callback.
 *
 * @param[in] reg
 * Eight-bit register, unused by this callback.
 *
 * @param[in] data
 * Register payload, unused by this callback.
 */
void writeRegister(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::uint8 reg, const XWalkHal::bytevector& data)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(address);
    static_cast<void>(reg);
    static_cast<void>(data);
    ++backend.i2cWriteCount;
}

/**
 * @brief Supplies a zero-filled simulated I2C response.
 *
 * @param[in] context
 * Opaque test context, unused by this callback.
 *
 * @param[in] address
 * Seven-bit address, unused by this callback.
 *
 * @param[in] length
 * Requested response length in bytes.
 *
 * @return
 * Zero-filled response containing `length` bytes.
 */
XWalkHal::bytevector read(XWalkHal::contextpointer context, XWalkHal::uint8 address,
    XWalkHal::size length)
{
    static_cast<void>(context);
    static_cast<void>(address);
    return XWalkHal::bytevector(length, 0U);
}

/**
 * @brief Records one simulated GPIO configuration.
 *
 * @param[in,out] context
 * Non-null test backend context.
 *
 * @param[in] pin
 * GPIO line offset, unused by this callback.
 *
 * @param[in] mode
 * Requested direction, unused by this callback.
 *
 * @param[in] pull
 * Requested bias, unused by this callback.
 *
 * @param[in] initialValue
 * Initial physical output value.
 */
void configureGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
    XWalkHal::boolean initialValue)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    ++backend.gpioConfigureCount;
    backend.gpioValue = initialValue;
}

/**
 * @brief Returns the most recent simulated GPIO level.
 *
 * @param[in] context
 * Non-null test backend context.
 *
 * @param[in] pin
 * GPIO line offset, unused by this callback.
 *
 * @return
 * Most recent physical GPIO output level.
 */
XWalkHal::boolean readGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    const TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    return backend.gpioValue;
}

/**
 * @brief Records one simulated GPIO output operation.
 *
 * @param[in,out] context
 * Non-null test backend context.
 *
 * @param[in] pin
 * GPIO line offset, unused by this callback.
 *
 * @param[in] value
 * Physical output level to record.
 */
void writeGpio(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::boolean value)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    ++backend.gpioWriteCount;
    backend.gpioValue = value;
}

/**
 * @brief Accepts an unused simulated interrupt registration.
 *
 * @param[in] context
 * Opaque test context.
 *
 * @param[in] pin
 * GPIO line offset.
 *
 * @param[in] edge
 * Requested edge selection.
 *
 * @param[in] debounceMs
 * Requested debounce interval in milliseconds.
 *
 * @param[in] handlerContext
 * Opaque handler context.
 *
 * @param[in] handler
 * Application interrupt handler.
 */
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

/**
 * @brief Accepts an unused simulated interrupt cancellation.
 *
 * @param[in] context
 * Opaque test context.
 *
 * @param[in] pin
 * GPIO line offset.
 */
void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(context);
    static_cast<void>(pin);
}

/**
 * @brief Constructs the passive fixture and its PWM channel.
 */
PassiveFixture::PassiveFixture():
    i2c(&backend, &probe, &writeRegister, &read),
    pwm(i2c, 0U, XHAL_RPI5CAR_I2C_ADDRESS_1, timerState)
{
}

/**
 * @brief Creates the complete callback set for active-buzzer GPIO tests.
 *
 * @return
 * Callback set bound to translation-unit test functions.
 */
XWalkHal::XWalkGpioCallbacks gpioCallbacks()
{
    return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
}

/** @brief Verifies passive-buzzer construction, activation, and deactivation. */
void testPassiveControl()
{
    PassiveFixture fixture;
    XWalkHal::XWalkBuzzer buzzer(fixture.pwm);
    assert(buzzer.isPassive());
    assert(!buzzer.isOn());
    assert(fixture.pwm.pulseWidthPercent() == 0.0);

    buzzer.on();
    assert(buzzer.isOn());
    assert(fixture.pwm.pulseWidthPercent() == 50.0);
    buzzer.off();
    assert(!buzzer.isOn());
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
    assert(!buzzer.isOn());
    assert(fixture.pwm.pulseWidthPercent() == 0.0);
}

/** @brief Verifies active-buzzer construction, GPIO output, and restrictions. */
void testActiveControl()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbacks = gpioCallbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbacks, "D4");
    XWalkHal::XWalkBuzzer buzzer(gpio);
    assert(!buzzer.isPassive());
    assert(!buzzer.isOn());
    assert(!backend.gpioValue);

    buzzer.on();
    assert(buzzer.isOn());
    assert(backend.gpioValue);
    buzzer.off();
    assert(!buzzer.isOn());
    assert(!backend.gpioValue);

    xwalk::hal::test::expectFailure([&]()
    {
        buzzer.setFrequency(440.0);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        buzzer.play(440.0);
    });
}

/** @brief Verifies finite and non-negative playback-duration requirements. */
void testDurationValidation()
{
    PassiveFixture fixture;
    XWalkHal::XWalkBuzzer buzzer(fixture.pwm);

    xwalk::hal::test::expectFailure([&]()
    {
        buzzer.play(440.0, -1.0);
    });
    assert(!buzzer.isOn());

    xwalk::hal::test::expectFailure([&]()
    {
        buzzer.play(440.0, XHAL_POSITIVE_INFINITY(XWalkHal::float64));
    });
    assert(!buzzer.isOn());
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side buzzer tests.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main()
{
    testPassiveControl();
    testPassivePlayback();
    testActiveControl();
    testDurationValidation();
    return 0;
}
