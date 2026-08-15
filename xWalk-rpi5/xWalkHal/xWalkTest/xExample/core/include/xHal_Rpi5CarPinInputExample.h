/******************************************************************************
 * @file        xHal_Rpi5CarPinInputExample.h
 * @brief       Declares the bounded Robot HAT pin-input example flow.
 *
 * @details
 * Defines injected input, timing, and reporting operations so the source
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

#ifndef XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_H
#define XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Highest bounded sample count accepted by the input example. */
#define XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_MAXIMUM_SAMPLES 36'000U

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

    /** @brief Reads one logical value from the configured input pin. */
    using pininputexamplereadcallback = boolean (*)(contextpointer context);

    /** @brief Waits for one bounded duration between pin samples. */
    using pininputexamplewaitcallback = void (*)(contextpointer context, uint32 durationMilliseconds);

    /** @brief Reports one source-compatible logical pin value. */
    using pininputexamplereportcallback = void (*)(contextpointer context, boolean value);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Complete injected operation table required by the input example. */
    struct XWalkPinInputExampleCallbacks
    {
            /** @brief Reads one logical pin value. */
            pininputexamplereadcallback read{nullptr};
            /** @brief Waits between samples. */
            pininputexamplewaitcallback wait{nullptr};
            /** @brief Reports each sampled value. */
            pininputexamplereportcallback report{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Runs bounded pin sampling through an injected operation table. */
    class XWalkPinInputExample final
    {
        private:
            /** @brief Non-owning context forwarded to every operation. */
            contextpointer callbackContext;
            /** @brief Complete validated operation table copied during construction. */
            XWalkPinInputExampleCallbacks callbacks;

        public:
            /**
             * @brief Binds the complete pin-input example operation table.
             * @param[in,out] context Non-owning context forwarded to every callback.
             * @param[in] exampleCallbacks Table containing three non-null callbacks.
             * @throws std::invalid_argument If any callback is null.
             */
            XWalkPinInputExample(contextpointer context, const XWalkPinInputExampleCallbacks& exampleCallbacks);

            /** @brief Prevents copying of non-owning callback bindings. */
            XWalkPinInputExample(const XWalkPinInputExample&) = delete;
            /** @brief Prevents moving of non-owning callback bindings. */
            XWalkPinInputExample(XWalkPinInputExample&&) = delete;
            /** @brief Prevents copy assignment of non-owning callback bindings. */
            XWalkPinInputExample& operator=(const XWalkPinInputExample&) = delete;
            /** @brief Prevents move assignment of non-owning callback bindings. */
            XWalkPinInputExample& operator=(XWalkPinInputExample&&) = delete;

            /**
             * @brief Reads, reports, and waits for each requested sample.
             * @param[in] sampleCount Sample count in the inclusive range 1 through 36,000.
             * @throws std::out_of_range If `sampleCount` is outside its range.
             */
            void run(uint32 sampleCount);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_H */
