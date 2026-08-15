/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicExample.cpp
 * @brief       Implements the bounded Robot HAT ultrasonic example flow.
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

#include "xHal_Rpi5CarUltrasonicExample.h"

#include "xHal_Rpi5CarTrace.h"
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
     * @brief Binds and validates all ultrasonic operations.
     * @param[in,out] context Non-owning callback context.
     * @param[in] exampleCallbacks Complete operation table.
     * @throws std::invalid_argument If any callback is null.
     */
    XWalkUltrasonicExample::XWalkUltrasonicExample(contextpointer context,
                                                   const XWalkUltrasonicExampleCallbacks& exampleCallbacks)
        : callbackContext(context), callbacks(exampleCallbacks)
    {
        if ((callbacks.read == nullptr) || (callbacks.wait == nullptr) || (callbacks.report == nullptr))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Ultrasonic example requires a complete callback table");
        }
    }

    /**
     * @brief Runs the source-compatible bounded sampling loop.
     * @param[in] sampleCount Sample count from one through 18,000.
     * @throws std::out_of_range If `sampleCount` is outside its range.
     */
    void XWalkUltrasonicExample::run(uint32 sampleCount)
    {
        if ((sampleCount == 0U) || (sampleCount > XHAL_RPI5CAR_ULTRASONIC_EXAMPLE_MAXIMUM_SAMPLES))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Ultrasonic sample count is outside its range");
        }

        for (uint32 sampleIndex = 0U; sampleIndex < sampleCount; ++sampleIndex)
        {
            callbacks.report(callbackContext, callbacks.read(callbackContext));
            callbacks.wait(callbackContext, 200U);
        }
    }

} /* namespace xwalk::hal::example */
