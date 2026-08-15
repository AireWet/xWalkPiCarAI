/******************************************************************************
 * @file        xHal_Rpi5CarGpioTestSupport.cpp
 * @brief       Implements reusable GPIO host-test callbacks.
 *
 * @details
 * Supplies device-free GPIO callbacks that retain only the interrupt state
 * required by host assertions.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpioTestSupport.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test::gpio
 * @brief Contains reusable callback support for GPIO host tests.
 */
namespace xwalk::hal::test::gpio
{

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
                   XWalkHal::boolean initialValue)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
        static_cast<void>(initialValue);
    }

    /**
     * @brief Returns the fixed inactive input used by callback-only GPIO tests.
     * @param[in] context Optional non-owning backend context; unused.
     * @param[in] pin GPIO line offset; unused.
     * @return Always `false`.
     */
    XWalkHal::boolean read(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        return false;
    }

    /**
     * @brief Accepts one test GPIO output without changing backend state.
     * @param[in] context Optional non-owning backend context; unused.
     * @param[in] pin GPIO line offset; unused.
     * @param[in] value Requested logical output; unused.
     */
    void write(XWalkHal::contextpointer context, XWalkHal::uint8 pin, XWalkHal::boolean value)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(value);
    }

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
                   XWalkHal::gpiointerrupthandler handler)
    {
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        auto& backend = *static_cast<InterruptBackend*>(context);
        ++backend.registrationCount;
        backend.handlerContext = handlerContext;
        backend.handler = handler;
    }

    /**
     * @brief Records cancellation and clears the retained application handler.
     * @param[in,out] context Non-null `InterruptBackend` test context.
     * @param[in] pin GPIO line offset; unused by the test backend.
     */
    void cancelInterrupt(XWalkHal::contextpointer context, XWalkHal::uint8 pin)
    {
        static_cast<void>(pin);
        auto& backend = *static_cast<InterruptBackend*>(context);
        ++backend.cancellationCount;
        backend.handlerContext = nullptr;
        backend.handler = nullptr;
    }

    /**
     * @brief Records one application interrupt-handler invocation.
     * @param[in,out] context Non-null `HandlerData` application context.
     */
    void handleInterrupt(XWalkHal::contextpointer context)
    {
        auto& data = *static_cast<HandlerData*>(context);
        ++data.count;
    }

    /**
     * @brief Creates the complete callback table for callback-only GPIO tests.
     * @return Callback table bound to this test-support implementation.
     */
    XWalkHal::XWalkGpioCallbacks interruptCallbacks()
    {
        return {&configure, &read, &write, &interrupt, &cancelInterrupt};
    }

} /* namespace xwalk::hal::test::gpio */
