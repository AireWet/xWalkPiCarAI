/******************************************************************************
 * @file        xHal_Rpi5CarMusicSndFileDecoder.cpp
 * @brief       Implements optional libsndfile decoding for Music ALSA playback.
 *
 * @details
 * Reads bounded complete frames through libsndfile and serializes every sample
 * explicitly as signed sixteen-bit little-endian PCM for the shared ALSA owner.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicSndFileDecoder.h"

#include "xHal_Rpi5CarTrace.h"
#include <array>
#include <limits>
#include <memory>
#include <sndfile.h>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal {

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace {

/** @brief Maximum decoded PCM retained for one Music operation. */
constexpr size maximumDecodedPcmBytes = 64U * 1024U * 1024U;
/** @brief Complete frames requested during one libsndfile read. */
constexpr size decoderReadFrames = 1024U;
/** @brief Bytes emitted for one decoded signed sixteen-bit sample. */
constexpr size decodedSampleBytes = 2U;

/** @brief Closes one nullable libsndfile handle owned by a unique pointer. */
struct SndFileCloser {
  /**
   * @brief Closes one retained decoder handle.
   * @param[in,out] handle Nullable libsndfile handle.
   */
  void operator()(SNDFILE *handle) const noexcept {
    if (handle != nullptr) {
      static_cast<void>(::sf_close(handle));
    }
  }
};

} /* namespace */

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns the complete libsndfile decoder operation table.
 * @return Operation table containing one non-null decoder callback.
 */
XWalkMusicAlsaOperations XWalkMusicSndFileDecoder::operations() noexcept {
  return {&decodeAudio};
}

/******************************************************************************
 * Private member function definitions
 ******************************************************************************/

/**
 * @brief Decodes one libsndfile-supported audio file into PCM.
 * @param[in,out] context Unused nullable callback context.
 * @param[in] filename Non-empty path to a readable audio file.
 * @return Complete signed sixteen-bit little-endian PCM with stream metadata.
 */
XWalkMusicAlsaAudioData
XWalkMusicSndFileDecoder::decodeAudio(contextpointer context,
                                      stringview filename) {
  static_cast<void>(context);
  const hal::boolean filenameEmpty =
      static_cast<hal::boolean>(filename.empty());
  if (filenameEmpty) {
    XWALK_HAL_ERROR(XWALK_INVAL, "Music decoder requires an audio file path");
  }

  SF_INFO fileInformation{};
  const string ownedFilename{filename};
  std::unique_ptr<SNDFILE, SndFileCloser> file(
      ::sf_open(ownedFilename.c_str(), SFM_READ, &fileInformation));
  if (file == nullptr) {
    XWALK_HAL_ERROR(XWALK_EXCEPTION,
                    "Music audio file could not be opened by libsndfile");
  }
  const hal::boolean fileInformationSamplerateChannelsInvalid =
      static_cast<hal::boolean>(
          (fileInformation.samplerate <= 0) ||
          (fileInformation.channels <= 0) ||
          (fileInformation.channels >
           static_cast<int>(XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT)) ||
          (static_cast<unsigned long long>(fileInformation.samplerate) >
           static_cast<unsigned long long>(
               std::numeric_limits<uint32>::max())));
  if (fileInformationSamplerateChannelsInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music decoder returned invalid audio metadata");
  }

  const size channelCount = static_cast<size>(fileInformation.channels);
  const size maximumSampleCount = maximumDecodedPcmBytes / decodedSampleBytes;
  std::array<short,
             decoderReadFrames * XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT>
      sampleBuffer{};
  XWalkMusicAlsaAudioData audioData{};
  audioData.sampleRateHz = static_cast<uint32>(fileInformation.samplerate);
  audioData.channelCount = static_cast<uint8>(fileInformation.channels);

  while (true) {
    const sf_count_t frameCount =
        ::sf_readf_short(file.get(), sampleBuffer.data(),
                         static_cast<sf_count_t>(decoderReadFrames));
    if (frameCount < 0) {
      XWALK_HAL_ERROR(XWALK_RUNTIME, "Music audio file decoding failed");
    }
    if (frameCount == 0) {
      break;
    }
    const size sampleCount = static_cast<size>(frameCount) * channelCount;
    const hal::boolean sampleBufferTooLarge = static_cast<hal::boolean>(
        (sampleCount > maximumSampleCount) ||
        (audioData.pcmData.size() >
         ((maximumSampleCount - sampleCount) * decodedSampleBytes)));
    if (sampleBufferTooLarge) {
      XWALK_HAL_ERROR(XWALK_RANGE,
                      "Music decoded PCM exceeds the memory bound");
    }
    for (size sampleIndex = 0U; sampleIndex < sampleCount; ++sampleIndex) {
      const uint16 sampleBits = static_cast<uint16>(sampleBuffer[sampleIndex]);
      audioData.pcmData.push_back(static_cast<uint8>(sampleBits & 0x00FFU));
      audioData.pcmData.push_back(
          static_cast<uint8>((sampleBits >> 8U) & 0x00FFU));
    }
  }
  const hal::boolean fileAudioDataPcmDataInvalid = static_cast<hal::boolean>(
      (::sf_error(file.get()) != SF_ERR_NO_ERROR) || audioData.pcmData.empty());
  if (fileAudioDataPcmDataInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music audio file contains no complete decoded frames");
  }
  return audioData;
}

} /* namespace xwalk::hal */
