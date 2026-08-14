/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicExample.h
 * @brief       Declares the bounded Robot HAT ultrasonic example flow.
 *
 * @details
 * Defines injected ranging, timing, and reporting operations so the source
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

#ifndef XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_H
#define XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Highest bounded sample count accepted by the ranging example. */
#define XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_MAXIMUM_SAMPLES 18'000U

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

/** @brief Reads one distance in centimeters from the configured sensor. */
using ultrasonicexamplereadcallback = float64 (*)(contextpointer context);

/** @brief Waits for one bounded duration between distance samples. */
using ultrasonicexamplewaitcallback = void (*)(contextpointer context,
    uint32 durationMilliseconds);

/** @brief Reports one source-compatible distance in centimeters. */
using ultrasonicexamplereportcallback = void (*)(contextpointer context,
    float64 distanceCentimeters);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Complete injected operation table required by the ranging example. */
struct XWalkUltrasonicExampleCallbacks
{
    /** @brief Reads one distance in centimeters. */
    ultrasonicexamplereadcallback read{nullptr};
    /** @brief Waits between samples. */
    ultrasonicexamplewaitcallback wait{nullptr};
    /** @brief Reports each sampled value. */
    ultrasonicexamplereportcallback report{nullptr};
};

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Runs bounded distance sampling through an injected operation table. */
class XWalkUltrasonicExample final
{
private:

    /** @brief Non-owning context forwarded to every operation. */
    contextpointer callbackContext;
    /** @brief Complete validated operation table copied during construction. */
    XWalkUltrasonicExampleCallbacks callbacks;

public:

    /**
     * @brief Binds the complete ultrasonic example operation table.
     * @param[in,out] context Non-owning context forwarded to every callback.
     * @param[in] exampleCallbacks Table containing three non-null callbacks.
     * @throws std::invalid_argument If any callback is null.
     */
    XWalkUltrasonicExample(contextpointer context,
        const XWalkUltrasonicExampleCallbacks& exampleCallbacks);

    /** @brief Prevents copying of non-owning callback bindings. */
    XWalkUltrasonicExample(const XWalkUltrasonicExample&) = delete;
    /** @brief Prevents moving of non-owning callback bindings. */
    XWalkUltrasonicExample(XWalkUltrasonicExample&&) = delete;
    /** @brief Prevents copy assignment of non-owning callback bindings. */
    XWalkUltrasonicExample& operator=(const XWalkUltrasonicExample&) = delete;
    /** @brief Prevents move assignment of non-owning callback bindings. */
    XWalkUltrasonicExample& operator=(XWalkUltrasonicExample&&) = delete;

    /**
     * @brief Reads, reports, and waits for each requested sample.
     * @param[in] sampleCount Sample count in the inclusive range 1 through 18,000.
     * @throws std::out_of_range If `sampleCount` is outside its range.
     */
    void run(uint32 sampleCount);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_H */
