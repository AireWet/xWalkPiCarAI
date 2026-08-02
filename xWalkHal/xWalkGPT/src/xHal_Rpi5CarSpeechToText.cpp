/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToText.cpp
 * @brief       Implements synchronous speech-recognition operations.
 *
 * @details
 * Dispatches readiness, bounded microphone recognition, file transcription,
 * and stop requests through the validated application backend.
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

#include "xHal_Rpi5CarSpeechToText.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Reports whether the backend can begin recognition.
 *
 * @return
 * `true` when the backend reports readiness; otherwise `false`.
 *
 * @note
 * Any exception raised by the injected callback is propagated.
 */
boolean XWalkSpeechToText::isReady() const
{
    return callbacks.ready(backendContextPointer);
}

/**
 * @brief Records microphone input and returns final recognized text.
 *
 * @param[in] timeoutMs
 * Non-zero maximum recognition interval in milliseconds, no greater than
 * `XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS`.
 *
 * @return
 * Owned recognized text, or an empty string when no speech is recognized.
 *
 * @throws std::out_of_range
 * If `timeoutMs` is outside the supported range.
 *
 * @note
 * Any exception raised by the injected callback is propagated.
 */
string XWalkSpeechToText::listen(uint32 timeoutMs)
{
    validateTimeout(timeoutMs);
    return callbacks.listen(backendContextPointer, timeoutMs);
}

/**
 * @brief Transcribes one backend-supported audio file.
 *
 * @param[in] filePath
 * Non-empty path forwarded synchronously without filesystem inspection.
 *
 * @return
 * Owned recognized text, or an empty string when no speech is recognized.
 *
 * @throws std::invalid_argument
 * If `filePath` is empty.
 *
 * @note
 * Backend file, format, model, and exception behavior is propagated.
 */
string XWalkSpeechToText::transcribeFile(stringview filePath)
{
    validateFilePath(filePath);
    return callbacks.transcribeFile(backendContextPointer, filePath);
}

/**
 * @brief Requests cancellation of active recognition.
 *
 * @post
 * The stop request has been delivered synchronously to the backend.
 *
 * @note
 * Any exception raised by the injected callback is propagated.
 */
void XWalkSpeechToText::stop()
{
    callbacks.stop(backendContextPointer);
}

} /* namespace xwalk::hal */
