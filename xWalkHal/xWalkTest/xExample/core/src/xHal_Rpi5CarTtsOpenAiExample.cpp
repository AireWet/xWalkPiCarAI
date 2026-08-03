/******************************************************************************
 * @file        xHal_Rpi5CarTtsOpenAiExample.cpp
 * @brief       Implements the OpenAI text-to-speech example.
 *
 * @details
 * Reports and forwards the exact three source requests in stable order through
 * injected callbacks without performing network or audio access directly.
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

#include "xHal_Rpi5CarTtsOpenAiExample.h"

namespace xwalk::hal::example
{

/** @brief Binds and validates the required synchronous operations. */
XWalkTtsOpenAiExample::XWalkTtsOpenAiExample(contextpointer context,
    ttsopenaispeakcallback speak, ttsopenaireportcallback report):
    callbackContext(context), speakCallback(speak), reportCallback(report)
{
    if ((speakCallback == nullptr) || (reportCallback == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "OpenAI TTS example requires speech and report callbacks");
    }
}

/** @brief Delivers the exact source messages and instructions in order. */
void XWalkTtsOpenAiExample::run()
{
    constexpr stringview firstMessage{"Hello! I'm OpenAI TTS."};
    constexpr stringview secondMessage{
        "with instructions, I can say word sadly"};
    constexpr stringview secondInstructions{"say it sadly"};
    constexpr stringview thirdMessage{"or say something dramaticly."};
    constexpr stringview thirdInstructions{"say it dramaticly"};

    reportCallback(callbackContext, "Say: Hello! I'm OpenAI TTS.");
    speakCallback(callbackContext, XWALK_TTS_OPEN_AI_EXAMPLE_MODEL,
        XWALK_TTS_OPEN_AI_EXAMPLE_VOICE, firstMessage, {});
    reportCallback(callbackContext,
        "Say: with instructions, I can say word sadly, with instructions: "
        "'say it sadly'");
    speakCallback(callbackContext, XWALK_TTS_OPEN_AI_EXAMPLE_MODEL,
        XWALK_TTS_OPEN_AI_EXAMPLE_VOICE, secondMessage, secondInstructions);
    reportCallback(callbackContext,
        "Say: or say something dramaticly., with instructions: "
        "'say it dramaticly'");
    speakCallback(callbackContext, XWALK_TTS_OPEN_AI_EXAMPLE_MODEL,
        XWALK_TTS_OPEN_AI_EXAMPLE_VOICE, thirdMessage, thirdInstructions);
}

} /* namespace xwalk::hal::example */
