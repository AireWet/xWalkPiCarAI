/******************************************************************************
 * @file        xHal_Rpi5CarGpioTest.cpp
 * @brief       Verifies xWalk GPIO behavior with an in-memory backend.
 *
 * @details
 * Tests named-pin mapping, automatic mode changes, polarity, digital I/O, interrupts, and validation.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Test
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

#include "xHal_Rpi5CarGpio.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records the most recent operation received by the GPIO callbacks. */
struct TestBackend
{
    XWalkHal::size configureCount{}; /**< Number of line configurations. */
    XWalkHal::size writeCount{}; /**< Number of physical writes. */
    XWalkHal::size interruptCount{}; /**< Number of interrupt registrations. */
    XWalkHal::size cancelCount{}; /**< Number of interrupt cancellations. */
    XWalkHal::uint8 pin{}; /**< Most recent GPIO line offset. */
    XWalkHal::XWalkGpioMode mode{XWalkHal::XWalkGpioMode::Output}; /**< Most recent direction. */
    XWalkHal::XWalkGpioPull pull{XWalkHal::XWalkGpioPull::None}; /**< Most recent pull. */
    XWalkHal::XWalkGpioEdge edge{XWalkHal::XWalkGpioEdge::Both}; /**< Most recent edge. */
    XWalkHal::uint32 debounceMs{}; /**< Most recent debounce interval in milliseconds. */
    XWalkHal::boolean initialValue{}; /**< Most recent initial physical output level. */
    XWalkHal::boolean readValue{}; /**< Physical level returned by the read callback. */
    XWalkHal::boolean writeValue{}; /**< Most recent physical level written. */
    XWalkHal::contextpointer handlerContext{nullptr}; /**< Non-owning stored handler context. */
    XWalkHal::gpiointerrupthandler handler{nullptr}; /**< Non-owning stored application handler. */
};

/** @brief Counts application interrupt-handler invocations. */
struct HandlerData
{
    XWalkHal::size count{}; /**< Number of accepted simulated events. */
};

/** @brief Associates one Python-compatible Robot HAT pin name with its Linux line offset. */
struct PinMapping
{
    XWalkHal::cstring name; /**< Non-owning static pin name. */
    XWalkHal::uint8 pin; /**< Expected Linux GPIO line offset. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records one simulated GPIO configuration.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line offset to record.
 *
 * @param[in] mode
 * Direction to record.
 *
 * @param[in] pull
 * Internal bias to record.
 *
 * @param[in] initialValue
 * Initial physical output level to record.
 */
void configure(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::XWalkGpioMode mode,
    XWalkHal::XWalkGpioPull pull, XWalkHal::boolean initialValue)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.configureCount;
    backend.pin = pin;
    backend.mode = mode;
    backend.pull = pull;
    backend.initialValue = initialValue;
}

/**
 * @brief Returns the configured simulated physical GPIO level.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line offset to record.
 *
 * @return
 * Physical level stored in the backend.
 */
XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    backend.pin = pin;
    return backend.readValue;
}

/**
 * @brief Records one simulated physical GPIO write.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line offset to record.
 *
 * @param[in] value
 * Physical level to record.
 */
void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.writeCount;
    backend.pin = pin;
    backend.writeValue = value;
}

/**
 * @brief Records one simulated GPIO interrupt registration.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line offset to record.
 *
 * @param[in] edge
 * Edge selection to record.
 *
 * @param[in] debounceMs
 * Debounce interval in milliseconds.
 *
 * @param[in,out] handlerContext
 * Non-owning application context to store.
 *
 * @param[in] handler
 * Non-owning application handler to store.
 */
void interrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::XWalkGpioEdge edge,
    XWalkHal::uint32 debounceMs, XWalkHal::contextpointer handlerContext,
    XWalkHal::gpiointerrupthandler handler)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.interruptCount;
    backend.pin = pin;
    backend.edge = edge;
    backend.debounceMs = debounceMs;
    backend.handlerContext = handlerContext;
    backend.handler = handler;
}

/**
 * @brief Records one simulated GPIO interrupt cancellation.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line offset to record.
 */
void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.cancelCount;
    backend.pin = pin;
    backend.handlerContext = nullptr;
    backend.handler = nullptr;
}

/**
 * @brief Counts one simulated accepted interrupt event.
 *
 * @param[in,out] context
 * Non-null pointer to `HandlerData`.
 */
void handleInterrupt(XWalkHal::contextpointer context)
{
    HandlerData& data = *static_cast<HandlerData*>(context);
    ++data.count;
}

/**
 * @brief Creates the complete callback set used by every host scenario.
 *
 * @return
 * Callback set bound to translation-unit test functions.
 */
XWalkHal::XWalkGpioCallbacks callbacks()
{
    return {&configure, &read, &write, &interrupt, &cancelInterrupt};
}

