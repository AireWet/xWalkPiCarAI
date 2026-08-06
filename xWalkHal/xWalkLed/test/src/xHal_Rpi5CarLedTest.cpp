/******************************************************************************
 * @file        xHal_Rpi5CarLedTest.cpp
 * @brief       Verifies LED behavior using an in-memory GPIO backend.
 *
 * @details
 * Checks initial state, direct output, toggling, background blinking, duration
 * validation, worker shutdown, and fatal worker failures.
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

#include "xHal_Rpi5CarLed.h"

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

/** @brief Records simulated GPIO operations and optional output failure. */
struct TestBackend
{
    /** @brief Number of simulated GPIO configurations. */
    XWalkHal::uint32 configureCount{};
    /** @brief Number of simulated GPIO writes. */
    XWalkHal::uint32 writeCount{};
    /** @brief Most recent physical GPIO level. */
    XWalkHal::boolean value{};
    /** @brief `true` when subsequent writes must report a hardware failure. */
    XWalkHal::atomicboolean failWrites{false};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

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
void configure(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
    XWalkHal::boolean initialValue)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    static_cast<void>(mode);
    static_cast<void>(pull);
    ++backend.configureCount;
    backend.value = initialValue;
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
 * Most recent physical output level.
 */
XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    const TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    return backend.value;
}

/**
 * @brief Records one simulated GPIO output or reports the requested failure.
 *
 * @param[in,out] context
 * Non-null test backend context.
 *
 * @param[in] pin
 * GPIO line offset, unused by this callback.
 *
 * @param[in] value
 * Physical output level to record.
 *
 * @throws std::runtime_error
 * If the backend is configured to fail writes.
 */
void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::boolean value)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(pin);
    const hal::boolean simulatedWriteFailure =
        static_cast<hal::boolean>(
            backend.failWrites.load());
    if (simulatedWriteFailure)
    {
        XHAL_THROW_RUNTIME_ERROR("Simulated LED GPIO failure");
    }
    ++backend.writeCount;
    backend.value = value;
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
 * @brief Creates the complete callback set for LED tests.
 *
 * @return
 * Callback set bound to translation-unit test functions.
 */
XWalkHal::XWalkGpioCallbacks callbacks()
{
    return {&configure, &read, &write, &registerInterrupt, &cancelInterrupt};
}

/** @brief Verifies construction, direct output operations, and toggling. */
void testDirectControl()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
    XWalkHal::XWalkLed led(gpio);
    assert(!led.isOn());
    assert(!backend.value);

    led.on();
    assert(led.isOn());
    assert(backend.value);
    led.toggle();
    assert(!led.isOn());
    assert(!backend.value);
    led.toggle();
    assert(led.isOn());
    led.close();
    assert(!led.isOn());
    assert(!backend.value);
}

/** @brief Verifies background blinking, responsive stopping, and inactive completion. */
void testBlinking()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
    XWalkHal::XWalkLed led(gpio);
    led.blink(1U, 0.001, 0.0);
    XWalkHal::common::sleepMilliseconds(40U);
    led.stopBlinking();
    assert(backend.writeCount > 1U);
    assert(!led.isBlinking());
    assert(!led.isOn());
    assert(!backend.value);
}

/** @brief Verifies rejection of invalid blink count and timing values. */
void testValidation()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
    XWalkHal::XWalkLed led(gpio);

    xwalk::hal::test::expectFailure([&]()
    {
        led.blink(0U);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        led.blink(1U, -0.1);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        led.blink(1U, 0.1, XHAL_POSITIVE_INFINITY(XWalkHal::float64));
    });
}

/** @brief Verifies that a worker hardware failure terminates the process. */
void testWorkerFailure()
{
    xwalk::hal::test::expectFailure([&]()
    {
        TestBackend backend;
        const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
        XWalkHal::XWalkGpio gpio(&backend, callbackSet, "LED");
        XWalkHal::XWalkLed led(gpio);
        backend.failWrites.store(true);
        led.blink(1U, 0.001, 0.0);
        XWalkHal::common::sleepMilliseconds(20U);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side LED tests.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main()
{
    testDirectControl();
    testBlinking();
    testValidation();
    testWorkerFailure();
    return 0;
}
