/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeech.cpp
 * @brief       Implements synchronous text-to-speech backend forwarding.
 *
 * @details
 * Delivers caller-provided text without modification to the injected speech
 * engine after successful Robot HAT speaker activation.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarTextToSpeech.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Synthesizes and outputs one text value through the backend.
 *
 * @param[in] text
 * Text view forwarded synchronously without modification. The backend defines
 * supported encoding, language, length, and empty-text behavior.
 *
 * @pre
 * The Robot HAT speaker was enabled successfully during construction.
 *
 * @note
 * Any exception raised by the injected callback is propagated.
 */
void XWalkTextToSpeech::speak(stringview text) {
  speakCallback(backendContextPointer, text);
  XWALK_HAL_TRACE_UID1(
      RPI .362, "Text-to-speech completed for %zu character(s)", text.size());
}

} /* namespace xwalk::hal */
