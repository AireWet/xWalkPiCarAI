/******************************************************************************
 * @file        xHal_Rpi5CarTtsPico2WaveExample.cpp
 * @brief       Implements the Pico2Wave text-to-speech example.
 *
 * @details
 * Validates one injected speech operation and forwards the exact fixed
 * language and message once without modifying either value.
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

#include "xHal_Rpi5CarTtsPico2WaveExample.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains host-testable ports of upstream Robot HAT examples.
 */
namespace xwalk::hal::example
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Binds and validates one synchronous Pico2Wave speech operation.
 * @param[in,out] context Non-owning context forwarded to `speak`.
 * @param[in] speak Non-null operation accepting language and text views.
 * @throws std::invalid_argument If `speak` is null.
 */
XWalkTtsPico2WaveExample::XWalkTtsPico2WaveExample(
    contextpointer context, ttspico2wavespeakcallback speak):
    callbackContext(context), speakCallback(speak)
{
    if (speakCallback == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT(
            "Pico2Wave example requires a speech callback");
    }
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Delivers the exact source language and message once.
 */
void XWalkTtsPico2WaveExample::run()
{
    speakCallback(callbackContext, XWALK_TTS_PICO2WAVE_EXAMPLE_LANGUAGE,
        XWALK_TTS_PICO2WAVE_EXAMPLE_MESSAGE);
}

} /* namespace xwalk::hal::example */
