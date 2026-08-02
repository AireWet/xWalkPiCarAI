/******************************************************************************
 * @file        xHal_Rpi5CarMusicSndFileDecoder.h
 * @brief       Declares optional libsndfile decoding for Music ALSA playback.
 *
 * @details
 * Exposes one stateless decoder operation that converts libsndfile-supported
 * audio files, including MP3, into signed sixteen-bit little-endian PCM.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic SndFile Decoder
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

#ifndef XHAL_RPI5CAR_MUSIC_SNDFILE_DECODER_H
#define XHAL_RPI5CAR_MUSIC_SNDFILE_DECODER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicAlsaTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkMusicSndFileDecoder
 * @brief Provides one optional libsndfile decoder callback for Music ALSA.
 *
 * @details
 * The class has no instances or retained state. Each callback invocation owns
 * its libsndfile handle until decoding completes and returns owned PCM bytes.
 */
class XWalkMusicSndFileDecoder final
{
    private:
        /**
         * @brief Decodes one libsndfile-supported audio file into PCM.
         *
         * @param[in,out] context
         * Unused nullable callback context.
         *
         * @param[in] filename
         * Non-empty path to a readable audio file.
         *
         * @return
         * Complete signed sixteen-bit little-endian PCM with stream metadata.
         *
         * @throws std::runtime_error
         * If the file cannot be opened, decoded, or represented safely.
         *
         * @throws std::out_of_range
         * If decoded PCM exceeds the configured memory bound.
         */
        static XWalkMusicAlsaAudioData decodeAudio(contextpointer context,
            stringview filename);

    public:
        /** @brief Prevents instances of this stateless operation provider. */
        XWalkMusicSndFileDecoder() = delete;

        /**
         * @brief Returns the complete libsndfile decoder operation table.
         * @return Operation table containing one non-null decoder callback.
         */
        static XWalkMusicAlsaOperations operations() noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MUSIC_SNDFILE_DECODER_H */
