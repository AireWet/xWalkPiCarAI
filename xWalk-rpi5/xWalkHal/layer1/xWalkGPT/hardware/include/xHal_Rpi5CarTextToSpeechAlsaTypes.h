/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechAlsaTypes.h
 * @brief       Declares bounded synthesis data and provider operations.
 *
 * @details
 * Defines signed sixteen-bit PCM returned by one application-selected speech
 * provider before playback through the shared ALSA owner.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Text-to-Speech ALSA Backend
 *
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_ALSA_TYPES_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_ALSA_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTextToSpeechTypes.h"

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
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkTextToSpeechPcmData
     * @brief Contains one bounded synthesized signed sixteen-bit PCM result.
     *
     * @details The payload is interleaved little-endian PCM. Empty data represents
     * a completed synthesis request with no audible output.
     */
    struct XWalkTextToSpeechPcmData
    {
            /** @brief Complete interleaved signed sixteen-bit little-endian PCM bytes. */
            bytevector pcmData{};

            /** @brief Positive playback sample rate in Hertz when PCM data is non-empty. */
            uint32 sampleRateHz{};

            /** @brief Interleaved channel count from one through eight when PCM data is non-empty. */
            uint8 channelCount{};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Synthesizes one text value into bounded signed sixteen-bit PCM.
     *
     * @param[in,out] context
     * Nullable non-owning provider context that must outlive the adapter.
     *
     * @param[in] text
     * Text view valid only for this synchronous call and never retained by the provider.
     *
     * @return
     * Owned PCM data, sample rate, and channel count. Empty PCM requests no playback.
     *
     * @warning
     * Provider credentials and request content must not be written to normal diagnostics.
     */
    using texttospeechsynthesizecallback = XWalkTextToSpeechPcmData (*)(contextpointer context, stringview text);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkTextToSpeechAlsaOperations
     * @brief Contains the complete injectable synthesis-provider operation table.
     */
    struct XWalkTextToSpeechAlsaOperations
    {
            /** @brief Synthesizes one request before any ALSA stream is opened. */
            texttospeechsynthesizecallback synthesize{nullptr};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_ALSA_TYPES_H */
