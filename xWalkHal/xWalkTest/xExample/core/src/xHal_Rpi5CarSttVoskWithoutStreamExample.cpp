/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskWithoutStreamExample.cpp
 * @brief       Implements the bounded non-streaming Vosk speech example.
 *
 * @details
 * Validates injected operations and coordinates exact prompts, synchronous
 * recognition requests, and unlabeled final transcript output.
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

#include "xHal_Rpi5CarSttVoskWithoutStreamExample.h"

namespace xwalk::hal::example
{

/** @brief Binds and validates synchronous listen and report operations. */
XWalkSttVoskWithoutStreamExample::XWalkSttVoskWithoutStreamExample(
    contextpointer context,
    const XWalkSttVoskWithoutStreamExampleCallbacks& exampleCallbacks):
    callbackContext(context), callbacks(exampleCallbacks)
{
    if ((callbacks.listen == nullptr) || (callbacks.report == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Non-streaming Vosk example requires complete callbacks");
    }
}

/** @brief Runs bounded source-compatible prompt, listen, and report cycles. */
void XWalkSttVoskWithoutStreamExample::run(
    uint32 sessionCount, uint32 timeoutMs)
{
    if ((sessionCount == 0U) ||
        (sessionCount > XHAL_RPI5CAR_STT_VOSK_WITHOUT_STREAM_EXAMPLE_MAXIMUM_SESSIONS))
    {
        XHAL_THROW_OUT_OF_RANGE("Non-streaming Vosk session count is outside its range");
    }
    if ((timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        XHAL_THROW_OUT_OF_RANGE("Non-streaming Vosk timeout is outside its range");
    }

    for (uint32 sessionIndex = 0U; sessionIndex < sessionCount; ++sessionIndex)
    {
        callbacks.report(callbackContext, "Say something");
        callbacks.report(callbackContext,
            callbacks.listen(callbackContext, timeoutMs));
    }
}

} /* namespace xwalk::hal::example */
