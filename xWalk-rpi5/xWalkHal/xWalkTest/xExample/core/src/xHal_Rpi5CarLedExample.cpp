/******************************************************************************
 * @file        xHal_Rpi5CarLedExample.cpp
 * @brief       Implements the ported Robot HAT LED example flow.
 *
 * @details
 * Preserves the source status messages, LED commands, blink arguments, waits,
 * and final close while guaranteeing a close attempt after callback failures.
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

#include "xHal_Rpi5CarLedExample.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

    /**
     * @brief Binds and validates all LED example operations.
     *
     * @param[in,out] context Non-owning callback context.
     * @param[in] exampleCallbacks Complete operation table.
     *
     * @throws std::invalid_argument If any callback is null.
     */
    XWalkLedExample::XWalkLedExample(contextpointer context, const XWalkLedExampleCallbacks& exampleCallbacks)
        : callbackContext(context), callbacks(exampleCallbacks)
    {
        if ((callbacks.on == nullptr) || (callbacks.off == nullptr) || (callbacks.blink == nullptr) ||
            (callbacks.close == nullptr) || (callbacks.wait == nullptr) || (callbacks.report == nullptr))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "LED example requires a complete callback table");
        }
    }

    /**
     * @brief Runs the complete source-compatible LED example.
     *
     * @post The close callback is attempted on both normal and exceptional exits.
     */
    void XWalkLedExample::run()
    {
        try
        {
            callbacks.report(callbackContext, "On");
            callbacks.on(callbackContext);
            callbacks.wait(callbackContext, 2'000U);

            callbacks.report(callbackContext, "Off");
            callbacks.off(callbackContext);
            callbacks.wait(callbackContext, 2'000U);

            callbacks.report(callbackContext, "Blink delay 1 second");
            callbacks.blink(callbackContext, 1U, 1.0, 0.0);
            callbacks.wait(callbackContext, 5'000U);

            callbacks.report(callbackContext, "Blink 3 times delay 0.1 second pause 0.5 second");
            callbacks.blink(callbackContext, 3U, 0.1, 0.5);
            callbacks.wait(callbackContext, 5'000U);

            callbacks.report(callbackContext, "Blink 2 times delay 0.2 second pause 1 second");
            callbacks.blink(callbackContext, 2U, 0.2, 1.0);
            callbacks.wait(callbackContext, 5'000U);

            callbacks.report(callbackContext, "Done");
            callbacks.close(callbackContext);
        }
        catch (...)
        {
            try
            {
                callbacks.close(callbackContext);
            }
            catch (...)
            {
            }
            throw;
        }
    }

} /* namespace xwalk::hal::example */
