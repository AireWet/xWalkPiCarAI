/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaDecode.cpp
 * @brief       Implements bounded Speaker decoding and validation.
 *
 * @details
 * Parses uncompressed sixteen-bit PCM RIFF/WAVE files after enforcing an input
 * byte bound and validates every built-in or optionally decoded sample vector.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker ALSA Adapter
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

#include "xHal_Rpi5CarSpeakerAlsa.h"

#include "xHal_Rpi5CarFileFunctions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Returns the built-in bounded PCM WAVE decoder operation table.
 *
 * @return
 * Operation table containing the built-in decoder.
 */
XWalkSpeakerAlsaOperations XWalkSpeakerAlsa::systemOperations() noexcept
{
    return {&decodeWave};
}

/**
 * @brief Reads one little-endian unsigned sixteen-bit value.
 *
 * @param[in] data
 * Complete bounded binary file contents.
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
uint16 XWalkSpeakerAlsa::readUint16(stringview data, size byteOffset)
{
    const hal::boolean byteOffsetInvalid =
        static_cast<hal::boolean>(
            (byteOffset > data.size()) || ((data.size() - byteOffset) < 2U));
    if (byteOffsetInvalid)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker WAVE file contains a truncated value");
    }
    const uint16 lowByte = static_cast<uint8>(data[byteOffset]);
    const uint16 highByte = static_cast<uint8>(data[byteOffset + 1U]);
    return static_cast<uint16>(lowByte | static_cast<uint16>(highByte << 8U));
}

/**
 * @brief Reads one little-endian unsigned thirty-two-bit value.
 *
 * @param[in] data
 * Complete bounded binary file contents.
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
uint32 XWalkSpeakerAlsa::readUint32(stringview data, size byteOffset)
{
    const hal::boolean byteOffsetInvalid =
        static_cast<hal::boolean>(
            (byteOffset > data.size()) || ((data.size() - byteOffset) < 4U));
    if (byteOffsetInvalid)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker WAVE file contains a truncated value");
    }
    uint32 value{};
    for (size byteIndex = 0U; byteIndex < 4U; ++byteIndex)
    {
        const uint32 byteValue = static_cast<uint8>(data[byteOffset + byteIndex]);
        const uint32 shiftBits = static_cast<uint32>(byteIndex * 8U);
        value |= byteValue << shiftBits;
    }
    return value;
}

/**
 * @brief Decodes one bounded uncompressed sixteen-bit PCM WAVE file.
 *
 * @param[in,out] context
 * Unused nullable decoder context.
 *
 * @param[in] filePath
 * Existing regular audio-file path.
 *
 * @param[in] handler
 * SoundFile family required by the built-in decoder.
 *
 * @return
 * Normalized interleaved samples with rate and channel metadata.
 *
 * @throws std::runtime_error
 * If the file is malformed, unsupported, empty, or incomplete.
 *
 * @throws std::out_of_range
 * If input bytes or decoded samples exceed their configured bounds.
 */
