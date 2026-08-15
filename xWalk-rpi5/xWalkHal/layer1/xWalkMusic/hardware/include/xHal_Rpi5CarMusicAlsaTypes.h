/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaTypes.h
 * @brief       Declares decoded music data and decoder callback types.
 *
 * @details
 * Defines signed sixteen-bit interleaved PCM data passed from a file decoder to
 * the ALSA music adapter without coupling the music module to speaker playback.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic ALSA Adapter
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

#ifndef XHAL_RPI5CAR_MUSIC_ALSA_TYPES_H
#define XHAL_RPI5CAR_MUSIC_ALSA_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicTypes.h"

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
     * @struct XWalkMusicAlsaAudioData
     * @brief Contains decoded signed sixteen-bit little-endian PCM frames.
     */
    struct XWalkMusicAlsaAudioData
    {
            /** @brief Complete interleaved signed sixteen-bit little-endian PCM bytes. */
            bytevector pcmData{};
            /** @brief Positive playback sample rate in Hertz. */
            uint32 sampleRateHz{};
            /** @brief Interleaved channel count from one through eight. */
            uint8 channelCount{};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Decodes one audio file into signed sixteen-bit PCM.
     *
     * @param[in,out] context
     * Nullable non-owning decoder context that outlives the adapter.
     *
     * @param[in] filename
     * Non-empty path view valid only for the callback duration.
     *
     * @return
     * Owned decoded PCM data with positive rate and channel metadata.
     */
    using musicalsaudiodecodecallback = XWalkMusicAlsaAudioData (*)(contextpointer context, stringview filename);

    /**
     * @struct XWalkMusicAlsaOperations
     * @brief Contains the complete injectable file-decoding operation table.
     */
    struct XWalkMusicAlsaOperations
    {
            /** @brief Decodes one file before synchronous or worker playback begins. */
            musicalsaudiodecodecallback decodeAudio{nullptr};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MUSIC_ALSA_TYPES_H */
