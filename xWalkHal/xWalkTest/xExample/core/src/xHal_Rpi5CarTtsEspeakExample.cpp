/******************************************************************************
 * @file        xHal_Rpi5CarTtsEspeakExample.cpp
 * @brief       Implements the configured Espeak text-to-speech example.
 *
 * @details
 * Validates one injected speech operation and forwards every exact fixed
 * setting and the fixed message once without modifying their values.
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

#include "xHal_Rpi5CarTtsEspeakExample.h"

namespace xwalk::hal::example
{

/** @brief Binds and validates one configured synchronous speech operation. */
XWalkTtsEspeakExample::XWalkTtsEspeakExample(
    contextpointer context, ttsespeakspeakcallback speak):
    callbackContext(context), speakCallback(speak)
{
    if (speakCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Espeak example requires a speech callback");
    }
}

/** @brief Delivers every exact source setting and message once. */
void XWalkTtsEspeakExample::run()
{
    speakCallback(callbackContext, XWALK_TTS_ESPEAK_EXAMPLE_AMPLITUDE,
        XWALK_TTS_ESPEAK_EXAMPLE_SPEED, XWALK_TTS_ESPEAK_EXAMPLE_GAP,
        XWALK_TTS_ESPEAK_EXAMPLE_PITCH, XWALK_TTS_ESPEAK_EXAMPLE_MESSAGE);
}

} /* namespace xwalk::hal::example */
