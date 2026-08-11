/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaDecode.cpp
 * @brief       Implements the built-in PCM WAVE decoder.
 *
 * @details
 * Reads RIFF chunks without assuming their order and returns complete signed
 * sixteen-bit little-endian PCM frames for the shared ALSA backend.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusicAlsa.h"

#include "xHal_Rpi5CarFileFunctions.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Returns the default PCM WAVE decoder operation table.
 *
 * @return
 * Operation table containing the built-in decoder.
 */
XWalkMusicAlsaOperations XWalkMusicAlsa::systemOperations() noexcept {
  return {&decodeWave};
}

/**
 * @brief Reads one little-endian unsigned sixteen-bit value from binary data.
 *
 * @param[in] data
 * Complete binary file contents.
 *
 * @param[in] byteOffset
 * Offset of the first of two required bytes.
 *
 * @return
 * Decoded unsigned value.
 *
 * @throws std::runtime_error
 * If two complete bytes are unavailable.
 */
uint16 XWalkMusicAlsa::readUint16(stringview data, size byteOffset) {
  const hal::boolean byteOffsetInvalid = static_cast<hal::boolean>(
      (byteOffset > data.size()) || ((data.size() - byteOffset) < 2U));
  if (byteOffsetInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music WAVE file contains a truncated value");
  }
  const uint16 lowByte = static_cast<uint8>(data[byteOffset]);
  const uint16 highByte = static_cast<uint8>(data[byteOffset + 1U]);
  return static_cast<uint16>(lowByte | static_cast<uint16>(highByte << 8U));
}

/**
 * @brief Reads one little-endian unsigned thirty-two-bit value from binary
 * data.
 *
 * @param[in] data
 * Complete binary file contents.
 *
 * @param[in] byteOffset
 * Offset of the first of four required bytes.
 *
 * @return
 * Decoded unsigned value.
 *
 * @throws std::runtime_error
 * If four complete bytes are unavailable.
 */
uint32 XWalkMusicAlsa::readUint32(stringview data, size byteOffset) {
  const hal::boolean byteOffsetInvalid = static_cast<hal::boolean>(
      (byteOffset > data.size()) || ((data.size() - byteOffset) < 4U));
  if (byteOffsetInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music WAVE file contains a truncated value");
  }
  uint32 value{};
  for (size byteIndex = 0U; byteIndex < 4U; ++byteIndex) {
    const uint32 byteValue = static_cast<uint8>(data[byteOffset + byteIndex]);
    const uint32 shiftBits = static_cast<uint32>(byteIndex * 8U);
    value |= byteValue << shiftBits;
  }
  return value;
}

/**
 * @brief Decodes one uncompressed sixteen-bit PCM WAVE file.
 *
 * @param[in,out] context
 * Unused nullable operation context.
 *
 * @param[in] filename
 * Path to an existing RIFF/WAVE file.
 *
 * @return
 * Complete signed sixteen-bit PCM frames and their stream metadata.
 *
 * @throws std::runtime_error
 * If the file is malformed, unsupported, empty, or incomplete.
 */