/** @brief Verifies pin mapping, automatic direction changes, and digital I/O. */
void testDigitalIo()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    xwalk::hal::XWalkGpio gpio(&backend, callbackSet, "D4");
    assert(gpio.pin() == 23U);
    assert(gpio.name() == "GPIO23");
    assert(backend.configureCount == 1U);
    assert(backend.mode == XWalkHal::XWalkGpioMode::Output);

    backend.readValue = true;
    const XWalkHal::boolean inputValue = gpio.read();
    assert(inputValue);
    assert(gpio.mode() == XWalkHal::XWalkGpioMode::Input);
    assert(backend.configureCount == 2U);

    const XWalkHal::boolean outputValue = gpio.off();
    assert(!outputValue);
    assert(gpio.mode() == XWalkHal::XWalkGpioMode::Output);
    assert(backend.configureCount == 3U);
    assert(backend.writeCount == 1U);
    assert(!backend.writeValue);
}

/** @brief Verifies logical polarity inversion and duplicate Robot HAT aliases. */
void testPolarityAndAliases()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    xwalk::hal::XWalkGpio gpio(&backend, callbackSet, "USER", XWalkHal::XWalkGpioMode::Input,
        XWalkHal::XWalkGpioPull::Up, false);
    assert(gpio.pin() == 25U);
    assert(backend.pull == XWalkHal::XWalkGpioPull::Up);
    backend.readValue = true;
    const XWalkHal::boolean logicalInput = gpio.read();
    assert(!logicalInput);
    const XWalkHal::boolean logicalOutput = gpio.on();
    assert(logicalOutput);
    assert(!backend.writeValue);

    TestBackend aliasBackend;
    xwalk::hal::XWalkGpio alias(&aliasBackend, callbackSet, "D7");
    assert(alias.pin() == 4U);
}

/** @brief Verifies every named entry copied from the Python Robot HAT pin dictionary. */
void testNamedPinMap()
{
    const XWalkHal::fixedarray<PinMapping, 26U> mappings{{
        {"D0", 17U}, {"D1", 4U}, {"D2", 27U}, {"D3", 22U}, {"D4", 23U}, {"D5", 24U},
        {"D6", 25U}, {"D7", 4U}, {"D8", 5U}, {"D9", 6U}, {"D10", 12U}, {"D11", 13U},
        {"D12", 19U}, {"D13", 16U}, {"D14", 26U}, {"D15", 20U}, {"D16", 21U}, {"SW", 25U},
        {"USER", 25U}, {"LED", 26U}, {"BOARD_TYPE", 12U}, {"RST", 16U}, {"BLEINT", 13U},
        {"BLERST", 20U}, {"MCURST", 5U}, {"CE", 8U}
    }};
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    for (const PinMapping& mapping : mappings)
    {
        TestBackend backend;
        xwalk::hal::XWalkGpio gpio(&backend, callbackSet, mapping.name);
        assert(gpio.pin() == mapping.pin);
    }
}

/** @brief Verifies interrupt forwarding, simulated dispatch, and cancellation. */
void testInterrupt()
{
    TestBackend backend;
    HandlerData handlerData;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    xwalk::hal::XWalkGpio gpio(&backend, callbackSet, "LED");
    gpio.irq(&handlerData, &handleInterrupt, XWalkHal::XWalkGpioEdge::Rising, 250U);
    assert(backend.interruptCount == 1U);
    assert(backend.edge == XWalkHal::XWalkGpioEdge::Rising);
    assert(backend.debounceMs == 250U);
    backend.handler(backend.handlerContext);
    assert(handlerData.count == 1U);
    gpio.deinit();
    assert(backend.cancelCount == 1U);
}

/** @brief Verifies rejection of unsupported pins, names, callbacks, and handlers. */
void testValidation()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::XWalkGpio gpio(&backend, callbackSet, 7U);
    });

    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::XWalkGpio gpio(&backend, callbackSet, "UNKNOWN");
    });

    XWalkHal::XWalkGpioCallbacks invalidCallbacks = callbackSet;
    invalidCallbacks.read = nullptr;
    xwalk::hal::test::expectFailure([&]()
    {
        xwalk::hal::XWalkGpio gpio(&backend, invalidCallbacks, "D0");
    });

    xwalk::hal::XWalkGpio gpio(&backend, callbackSet, "D0");
    xwalk::hal::test::expectFailure([&]()
    {
        gpio.irq(nullptr, nullptr, XWalkHal::XWalkGpioEdge::Both);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all xWalk GPIO host-test scenarios.
 *
 * @return
 * Zero when every assertion passes. A failed assertion terminates the process.
 */
XWalkHal::int32 main()
{
    testDigitalIo();
    testPolarityAndAliases();
    testNamedPinMap();
    testInterrupt();
    testValidation();
    return 0;
}
