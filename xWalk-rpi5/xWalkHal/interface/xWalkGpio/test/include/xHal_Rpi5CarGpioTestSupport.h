/******************************************************************************
 * @file        xHal_Rpi5CarGpioTestSupport.h
 * @brief       Declares reusable GPIO host-test callback support.
 *
 * @details
 * Defines the test-only state records and callback contracts shared by GPIO
 * test scenarios while keeping their implementations in a separate source.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Host Test
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_GPIO_TEST_SUPPORT_H
#define XHAL_RPI5CAR_GPIO_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test::gpio
 * @brief Contains reusable callback support for GPIO host tests.
 */
namespace xwalk::hal::test::gpio
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Records interrupt callback operations without starting a worker thread. */
    struct InterruptBackend
    {
            /** @brief Number of interrupt registrations observed by the test backend. */
            XWalkHal::size registrationCount{};

            /** @brief Number of interrupt cancellations observed by the test backend. */
            XWalkHal::size cancellationCount{};

            /** @brief Nullable non-owning context retained until cancellation. */
            XWalkHal::contextpointer handlerContext{nullptr};

            /** @brief Nullable application handler retained until cancellation. */
            XWalkHal::gpiointerrupthandler handler{nullptr};
    };

    /** @brief Counts application interrupt-handler invocations. */
    struct HandlerData
    {
            /** @brief Number of application handler invocations observed by the test. */
            XWalkHal::size count{};
    };

    /** @brief Associates one Robot HAT pin name with its Linux line offset. */
    struct PinMapping
    {
            /** @brief Non-owning null-terminated Robot HAT pin name. */
            XWalkHal::cstring name;

            /** @brief Expected Linux GPIO line offset. */
            XWalkHal::uint8 pin;
    };

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Accepts one test GPIO configuration without changing backend state.
     * @param[in] context Optional non-owning backend context; unused.
     * @param[in] pin GPIO line offset; unused.
     * @param[in] mode Requested GPIO mode; unused.
     * @param[in] pull Requested GPIO pull configuration; unused.
     * @param[in] initialValue Requested initial logical output; unused.
     */
    void configure(XWalkHal::contextpointer context,
                   XWalkHal::uint8 pin,
                   XWalkHal::XWalkGpioMode mode,
                   XWalkHal::XWalkGpioPull pull,
                   XWalkHal::boolean initialValue);

    /**
     * @brief Returns the fixed inactive input used by callback-only GPIO tests.
     * @param[in] context Optional non-owning backend context; unused.
     * @param[in] pin GPIO line offset; unused.
     * @return Always `false`.
     */
    XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin);

    /**
     * @brief Accepts one test GPIO output without changing backend state.
     * @param[in] context Optional non-owning backend context; unused.
     * @param[in] pin GPIO line offset; unused.
     * @param[in] value Requested logical output; unused.
     */
    void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value);

    /**
     * @brief Records one application interrupt registration.
     * @param[in,out] context Non-null `InterruptBackend` test context.
     * @param[in] pin GPIO line offset; unused by the test backend.
     * @param[in] edge Requested interrupt edge; unused by the test backend.
     * @param[in] debounceMs Requested debounce interval in milliseconds; unused.
     * @param[in] handlerContext Nullable non-owning application handler context.
     * @param[in] handler Nullable application interrupt handler retained for the test.
     */
    void interrupt(XWalkHal::contextpointer context,
                   XWalkHal::uint8 pin,
                   XWalkHal::XWalkGpioEdge edge,
                   XWalkHal::uint32 debounceMs,
                   XWalkHal::contextpointer handlerContext,
                   XWalkHal::gpiointerrupthandler handler);

    /**
     * @brief Records cancellation and clears the retained application handler.
     * @param[in,out] context Non-null `InterruptBackend` test context.
     * @param[in] pin GPIO line offset; unused by the test backend.
     */
    void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin);

    /**
     * @brief Records one application interrupt-handler invocation.
     * @param[in,out] context Non-null `HandlerData` application context.
     */
    void handleInterrupt(XWalkHal::contextpointer context);

    /**
     * @brief Creates the complete callback table for callback-only GPIO tests.
     * @return Callback table bound to this test-support implementation.
     */
    XWalkHal::XWalkGpioCallbacks interruptCallbacks();

} /* namespace xwalk::hal::test::gpio */

#endif /* XHAL_RPI5CAR_GPIO_TEST_SUPPORT_H */
