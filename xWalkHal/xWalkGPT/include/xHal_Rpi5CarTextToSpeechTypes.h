/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechTypes.h
 * @brief       Declares the text-to-speech backend callback contract.
 *
 * @details
 * Defines synchronous text delivery to an application-selected local or
 * remote speech engine without coupling the firmware core to that engine.
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

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_TYPES_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Callback that synthesizes and outputs one text value.
 *
 * @param[in,out] context
 * Nullable non-owning backend context supplied during construction. Null is
 * permitted only when the callback implementation supports it.
 *
 * @param[in] text
 * Non-owning text view valid only for the duration of the synchronous callback.
 * The backend determines its supported encoding, language, and maximum length.
 *
 * @pre
 * Any non-null context remains valid for the text-to-speech object's lifetime.
 *
 * @warning
 * The callback must not retain `text` beyond the invocation.
 */
using texttospeechspeakcallback = void (*)(contextpointer context, stringview text);

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_TYPES_H */