XWalkMusicAlsaAudioData XWalkMusicAlsa::decodeWave(contextpointer context,
                                                   stringview filename) {
  static_cast<void>(context);
  const string fileData = readFileContents(filesystempath{string(filename)});
  const stringview data{fileData};
  const hal::boolean predicateInvalid = static_cast<hal::boolean>(
      (data.size() < 12U) || (data.substr(0U, 4U) != "RIFF") ||
      (data.substr(8U, 4U) != "WAVE"));
  if (predicateInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music audio file is not a RIFF/WAVE stream");
  }

  uint16 formatTag{};
  uint16 channelCount{};
  uint32 sampleRateHz{};
  uint16 sampleBits{};
  stringview pcmBytes{};
  size chunkOffset = 12U;
  const hal::boolean processingLoopRequested{true};
  while (processingLoopRequested) {
    const hal::boolean chunkHeaderAvailable = static_cast<hal::boolean>(
        (chunkOffset <= data.size()) && ((data.size() - chunkOffset) >= 8U));
    if (chunkHeaderAvailable == false) {
      break;
    }
    const stringview chunkName = data.substr(chunkOffset, 4U);
    const size chunkBytes =
        static_cast<size>(readUint32(data, chunkOffset + 4U));
    const size payloadOffset = chunkOffset + 8U;
    const hal::boolean payloadOffsetChunkBytesInvalid =
        static_cast<hal::boolean>((payloadOffset > data.size()) ||
                                  (chunkBytes > (data.size() - payloadOffset)));
    if (payloadOffsetChunkBytesInvalid) {
      XWALK_HAL_ERROR(XWALK_RUNTIME,
                      "Music WAVE file contains a truncated chunk");
    }
    if (chunkName == "fmt ") {
      if (chunkBytes < XHAL_RPI5CAR_MUSIC_WAVE_FORMAT_CHUNK_BYTES) {
        XWALK_HAL_ERROR(XWALK_RUNTIME, "Music WAVE format chunk is too short");
      }
      formatTag = readUint16(data, payloadOffset);
      channelCount = readUint16(data, payloadOffset + 2U);
      sampleRateHz = readUint32(data, payloadOffset + 4U);
      sampleBits = readUint16(data, payloadOffset + 14U);
    } else if (chunkName == "data") {
      pcmBytes = data.substr(payloadOffset, chunkBytes);
    }
    const size paddingBytes = chunkBytes % 2U;
    const hal::boolean chunkSizeOverflow = static_cast<hal::boolean>(
        chunkBytes >
        (std::numeric_limits<size>::max() - payloadOffset - paddingBytes));
    if (chunkSizeOverflow) {
      XWALK_HAL_ERROR(XWALK_RUNTIME,
                      "Music WAVE chunk size is unrepresentable");
    }
    chunkOffset = payloadOffset + chunkBytes + paddingBytes;
  }

  const hal::boolean formatTagSampleBitsChannelCountInvalid =
      static_cast<hal::boolean>(
          (formatTag != XHAL_RPI5CAR_MUSIC_WAVE_PCM_FORMAT) ||
          (sampleBits != XHAL_RPI5CAR_MUSIC_WAVE_SAMPLE_BITS) ||
          (channelCount == 0U) ||
          (channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT) ||
          (sampleRateHz == 0U) || pcmBytes.empty());
  if (formatTagSampleBitsChannelCountInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music WAVE file requires non-empty sixteen-bit PCM audio");
  }

  XWalkMusicAlsaAudioData audioData{};
  audioData.sampleRateHz = sampleRateHz;
  audioData.channelCount = static_cast<uint8>(channelCount);
  audioData.pcmData.reserve(pcmBytes.size());
  for (const char pcmByte : pcmBytes) {
    audioData.pcmData.push_back(static_cast<uint8>(pcmByte));
  }
  validateAudioData(audioData);
  return audioData;
}

/**
 * @brief Validates decoded frame metadata and byte alignment.
 *
 * @param[in] audioData
 * Decoded signed sixteen-bit PCM data to validate.
 *
 * @throws std::runtime_error
 * If metadata, channel count, payload size, or alignment is invalid.
 */
void XWalkMusicAlsa::validateAudioData(
    const XWalkMusicAlsaAudioData &audioData) {
  if ((audioData.sampleRateHz == 0U) || (audioData.channelCount == 0U) ||
      (audioData.channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT)) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music decoder returned invalid audio metadata");
  }
  const size channelCount = static_cast<size>(audioData.channelCount);
  const size bytesPerFrame = channelCount * XHAL_RPI5CAR_MUSIC_SAMPLE_BYTES;
  const hal::boolean audioDataPcmDataBytesPerFrameInvalid =
      static_cast<hal::boolean>(
          audioData.pcmData.empty() ||
          ((audioData.pcmData.size() % bytesPerFrame) != 0U));
  if (audioDataPcmDataBytesPerFrameInvalid) {
    XWALK_HAL_ERROR(XWALK_RUNTIME,
                    "Music decoder returned incomplete PCM frames");
  }
}

} /* namespace xwalk::hal */
