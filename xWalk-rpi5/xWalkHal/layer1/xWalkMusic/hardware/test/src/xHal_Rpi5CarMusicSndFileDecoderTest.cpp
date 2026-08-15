/******************************************************************************
 * @file        xHal_Rpi5CarMusicSndFileDecoderTest.cpp
 * @brief       Verifies native MP3 decoding through libsndfile.
 *
 * @details
 * Decodes the packaged MP3 through the production callback and verifies the
 * signed sixteen-bit interleaved PCM contract without accessing audio hardware.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic SndFile Decoder Test
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicSndFileDecoder.h"

#include <cassert>

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Decodes one packaged MP3 and verifies its PCM metadata and alignment.
 * @param[in] argumentCount Number of process arguments including the executable name.
 * @param[in] arguments Non-owning process argument array containing the MP3 path.
 * @return Zero after every assertion passes.
 */
int main(int argumentCount, char* arguments[])
{
    assert(argumentCount == 2);
    const XWalkHal::XWalkMusicAlsaOperations operations = XWalkHal::XWalkMusicSndFileDecoder::operations();
    assert(operations.decodeAudio != nullptr);
    const XWalkHal::XWalkMusicAlsaAudioData audioData = operations.decodeAudio(nullptr, arguments[1]);
    assert(audioData.sampleRateHz > 0U);
    assert(audioData.channelCount > 0U);
    assert(audioData.channelCount <= XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT);
    assert(!audioData.pcmData.empty());
    const XWalkHal::size bytesPerFrame = static_cast<XWalkHal::size>(audioData.channelCount) * 2U;
    assert((audioData.pcmData.size() % bytesPerFrame) == 0U);
    return 0;
}
