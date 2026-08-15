/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextTypes.h
 * @brief       Declares speech-recognition backend callback types.
 *
 * @details
 * Defines readiness, bounded microphone recognition, audio-file transcription,
 * and stop operations supplied by an application-owned backend.
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

#ifndef XHAL_RPI5CAR_SPEECH_TO_TEXT_TYPES_H
#define XHAL_RPI5CAR_SPEECH_TO_TEXT_TYPES_H

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
     * @brief Callback that reports whether the recognition backend is ready.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction. Null is
     * permitted only when the callback implementation supports it.
     *
     * @return
     * `true` when recognition can begin; otherwise `false`.
     */
    using speechtotextreadycallback = boolean (*)(contextpointer context);

    /**
     * @brief Callback that records microphone input and returns recognized text.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] timeoutMs
     * Non-zero maximum recognition interval in milliseconds, no greater than
     * `XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS`.
     *
     * @return
     * Owned recognized text. An empty string indicates that no speech was recognized.
     *
     * @warning
     * The callback must return within the supplied bounded interval plus documented
     * backend shutdown latency.
     */
    using speechtotextlistencallback = string (*)(contextpointer context, uint32 timeoutMs);

    /**
     * @brief Callback that transcribes one backend-supported audio file.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @param[in] filePath
     * Non-empty path view valid only for the synchronous callback duration.
     *
     * @return
     * Owned recognized text. An empty string indicates that no speech was recognized.
     */
    using speechtotextfilecallback = string (*)(contextpointer context, stringview filePath);

    /**
     * @brief Callback that requests cancellation of active recognition.
     *
     * @param[in,out] context
     * Nullable non-owning backend context supplied during construction.
     *
     * @note
     * The callback must tolerate a request when no recognition operation is active.
     */
    using speechtotextstopcallback = void (*)(contextpointer context);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Groups the complete operations required from a speech backend.
     *
     * @details
     * Every callback is required and copied during construction. The table owns no
     * backend object or recognition resource.
     */
    struct XWalkSpeechToTextCallbacks
    {
            speechtotextreadycallback ready{nullptr};         /**< Reports backend readiness. */
            speechtotextlistencallback listen{nullptr};       /**< Performs bounded microphone recognition. */
            speechtotextfilecallback transcribeFile{nullptr}; /**< Transcribes one audio file. */
            speechtotextstopcallback stop{nullptr};           /**< Requests active-recognition cancellation. */
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_TO_TEXT_TYPES_H */
