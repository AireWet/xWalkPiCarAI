/******************************************************************************
 * @file        xHal_Rpi5CarMusicTypes.h
 * @brief       Declares music state and injected audio-backend types.
 *
 * @details
 * Defines fixed music-theory values and the callback contract used to route
 * file playback and generated PCM tones to a caller-owned audio backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_MUSIC_TYPES_H
#define XHAL_RPI5CAR_MUSIC_TYPES_H

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

    /** @brief Fixed upper and lower values of one musical time signature. */
    using musictimesignature = fixedarray<uint32, XHAL_RPI5CAR_MUSIC_PAIR_VALUE_COUNT>;

    /** @brief Fixed tempo in beats per minute and its whole-note beat value. */
    using musictempo = fixedarray<float64, XHAL_RPI5CAR_MUSIC_PAIR_VALUE_COUNT>;

    /**
     * @brief Callback that enables the platform audio output.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     */
    using musicenableoutputcallback = void (*)(contextpointer context);

    /**
     * @brief Callback that plays one sound effect synchronously or asynchronously.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     *
     * @param[in] filename
     * Non-empty path view that remains valid only for the callback duration.
     *
     * @param[in] normalizedVolume
     * Optional volume in the inclusive range 0.0 to 1.0; omission preserves the
     * backend sound object's current volume.
     */
    using musicsoundplaycallback = void (*)(contextpointer context,
                                            stringview filename,
                                            optionalfloat64 normalizedVolume);

    /**
     * @brief Callback that starts streamed music-file playback.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     *
     * @param[in] filename
     * Non-empty path view that remains valid only for the callback duration.
     *
     * @param[in] loops
     * Non-negative Python-compatible loop argument forwarded without conversion.
     *
     * @param[in] startSeconds
     * Finite non-negative playback offset in seconds.
     */
    using musicfileplaycallback = void (*)(contextpointer context,
                                           stringview filename,
                                           int32 loops,
                                           float64 startSeconds);

    /**
     * @brief Callback that changes streamed-music volume.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     *
     * @param[in] normalizedVolume
     * Rounded volume in the inclusive range 0.0 to 1.0.
     */
    using musicvolumecallback = void (*)(contextpointer context, float64 normalizedVolume);

    /**
     * @brief Callback that controls the current streamed-music operation.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     */
    using musiccontrolcallback = void (*)(contextpointer context);

    /**
     * @brief Callback that measures one sound-effect file.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     *
     * @param[in] filename
     * Non-empty path view that remains valid only for the callback duration.
     *
     * @return
     * Finite non-negative sound duration in seconds.
     */
    using musicsoundlengthcallback = float64 (*)(contextpointer context, stringview filename);

    /**
     * @brief Callback that writes generated signed 16-bit mono PCM data.
     *
     * @param[in,out] context
     * Non-owning backend context; nullability is backend-specific.
     *
     * @param[in] pcmData
     * Little-endian signed 16-bit PCM bytes valid only for the callback duration.
     *
     * @param[in] sampleRateHz
     * PCM sample rate in Hertz.
     *
     * @param[in] channelCount
     * Number of interleaved PCM channels.
     */
    using musictoneplaycallback = void (*)(contextpointer context,
                                           const bytevector& pcmData,
                                           uint32 sampleRateHz,
                                           uint8 channelCount);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Contains the complete caller-owned audio-backend operation table.
     *
     * @details
     * Every callback is required. The structure stores no resources and all
     * operations receive the same non-owning context supplied to `XWalkMusic`.
     */
    struct XWalkMusicCallbacks
    {
            /** @brief Enables the platform output during music-controller construction. */
            musicenableoutputcallback enableOutput{nullptr};
            /** @brief Plays a sound effect and returns after playback completes. */
            musicsoundplaycallback playSound{nullptr};
            /** @brief Starts sound-effect playback without waiting for completion. */
            musicsoundplaycallback playSoundBackground{nullptr};
            /** @brief Loads and starts streamed music playback. */
            musicfileplaycallback playMusic{nullptr};
            /** @brief Changes only the streamed-music volume. */
            musicvolumecallback setMusicVolume{nullptr};
            /** @brief Stops streamed music. */
            musiccontrolcallback stopMusic{nullptr};
            /** @brief Pauses streamed music. */
            musiccontrolcallback pauseMusic{nullptr};
            /** @brief Resumes paused streamed music. */
            musiccontrolcallback resumeMusic{nullptr};
            /** @brief Measures a sound effect without starting playback. */
            musicsoundlengthcallback getSoundLength{nullptr};
            /** @brief Writes generated PCM bytes to the platform output. */
            musictoneplaycallback playTone{nullptr};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MUSIC_TYPES_H */
