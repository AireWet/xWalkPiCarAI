/******************************************************************************
 * @file        xHal_Rpi5CarSpeechRecognizerVoskTypes.h
 * @brief       Declares dynamically loaded Vosk C API operations.
 *
 * @details
 * Defines opaque Vosk handles and the function signatures resolved from the
 * deployment-selected shared library without requiring vendor headers.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Vosk Provider
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SPEECH_RECOGNIZER_VOSK_TYPES_H
#define XHAL_RPI5CAR_SPEECH_RECOGNIZER_VOSK_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToTextAlsaTypes.h"

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

/** @brief Opaque Vosk model handle owned by the provider. */
using voskmodelhandle = contextpointer;
/** @brief Opaque per-request Vosk recognizer handle. */
using voskrecognizerhandle = contextpointer;
/** @brief Creates one Vosk model from a non-empty directory path. */
using voskmodelnewfunction = voskmodelhandle (*)(cstring modelPath);
/** @brief Releases one non-null Vosk model handle. */
using voskmodelfreefunction = void (*)(voskmodelhandle model);
/** @brief Creates one recognizer for a model and positive sample rate. */
using voskrecognizernewfunction = voskrecognizerhandle (*)(voskmodelhandle model,
    float sampleRateHz);
/** @brief Accepts signed sixteen-bit PCM bytes into one recognizer. */
using voskacceptwaveformfunction = int32 (*)(voskrecognizerhandle recognizer,
    cstring pcmData, int32 byteCount);
/** @brief Returns recognizer-owned final JSON text. */
using voskfinalresultfunction = cstring (*)(voskrecognizerhandle recognizer);
/** @brief Releases one non-null recognizer handle. */
using voskrecognizerfreefunction = void (*)(voskrecognizerhandle recognizer);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkVoskApi
 * @brief Stores every Vosk function required by synchronous recognition.
 */
struct XWalkVoskApi
{
    /** @brief Non-null model-construction function. */
    voskmodelnewfunction modelNew{nullptr};
    /** @brief Non-null model-destruction function. */
    voskmodelfreefunction modelFree{nullptr};
    /** @brief Non-null recognizer-construction function. */
    voskrecognizernewfunction recognizerNew{nullptr};
    /** @brief Non-null PCM ingestion function. */
    voskacceptwaveformfunction acceptWaveform{nullptr};
    /** @brief Non-null final-result function. */
    voskfinalresultfunction finalResult{nullptr};
    /** @brief Non-null recognizer-destruction function. */
    voskrecognizerfreefunction recognizerFree{nullptr};
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_RECOGNIZER_VOSK_TYPES_H */
