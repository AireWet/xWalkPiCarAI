/******************************************************************************
 * @file        xHal_Rpi5CarTtsOpenAiExample.h
 * @brief       Declares the OpenAI text-to-speech example.
 *
 * @details
 * Preserves the exact upstream model, voice, messages, and speech instructions
 * behind injected synchronous callbacks without owning network or audio I/O.
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

#ifndef XHAL_RPI5CAR_TTS_OPEN_AI_EXAMPLE_H
#define XHAL_RPI5CAR_TTS_OPEN_AI_EXAMPLE_H

#include "xHal_Rpi5CarCommon.h"

namespace xwalk::hal::example
{

/** @brief Exact OpenAI speech model selected by the source example. */
inline constexpr stringview XWALK_TTS_OPEN_AI_EXAMPLE_MODEL{"gpt-4o-mini-tts"};
/** @brief Exact OpenAI voice selected by the source example. */
inline constexpr stringview XWALK_TTS_OPEN_AI_EXAMPLE_VOICE{"alloy"};

/** @brief Synthesizes and plays one OpenAI speech request. */
using ttsopenaispeakcallback = void (*)(contextpointer context,
    stringview model, stringview voice, stringview text,
    stringview instructions);
/** @brief Reports one source-compatible console message. */
using ttsopenaireportcallback = void (*)(contextpointer context,
    stringview message);

/** @brief Delivers the three exact upstream OpenAI TTS requests. */
class XWalkTtsOpenAiExample final
{
private:

    /** @brief Non-owning context forwarded to both callbacks. */
    contextpointer callbackContext;
    /** @brief Non-null synchronous speech operation. */
    ttsopenaispeakcallback speakCallback;
    /** @brief Non-null synchronous reporting operation. */
    ttsopenaireportcallback reportCallback;

public:

    /**
     * @brief Binds required OpenAI speech and reporting operations.
     * @param[in,out] context Non-owning context forwarded to both callbacks.
     * @param[in] speak Non-null synchronous speech operation.
     * @param[in] report Non-null synchronous reporting operation.
     * @throws std::invalid_argument If either callback is null.
     */
    XWalkTtsOpenAiExample(contextpointer context,
        ttsopenaispeakcallback speak, ttsopenaireportcallback report);

    XWalkTtsOpenAiExample(const XWalkTtsOpenAiExample&) = delete;
    XWalkTtsOpenAiExample(XWalkTtsOpenAiExample&&) = delete;
    XWalkTtsOpenAiExample& operator=(const XWalkTtsOpenAiExample&) = delete;
    XWalkTtsOpenAiExample& operator=(XWalkTtsOpenAiExample&&) = delete;

    /** @brief Reports, synthesizes, and plays all three fixed requests. */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_OPEN_AI_EXAMPLE_H */
