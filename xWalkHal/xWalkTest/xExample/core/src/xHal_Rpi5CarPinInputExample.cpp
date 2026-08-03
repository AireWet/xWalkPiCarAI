/******************************************************************************
 * @file        xHal_Rpi5CarPinInputExample.cpp
 * @brief       Implements the bounded Robot HAT pin-input example flow.
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

#include "xHal_Rpi5CarPinInputExample.h"

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
 * @brief Binds and validates all pin-input operations.
 * @param[in,out] context Non-owning callback context.
 * @param[in] exampleCallbacks Complete operation table.
 * @throws std::invalid_argument If any callback is null.
 */
XWalkPinInputExample::XWalkPinInputExample(contextpointer context,
    const XWalkPinInputExampleCallbacks& exampleCallbacks):
    callbackContext(context), callbacks(exampleCallbacks)
{
    if ((callbacks.read == nullptr) || (callbacks.wait == nullptr) ||
        (callbacks.report == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Pin-input example requires a complete callback table");
    }
}

/**
 * @brief Runs the source-compatible bounded sampling loop.
 * @param[in] sampleCount Sample count from one through 36,000.
 * @throws std::out_of_range If `sampleCount` is outside its range.
 */
void XWalkPinInputExample::run(uint32 sampleCount)
{
    if ((sampleCount == 0U) ||
        (sampleCount > XHAL_RPI5CAR_PIN_INPUT_EXAMPLE_MAXIMUM_SAMPLES))
    {
        XHAL_THROW_OUT_OF_RANGE("Pin-input sample count is outside its range");
    }

    for (uint32 sampleIndex = 0U; sampleIndex < sampleCount; ++sampleIndex)
    {
        callbacks.report(callbackContext, callbacks.read(callbackContext));
        callbacks.wait(callbackContext, 100U);
    }
}

} /* namespace xwalk::hal::example */