XWalkSpeakerAudioData XWalkSpeakerAlsa::decodeWave(contextpointer context,
    stringview filePath, XWalkSpeakerAudioHandler handler)
{
    static_cast<void>(context);
    if (handler != XWalkSpeakerAudioHandler::SoundFile)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker compressed format requires an optional decoder");
    }
    const filesystempath audioPath{string(filePath)};
    const uint64 fileBytes = filesystemFileSize(audioPath);
    if (fileBytes > XHAL_RPI5CAR_SPEAKER_MAXIMUM_INPUT_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("Speaker audio file exceeds the decoder input bound");
    }
    const string fileData = readFileContents(audioPath);
    const stringview data{fileData};
    const hal::boolean predicateInvalid =
        static_cast<hal::boolean>(
            (data.size() < 12U) || (data.substr(0U, 4U) != "RIFF") ||
        (data.substr(8U, 4U) != "WAVE"));
    if (predicateInvalid)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker built-in decoder requires a PCM WAVE file");
    }

    uint16 formatTag{};
    uint16 channelCount{};
    uint32 sampleRateHz{};
    uint16 sampleBits{};
    stringview pcmBytes{};
    size chunkOffset = 12U;
    const hal::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const hal::boolean chunkHeaderAvailable =
            static_cast<hal::boolean>(
                (chunkOffset <= data.size()) && ((data.size() - chunkOffset) >= 8U));
        if (chunkHeaderAvailable == false)
        {
            break;
        }
        const stringview chunkName = data.substr(chunkOffset, 4U);
        const size chunkBytes = static_cast<size>(readUint32(data, chunkOffset + 4U));
        const size payloadOffset = chunkOffset + 8U;
        const hal::boolean payloadOffsetChunkBytesInvalid =
            static_cast<hal::boolean>(
                (payloadOffset > data.size()) || (chunkBytes > (data.size() - payloadOffset)));
        if (payloadOffsetChunkBytesInvalid)
        {
            XHAL_THROW_RUNTIME_ERROR("Speaker WAVE file contains a truncated chunk");
        }
        if (chunkName == "fmt ")
        {
            if (chunkBytes < 16U)
            {
                XHAL_THROW_RUNTIME_ERROR("Speaker WAVE format chunk is too short");
            }
            formatTag = readUint16(data, payloadOffset);
            channelCount = readUint16(data, payloadOffset + 2U);
            sampleRateHz = readUint32(data, payloadOffset + 4U);
            sampleBits = readUint16(data, payloadOffset + 14U);
        }
        else if (chunkName == "data")
        {
            pcmBytes = data.substr(payloadOffset, chunkBytes);
        }
        const size paddingBytes = chunkBytes % 2U;
        const size maximumChunkBytes = std::numeric_limits<size>::max() - payloadOffset;
        if ((chunkBytes > maximumChunkBytes) ||
            (paddingBytes > (maximumChunkBytes - chunkBytes)))
        {
            XHAL_THROW_RUNTIME_ERROR("Speaker WAVE chunk size is unrepresentable");
        }
        chunkOffset = payloadOffset + chunkBytes + paddingBytes;
    }

    const hal::boolean formatTagSampleBitsChannelCountInvalid =
        static_cast<hal::boolean>(
            (formatTag != XHAL_RPI5CAR_SPEAKER_WAVE_PCM_FORMAT) ||
        (sampleBits != XHAL_RPI5CAR_SPEAKER_WAVE_SAMPLE_BITS) ||
        (channelCount == 0U) || (channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT) ||
        (sampleRateHz == 0U) || pcmBytes.empty() || ((pcmBytes.size() % 2U) != 0U));
    if (formatTagSampleBitsChannelCountInvalid)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker WAVE decoder requires non-empty sixteen-bit PCM audio");
    }

    const size sampleCount = pcmBytes.size() / 2U;
    if (sampleCount > XHAL_RPI5CAR_SPEAKER_MAXIMUM_DECODED_SAMPLE_COUNT)
    {
        XHAL_THROW_OUT_OF_RANGE("Speaker decoded sample count exceeds its bound");
    }
    XWalkSpeakerAudioData audioData{};
    audioData.samples.reserve(sampleCount);
    audioData.sampleRateHz = sampleRateHz;
    audioData.channelCount = static_cast<uint8>(channelCount);
    for (size sampleIndex = 0U; sampleIndex < sampleCount; ++sampleIndex)
    {
        const uint16 packedSample = readUint16(pcmBytes, sampleIndex * 2U);
        const int32 signedSample = (packedSample <= 0x7FFFU) ?
            static_cast<int32>(packedSample) : static_cast<int32>(packedSample) - 65'536;
        const float64 sampleValue = static_cast<float64>(signedSample);
        audioData.samples.push_back(
            sampleValue / XHAL_RPI5CAR_SPEAKER_PCM16_NORMALIZATION_DIVISOR);
    }
    validateDecodedAudio(audioData);
    return audioData;
}

/**
 * @brief Validates decoded metadata, sample count, alignment, and finiteness.
 *
 * @param[in] audioData
 * Decoded interleaved samples returned by any decoder operation.
 *
 * @throws std::runtime_error
 * If metadata, alignment, or a sample is invalid.
 *
 * @throws std::out_of_range
 * If the decoded sample bound is exceeded.
 */
void XWalkSpeakerAlsa::validateDecodedAudio(const XWalkSpeakerAudioData& audioData)
{
    if ((audioData.sampleRateHz == 0U) || (audioData.channelCount == 0U) ||
        (audioData.channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT))
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker decoder returned invalid audio metadata");
    }
    const hal::boolean samplesEmpty =
        static_cast<hal::boolean>(
            audioData.samples.empty());
    if (samplesEmpty)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker decoder returned no audio samples");
    }
    const hal::boolean samplesTooLarge =
        static_cast<hal::boolean>(
            audioData.samples.size() > XHAL_RPI5CAR_SPEAKER_MAXIMUM_DECODED_SAMPLE_COUNT);
    if (samplesTooLarge)
    {
        XHAL_THROW_OUT_OF_RANGE("Speaker decoder returned an invalid bounded sample count");
    }
    const size channelCount = static_cast<size>(audioData.channelCount);
    const hal::boolean audioDataSamplesChannelCountDifferent =
        static_cast<hal::boolean>(
            (audioData.samples.size() % channelCount) != 0U);
    if (audioDataSamplesChannelCountDifferent)
    {
        XHAL_THROW_RUNTIME_ERROR("Speaker decoder returned incomplete interleaved frames");
    }
    for (const float64 sampleValue : audioData.samples)
    {
        const hal::boolean sampleInvalid =
            static_cast<hal::boolean>(
                !XHAL_IS_FINITE(sampleValue) || (sampleValue < -1.0) || (sampleValue > 1.0));
        if (sampleInvalid)
        {
            XHAL_THROW_RUNTIME_ERROR("Speaker decoder returned a sample outside minus one through one");
        }
    }
}

} /* namespace xwalk::hal */
