/******************************************************************************
 * @file        xHal_Rpi5CarTtsEdgeExample.h
 * @brief       Declares the Microsoft Edge text-to-speech example.
 *
 * @details
 * Defines the exact upstream voice, message, and injected synchronous speech
 * operation without owning a network service, process, or audio device.
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

#ifndef XHAL_RPI5CAR_TTS_EDGE_EXAMPLE_H
#define XHAL_RPI5CAR_TTS_EDGE_EXAMPLE_H

#include "xHal_Rpi5CarCommon.h"

namespace xwalk::hal::example
{

/** @brief Exact Microsoft neural voice selected by the source example. */
inline constexpr stringview XWALK_TTS_EDGE_EXAMPLE_VOICE{"en-US-AriaNeural"};
/** @brief Exact speech message supplied by the source example. */
inline constexpr stringview XWALK_TTS_EDGE_EXAMPLE_MESSAGE{
    "Hi, I'm Edge TTS. A free cloud text-to-speech service powered by Microsoft Edge."};

/** @brief Synthesizes and plays one message with the selected cloud voice. */
using ttsedgespeakcallback = void (*)(contextpointer context,
    stringview voice, stringview text);

/** @brief Delivers the exact upstream Edge TTS request through one callback. */
class XWalkTtsEdgeExample final
{
private:

    /** @brief Non-owning context forwarded to the speech operation. */
    contextpointer callbackContext;
    /** @brief Non-null synchronous speech operation. */
    ttsedgespeakcallback speakCallback;

public:

    /**
     * @brief Binds one Edge-compatible synchronous speech operation.
     * @param[in,out] context Non-owning context forwarded to `speak`.
     * @param[in] speak Non-null operation accepting voice and text views.
     * @throws std::invalid_argument If `speak` is null.
     */
    XWalkTtsEdgeExample(contextpointer context, ttsedgespeakcallback speak);

    XWalkTtsEdgeExample(const XWalkTtsEdgeExample&) = delete;
    XWalkTtsEdgeExample(XWalkTtsEdgeExample&&) = delete;
    XWalkTtsEdgeExample& operator=(const XWalkTtsEdgeExample&) = delete;
    XWalkTtsEdgeExample& operator=(XWalkTtsEdgeExample&&) = delete;

    /** @brief Synthesizes and plays the exact upstream message once. */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_EDGE_EXAMPLE_H */
