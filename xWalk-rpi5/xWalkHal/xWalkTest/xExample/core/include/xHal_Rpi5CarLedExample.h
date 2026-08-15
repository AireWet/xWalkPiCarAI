/******************************************************************************
 * @file        xHal_Rpi5CarLedExample.h
 * @brief       Declares the ported Robot HAT LED example flow.
 *
 * @details
 * Defines injected LED, timing, and reporting operations so the original
 * example can run against either an in-memory host or physical Linux adapter.
 *
 * @project     xWalk Firmware
 * @module      xExample
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_LED_EXAMPLE_H
#define XHAL_RPI5CAR_LED_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains host-testable behavior ported from upstream examples.
 */
namespace xwalk::hal::example
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief LED operation receiving the shared non-owning example context. */
    using ledexampleoperationcallback = void (*)(contextpointer context);

    /**
     * @brief Background blink operation used by the example.
     *
     * @param[in,out] context Non-owning example context.
     * @param[in] cycleCount Complete on/off cycles per repeated sequence.
     * @param[in] toggleDelaySeconds Delay between transitions in seconds.
     * @param[in] pauseSeconds Inactive delay after each repeated sequence.
     */
    using ledexampleblinkcallback = void (*)(contextpointer context,
                                             uint32 cycleCount,
                                             float64 toggleDelaySeconds,
                                             float64 pauseSeconds);

    /**
     * @brief Wait operation used between LED commands.
     *
     * @param[in,out] context Non-owning example context.
     * @param[in] durationMilliseconds Requested duration in milliseconds.
     */
    using ledexamplewaitcallback = void (*)(contextpointer context, uint32 durationMilliseconds);

    /**
     * @brief Status-reporting operation used before each example action.
     *
     * @param[in,out] context Non-owning example context.
     * @param[in] message Non-empty source-compatible status message.
     */
    using ledexamplereportcallback = void (*)(contextpointer context, stringview message);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Complete injected operation table required by the LED example. */
    struct XWalkLedExampleCallbacks
    {
            /** @brief Activates the LED. */
            ledexampleoperationcallback on{nullptr};
            /** @brief Deactivates the LED. */
            ledexampleoperationcallback off{nullptr};
            /** @brief Starts one background blink configuration. */
            ledexampleblinkcallback blink{nullptr};
            /** @brief Stops blinking and closes the LED. */
            ledexampleoperationcallback close{nullptr};
            /** @brief Waits between example actions. */
            ledexamplewaitcallback wait{nullptr};
            /** @brief Reports source-compatible progress text. */
            ledexamplereportcallback report{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Runs the bounded LED example through an injected operation table. */
    class XWalkLedExample
    {
        private:
            /** @brief Non-owning context forwarded to every operation. */
            contextpointer callbackContext;
            /** @brief Complete validated operation table copied during construction. */
            XWalkLedExampleCallbacks callbacks;

        public:
            /**
             * @brief Binds the complete LED example operation table.
             *
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Complete table containing six non-null callbacks.
             *
             * @throws std::invalid_argument If any callback is null.
             */
            XWalkLedExample(contextpointer context, const XWalkLedExampleCallbacks& exampleCallbacks);

            /** @brief Prevents copying of non-owning callback bindings. */
            XWalkLedExample(const XWalkLedExample&) = delete;
            /** @brief Prevents moving of non-owning callback bindings. */
            XWalkLedExample(XWalkLedExample&&) = delete;
            /** @brief Prevents copy assignment of non-owning callback bindings. */
            XWalkLedExample& operator=(const XWalkLedExample&) = delete;
            /** @brief Prevents move assignment of non-owning callback bindings. */
            XWalkLedExample& operator=(XWalkLedExample&&) = delete;

            /**
             * @brief Runs the complete bounded LED example.
             *
             * @post The close callback has been attempted and normal completion leaves
             * the physical adapter inactive.
             */
            void run();
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_LED_EXAMPLE_H */
