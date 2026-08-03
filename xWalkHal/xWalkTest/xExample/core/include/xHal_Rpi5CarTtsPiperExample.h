/******************************************************************************
 * @file        xHal_Rpi5CarTtsPiperExample.h
 * @brief       Declares the Piper text-to-speech example.
 *
 * @details
 * Defines the exact upstream voice model and message behind one injected
 * synchronous speech operation without owning platform resources.
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

#ifndef XHAL_RPI5CAR_TTS_PIPER_EXAMPLE_H
#define XHAL_RPI5CAR_TTS_PIPER_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains host-testable ports of upstream Robot HAT examples.
 */
namespace xwalk::hal::example
{

/******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief Exact Piper voice model selected by the source example. */
inline constexpr stringview XWALK_TTS_PIPER_EXAMPLE_MODEL{"en_US-amy-low"};

/** @brief Exact speech message supplied by the source example. */
inline constexpr stringview XWALK_TTS_PIPER_EXAMPLE_MESSAGE{
    "Hi, I'm piper TTS. A fast and local neural text-to-speech engine that "
    "embeds espeak-ng for phonemization."};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Synthesizes and plays one Piper message synchronously.
 * @param[in,out] context Non-owning context that must remain valid during the call.
 * @param[in] model Non-empty voice-model name selected by the example.
 * @param[in] text Non-empty speech text selected by the example.
 */
using ttspiperspeakcallback = void (*)(contextpointer context,
    stringview model, stringview text);

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkTtsPiperExample
 * @brief Delivers the exact upstream Piper request through one callback.
 */
class XWalkTtsPiperExample final
{
private:

    /**************************************************************************
     * Private data members
     **************************************************************************/

    /** @brief Non-owning context forwarded synchronously to the speech operation. */
    contextpointer callbackContext;

    /** @brief Non-null synchronous speech operation copied during construction. */
    ttspiperspeakcallback speakCallback;

public:

    /**************************************************************************
     * Public constructors and destructor
     **************************************************************************/

    /**
     * @brief Binds one Piper-compatible synchronous speech operation.
     * @param[in,out] context Non-owning context forwarded to `speak`.
     * @param[in] speak Non-null operation accepting model and text views.
     * @throws std::invalid_argument If `speak` is null.
     */
    XWalkTtsPiperExample(
        contextpointer context, ttspiperspeakcallback speak);

    /**************************************************************************
     * Public special member functions
     **************************************************************************/

    XWalkTtsPiperExample(const XWalkTtsPiperExample&) = delete;
    XWalkTtsPiperExample(XWalkTtsPiperExample&&) = delete;
    XWalkTtsPiperExample& operator=(
        const XWalkTtsPiperExample&) = delete;
    XWalkTtsPiperExample& operator=(XWalkTtsPiperExample&&) = delete;

    /**************************************************************************
     * Public member functions
     **************************************************************************/

    /** @brief Synthesizes and plays the exact upstream request once. */
    void run();
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_PIPER_EXAMPLE_H */
