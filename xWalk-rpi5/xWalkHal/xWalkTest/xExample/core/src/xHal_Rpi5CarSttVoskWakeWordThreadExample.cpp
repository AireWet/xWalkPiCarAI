/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordThreadExample.cpp
 * @brief       Implements the bounded threaded Vosk wake-word example.
 *
 * @details
 * Coordinates source-compatible start, three-second polling, detection, and
 * worker-stop behavior through a validated injected operation table.
 *
 * @project     xWalk Firmware
 * @module      xExample
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordThreadExample.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::example
{

    /** @brief Binds and validates every wake-word worker operation. */
    XWalkSttVoskWakeWordThreadExample::XWalkSttVoskWakeWordThreadExample(
        contextpointer context, const XWalkSttVoskWakeWordThreadExampleCallbacks& exampleCallbacks)
        : callbackContext(context), callbacks(exampleCallbacks)
    {
        if ((callbacks.startListening == nullptr) || (callbacks.isWaked == nullptr) ||
            (callbacks.stopListening == nullptr) || (callbacks.wait == nullptr) || (callbacks.report == nullptr))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Threaded Vosk wake-word example requires complete callbacks");
        }
    }

    /** @brief Runs each bounded source-compatible wake-word detection attempt. */
    void XWalkSttVoskWakeWordThreadExample::run(uint32 detectionCount, uint32 maximumPolls)
    {
        if ((detectionCount == 0U) || (detectionCount > XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_MAXIMUM_DETECTIONS))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Wake-word detection count is outside its range");
        }
        if ((maximumPolls == 0U) || (maximumPolls > XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_MAXIMUM_POLLS))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Wake-word poll count is outside its range");
        }

        for (uint32 detectionIndex = 0U; detectionIndex < detectionCount; ++detectionIndex)
        {
            callbacks.startListening(callbackContext);
            boolean detected{false};
            for (uint32 pollIndex = 0U; pollIndex < maximumPolls; ++pollIndex)
            {
                const hal::boolean wakedMatched = static_cast<hal::boolean>(callbacks.isWaked(callbackContext));
                if (wakedMatched)
                {
                    detected = true;
                    break;
                }
                callbacks.report(callbackContext, "Waiting for wake word...");
                callbacks.wait(callbackContext, 3'000U);
            }
            callbacks.stopListening(callbackContext);
            if (!detected)
            {
                XWALK_HAL_ERROR(XWALK_RUNTIME, "Wake word was not detected before the poll limit");
            }
            callbacks.report(callbackContext, "Wake word detected");
        }
    }

} /* namespace xwalk::hal::example */
