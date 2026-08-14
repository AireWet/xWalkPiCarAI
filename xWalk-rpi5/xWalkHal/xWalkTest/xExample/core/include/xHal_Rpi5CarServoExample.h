/******************************************************************************
 * @file        xHal_Rpi5CarServoExample.h
 * @brief       Declares the bounded Robot HAT servo sweep example.
 *
 * @details
 * Defines injected angle, timing, and reporting operations for host and Linux
 * execution while preserving the upstream channel-one sweep sequence.
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

#ifndef XHAL_RPI5CAR_SERVO_EXAMPLE_H
#define XHAL_RPI5CAR_SERVO_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Highest bounded sweep count accepted by the servo example. */
#define XHAL_RPI5CAR_SERVO_EXAMPLE_MAXIMUM_CYCLES 100U

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

/** @brief Commands one angle in degrees on servo channel one. */
using servoexamplesetanglecallback = void (*)(contextpointer context,
    float64 angleDegrees);

/** @brief Waits for one bounded duration during the sweep. */
using servoexamplewaitcallback = void (*)(contextpointer context,
    uint32 durationMilliseconds);

/** @brief Reports one commanded angle using source-compatible output. */
using servoexamplereportcallback = void (*)(contextpointer context,
    int32 angleDegrees);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Complete injected operation table required by the servo example. */
struct XWalkServoExampleCallbacks
{
    servoexamplesetanglecallback setAngle{nullptr};
    servoexamplewaitcallback wait{nullptr};
    servoexamplereportcallback report{nullptr};
};

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Runs bounded channel-one sweeps through injected operations. */
class XWalkServoExample final
{
private:

    contextpointer callbackContext;
    XWalkServoExampleCallbacks callbacks;

public:

    /**
     * @brief Binds the complete servo example operation table.
     * @param[in,out] context Non-owning context forwarded to every callback.
     * @param[in] exampleCallbacks Table containing three non-null callbacks.
     * @throws std::invalid_argument If any callback is null.
     */
    XWalkServoExample(contextpointer context,
        const XWalkServoExampleCallbacks& exampleCallbacks);

    /** @brief Prevents copying of non-owning callback bindings. */
    XWalkServoExample(const XWalkServoExample&) = delete;
    /** @brief Prevents moving of non-owning callback bindings. */
    XWalkServoExample(XWalkServoExample&&) = delete;
    /** @brief Prevents copy assignment of non-owning callback bindings. */
    XWalkServoExample& operator=(const XWalkServoExample&) = delete;
    /** @brief Prevents move assignment of non-owning callback bindings. */
    XWalkServoExample& operator=(XWalkServoExample&&) = delete;

    /**
     * @brief Runs the exact ascending and descending source sweep.
     * @param[in] cycleCount Complete cycles in the inclusive range 1 through 100.
     * @throws std::out_of_range If `cycleCount` is outside its range.
     */
    void run(uint32 cycleCount);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_SERVO_EXAMPLE_H */
