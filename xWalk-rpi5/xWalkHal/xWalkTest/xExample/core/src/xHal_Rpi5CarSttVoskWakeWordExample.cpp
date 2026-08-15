/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWakeWordExample.cpp
 * @brief       Implements the bounded synchronous Vosk wake-word example.
 *
 * @details
 * Reports the upstream prompt, performs bounded recognition attempts, matches
 * the wake phrase without case sensitivity, and reports successful detection.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSttVoskWakeWordExample.h"

#include "xHal_Rpi5CarTrace.h"
#include <algorithm>
#include <cctype>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

    /** @brief Binds and validates the recognition and reporting operations. */
    XWalkSttVoskWakeWordExample::XWalkSttVoskWakeWordExample(
        contextpointer context, const XWalkSttVoskWakeWordExampleCallbacks& exampleCallbacks)
        : callbackContext(context), callbacks(exampleCallbacks)
    {
        if ((callbacks.listen == nullptr) || (callbacks.report == nullptr))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Synchronous Vosk wake-word example requires complete callbacks");
        }
    }

    /** @brief Performs bounded synchronous recognition until the phrase is found.
     */
    void XWalkSttVoskWakeWordExample::run(uint32 maximumAttempts, uint32 timeoutMs)
    {
        if ((maximumAttempts == 0U) || (maximumAttempts > XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_MAXIMUM_ATTEMPTS))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Wake-word attempt count is outside its range");
        }
        if ((timeoutMs == 0U) || (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Wake-word listen timeout is outside its range");
        }

        callbacks.report(callbackContext, "Wake me with :\"Hey robot\"");
        for (uint32 attemptIndex = 0U; attemptIndex < maximumAttempts; ++attemptIndex)
        {
            const hal::boolean containsWakeWordCallbacksListenSet =
                static_cast<hal::boolean>(containsWakeWord(callbacks.listen(callbackContext, timeoutMs)));
            if (containsWakeWordCallbacksListenSet)
            {
                callbacks.report(callbackContext, "Wake word detected");
                return;
            }
        }
        XWALK_HAL_ERROR(XWALK_RUNTIME, "Wake word was not detected before the attempt limit");
    }

    /** @brief Performs case-insensitive phrase detection on one transcript. */
    boolean XWalkSttVoskWakeWordExample::containsWakeWord(stringview transcript)
    {
        string normalizedTranscript(transcript);
        std::transform(normalizedTranscript.begin(),
                       normalizedTranscript.end(),
                       normalizedTranscript.begin(),
                       [](char value)
                       {
                           return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
                       });
        return normalizedTranscript.find(XHAL_RPI5CAR_STT_VOSK_WAKE_WORD_EXAMPLE_PHRASE) != string::npos;
    }

} /* namespace xwalk::hal::example */
