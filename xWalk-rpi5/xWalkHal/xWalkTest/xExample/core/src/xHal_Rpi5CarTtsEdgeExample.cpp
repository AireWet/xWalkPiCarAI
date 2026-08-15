/******************************************************************************
 * @file        xHal_Rpi5CarTtsEdgeExample.cpp
 * @brief       Implements the Microsoft Edge text-to-speech example.
 *
 * @details
 * Validates one injected speech operation and forwards the exact fixed voice
 * and message once without modifying either value.
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

#include "xHal_Rpi5CarTtsEdgeExample.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::example
{

    /** @brief Binds and validates one synchronous speech operation. */
    XWalkTtsEdgeExample::XWalkTtsEdgeExample(contextpointer context, ttsedgespeakcallback speak)
        : callbackContext(context), speakCallback(speak)
    {
        if (speakCallback == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Edge TTS example requires a speech callback");
        }
    }

    /** @brief Delivers the exact source voice and message once. */
    void XWalkTtsEdgeExample::run()
    {
        speakCallback(callbackContext, XWALK_TTS_EDGE_EXAMPLE_VOICE, XWALK_TTS_EDGE_EXAMPLE_MESSAGE);
    }

} /* namespace xwalk::hal::example */
