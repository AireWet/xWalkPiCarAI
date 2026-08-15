/******************************************************************************
 * @file        xHal_Rpi5CarMusicSndFileDecoderTypes.h
 * @brief       Declares private libsndfile decoder support types.
 * @project     xWalk Firmware
 * @module      xWalkMusic SndFile Decoder
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_MUSIC_SNDFILE_DECODER_TYPES_H
#define XHAL_RPI5CAR_MUSIC_SNDFILE_DECODER_TYPES_H

#include <sndfile.h>

namespace xwalk::hal::music::decoder
{

    /** @brief Closes one nullable libsndfile handle owned by a unique pointer. */
    struct SndFileCloser
    {
            /** @brief Closes one retained decoder handle. */
            void operator()(SNDFILE* handle) const noexcept;
    };

} /* namespace xwalk::hal::music::decoder */

#endif /* XHAL_RPI5CAR_MUSIC_SNDFILE_DECODER_TYPES_H */
