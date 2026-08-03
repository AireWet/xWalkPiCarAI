/******************************************************************************
 * @file        xHal_Rpi5CarSttVoskStreamExample.cpp
 * @brief       Implements the bounded streaming Vosk speech example.
 *
 * @details
 * Validates injected operations and coordinates bounded prompts and streams.
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

#include "xHal_Rpi5CarSttVoskStreamExample.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::example
{

/** @brief Binds and validates all streaming speech operations. */
XWalkSttVoskStreamExample::XWalkSttVoskStreamExample(contextpointer context,
    const XWalkSttVoskStreamExampleCallbacks& exampleCallbacks):
    callbackContext(context), callbacks(exampleCallbacks)
{
    if ((callbacks.listen == nullptr) || (callbacks.reportPrompt == nullptr) ||
        (callbacks.reportResult == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Streaming Vosk example requires a complete callback table");
    }
}

/** @brief Runs source-compatible prompts and bounded recognition streams. */
void XWalkSttVoskStreamExample::run(uint32 sessionCount, uint32 timeoutMs)
{
    if ((sessionCount == 0U) ||
        (sessionCount > XHAL_RPI5CAR_STT_VOSK_STREAM_EXAMPLE_MAXIMUM_SESSIONS))
    {
        XHAL_THROW_OUT_OF_RANGE("Streaming Vosk session count is outside its range");
    }
    if ((timeoutMs == 0U) ||
        (timeoutMs > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        XHAL_THROW_OUT_OF_RANGE("Streaming Vosk timeout is outside its range");
    }

    for (uint32 sessionIndex = 0U; sessionIndex < sessionCount; ++sessionIndex)
    {
        callbacks.reportPrompt(callbackContext);
        callbacks.listen(callbackContext, timeoutMs, callbacks.reportResult);
    }
}

} /* namespace xwalk::hal::example */
