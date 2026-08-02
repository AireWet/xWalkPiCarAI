/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicTest.cpp
 * @brief       Verifies xWalk ultrasonic behavior using simulated GPIO callbacks.
 *
 * @details
 * Checks GPIO configuration, trigger sequencing, echo conversion, timeout
 * retries, invalid pulses, zero-attempt behavior, and timeout access.
 *
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic Host Test
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

#include "xHal_Rpi5CarUltrasonic.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains simulated GPIO state and host-test scenarios.
 */
namespace
{

/******************************************************************************
 * Enumeration declarations
 ******************************************************************************/

/** @brief Selects the echo waveform returned by the simulated input pin. */
enum class EchoBehavior : XWalkHal::uint8
{
    Pulse = 0U, /**< Returns one measurable high pulse. */
    Timeout = 1U, /**< Remains low until the acquisition timeout. */
    Invalid = 2U, /**< Changes state without a measurable high pulse. */
    TimeoutThenPulse = 3U /**< Times out once and returns a pulse on the next trigger. */
};

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Stores simulated trigger traffic and echo waveform state. */
struct TestBackend
{
    EchoBehavior behavior{EchoBehavior::Pulse}; /**< Echo waveform selected by the scenario. */
    XWalkHal::uint32 triggerCount{}; /**< Number of observed active trigger levels. */
    XWalkHal::uint32 echoReadCount{}; /**< Reads made since the latest trigger. */
    XWalkHal::XWalkGpioMode triggerMode{XWalkHal::XWalkGpioMode::Input};
    XWalkHal::XWalkGpioMode echoMode{XWalkHal::XWalkGpioMode::Output};
    XWalkHal::XWalkGpioPull echoPull{XWalkHal::XWalkGpioPull::None};
    XWalkHal::bytevector triggerLevels; /**< Ordered logical trigger writes as bytes. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Records simulated GPIO configuration.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line being configured.
 *
 * @param[in] mode
 * Requested direction.
 *
 * @param[in] pull
 * Requested internal bias.
 *
 * @param[in] initialValue
 * Initial output level, unused by this simulation.
 */
void configure(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
    XWalkHal::XWalkGpioMode mode, XWalkHal::XWalkGpioPull pull,
    XWalkHal::boolean initialValue)
{
    static_cast<void>(initialValue);
    TestBackend& backend = *static_cast<TestBackend*>(context);
    if (pin == 27U)
    {
        backend.triggerMode = mode;
    }
    if (pin == 22U)
    {
        backend.echoMode = mode;
        backend.echoPull = pull;
    }
}

/**
 * @brief Returns the selected simulated echo waveform.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * Echo GPIO line; ignored after interface validation.
 *
 * @return
 * Next physical echo level.
 */
XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
{
    static_cast<void>(pin);
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.echoReadCount;

    EchoBehavior activeBehavior = backend.behavior;
    if ((activeBehavior == EchoBehavior::TimeoutThenPulse) && (backend.triggerCount > 1U))
    {
        activeBehavior = EchoBehavior::Pulse;
    }
    if ((activeBehavior == EchoBehavior::Timeout) ||
        (activeBehavior == EchoBehavior::TimeoutThenPulse))
    {
        return false;
    }
    if (activeBehavior == EchoBehavior::Invalid)
    {
        return backend.echoReadCount == 1U;
    }
    if (backend.echoReadCount == 3U)
    {
        XWalkHal::common::sleepMicroseconds(1'000U);
    }
    return (backend.echoReadCount == 2U) || (backend.echoReadCount == 3U);
}

/**
 * @brief Records simulated trigger writes.
 *
 * @param[in,out] context
 * Non-null pointer to `TestBackend`.
 *
 * @param[in] pin
 * GPIO line being driven.
 *
 * @param[in] value
 * Physical output level.
 */
void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    if (pin == 27U)
    {
        backend.triggerLevels.push_back(value ? 1U : 0U);
        if (value)
        {
            ++backend.triggerCount;
            backend.echoReadCount = 0U;
        }
    }
}

/**
 * @brief Accepts an unused simulated interrupt registration.
 *
 * @param[in,out] context
 * Opaque backend context.
 *
 * @param[in] pin
 * GPIO line offset.
 *
 * @param[in] edge
 * Requested edge selection.
 *
 * @param[in] debounceMs
 * Requested debounce duration in milliseconds.
 *
 * @param[in,out] handlerContext
 * Opaque application callback context.
 *
 * @param[in] handler
 * Application callback function.
 */
void interrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin,
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
 * @param[in,out] context
 * Opaque backend context.
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
 * @brief Creates the callback set shared by the simulated GPIO objects.
 *
 * @return
 * Complete GPIO callback binding.
 */
XWalkHal::XWalkGpioCallbacks callbacks()
{
    return {&configure, &read, &write, &interrupt, &cancelInterrupt};
}

/** @brief Verifies configuration, trigger sequencing, and distance conversion. */
void testDistanceMeasurement()
{
    TestBackend backend;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    XWalkHal::XWalkGpio trigger(&backend, callbackSet, "D2");
    XWalkHal::XWalkGpio echo(&backend, callbackSet, "D3");
    XWalkHal::XWalkUltrasonic ultrasonic(trigger, echo);

    assert(backend.triggerMode == XWalkHal::XWalkGpioMode::Output);
    assert(backend.echoMode == XWalkHal::XWalkGpioMode::Input);
    assert(backend.echoPull == XWalkHal::XWalkGpioPull::Down);
    assert(ultrasonic.timeoutMicroseconds() == 20'000U);

    const XWalkHal::float64 distanceCentimeters = ultrasonic.read(1U);
    assert(distanceCentimeters > 10.0);
    assert(distanceCentimeters < 50.0);
    assert(backend.triggerLevels == XWalkHal::bytevector({0U, 1U, 0U}));
}

/** @brief Verifies that public reads retry only timeout results. */
void testRetryAndStatusResults()
{
    TestBackend backend;
    backend.behavior = EchoBehavior::TimeoutThenPulse;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    XWalkHal::XWalkGpio trigger(&backend, callbackSet, "D2");
    XWalkHal::XWalkGpio echo(&backend, callbackSet, "D3");
    XWalkHal::XWalkUltrasonic ultrasonic(trigger, echo, 5'000U);

    const XWalkHal::float64 distanceCentimeters = ultrasonic.read(2U);
    assert(distanceCentimeters > 10.0);
    assert(backend.triggerCount == 2U);

    backend.behavior = EchoBehavior::Invalid;
    backend.triggerCount = 0U;
    const XWalkHal::float64 invalidResult = ultrasonic.read(3U);
    assert(invalidResult == XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM);
    assert(backend.triggerCount == 1U);
}

/** @brief Verifies complete timeout and zero-attempt behavior. */
void testTimeoutResults()
{
    TestBackend backend;
    backend.behavior = EchoBehavior::Timeout;
    const XWalkHal::XWalkGpioCallbacks callbackSet = callbacks();
    XWalkHal::XWalkGpio trigger(&backend, callbackSet, "D2");
    XWalkHal::XWalkGpio echo(&backend, callbackSet, "D3");
    XWalkHal::XWalkUltrasonic ultrasonic(trigger, echo, 200U);

    const XWalkHal::float64 timeoutResult = ultrasonic.read(2U);
    assert(timeoutResult == XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM);
    assert(backend.triggerCount == 2U);
    backend.triggerCount = 0U;
    const XWalkHal::float64 zeroAttemptResult = ultrasonic.read(0U);
    assert(zeroAttemptResult == XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM);
    assert(backend.triggerCount == 0U);
    ultrasonic.close();
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every xWalk ultrasonic host-test scenario.
 *
 * @return
 * Zero when every assertion passes.
 */
XWalkHal::int32 main()
{
    testDistanceMeasurement();
    testRetryAndStatusResults();
    testTimeoutResults();
    return 0;
}
