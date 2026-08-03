/******************************************************************************
 * @file        xHal_Rpi5CarServoExample.cpp
 * @brief       Implements the bounded Robot HAT servo sweep example.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarServoExample.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains host-testable behavior ported from upstream examples.
 */
namespace xwalk::hal::example
{

/**
 * @brief Binds and validates all servo example operations.
 * @param[in,out] context Non-owning callback context.
 * @param[in] exampleCallbacks Complete operation table.
 * @throws std::invalid_argument If any callback is null.
 */
XWalkServoExample::XWalkServoExample(contextpointer context,
    const XWalkServoExampleCallbacks& exampleCallbacks):
    callbackContext(context), callbacks(exampleCallbacks)
{
    if ((callbacks.setAngle == nullptr) || (callbacks.wait == nullptr) ||
        (callbacks.report == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Servo example requires a complete callback table");
    }
}

/**
 * @brief Runs the exact bounded ascending and descending sweep.
 * @param[in] cycleCount Complete cycles from one through 100.
 * @throws std::out_of_range If `cycleCount` is outside its range.
 */
void XWalkServoExample::run(uint32 cycleCount)
{
    if ((cycleCount == 0U) ||
        (cycleCount > XHAL_RPI5CAR_SERVO_EXAMPLE_MAXIMUM_CYCLES))
    {
        XHAL_THROW_OUT_OF_RANGE("Servo example cycle count is outside its range");
    }

    for (uint32 cycle = 0U; cycle < cycleCount; ++cycle)
    {
        for (int32 angle = -90; angle < 90; ++angle)
        {
            callbacks.setAngle(callbackContext, static_cast<float64>(angle));
            callbacks.wait(callbackContext, 10U);
            callbacks.report(callbackContext, angle);
        }
        callbacks.wait(callbackContext, 1'000U);

        for (int32 angle = 90; angle > -90; --angle)
        {
            callbacks.setAngle(callbackContext, static_cast<float64>(angle));
            callbacks.wait(callbackContext, 10U);
            callbacks.report(callbackContext, angle);
        }
        callbacks.wait(callbackContext, 1'000U);
    }
}

} /* namespace xwalk::hal::example */
