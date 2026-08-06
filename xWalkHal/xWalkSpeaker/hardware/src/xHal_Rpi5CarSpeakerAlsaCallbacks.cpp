/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaCallbacks.cpp
 * @brief       Implements shared-ALSA Speaker callback operations.
 *
 * @details
 * Routes output volume, bounded decoding, float32 stream creation, frame-range
 * conversion, stream closure, and task identifiers through the adapter.
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
 * @brief Enables output by applying the configured conservative volume.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 */
void XWalkSpeakerAlsa::enableOutput(contextpointer context)
{
    XWalkSpeakerAlsa& self = adapter(context);
    self.audioBackend->setVolume(self.playbackVolumePercentValue);
}

/**
 * @brief Completes logical output disable without releasing shared ALSA.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 *
 * @note
 * The shared owner may serve Music or speech consumers, so disabling one
 * Speaker controller must not close or mute the shared backend.
 */
void XWalkSpeakerAlsa::disableOutput(contextpointer context)
{
    static_cast<void>(adapter(context));
}

/**
 * @brief Decodes one supported file through the configured operation.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 *
 * @param[in] filePath
 * Existing regular-file path.
 *
 * @param[in] handler
 * Decoder family selected by `XWalkSpeaker`.
 *
 * @return
 * Validated bounded normalized audio data.
 */
XWalkSpeakerAudioData XWalkSpeakerAlsa::decodeAudio(contextpointer context,
    stringview filePath, XWalkSpeakerAudioHandler handler)
{
    XWalkSpeakerAlsa& self = adapter(context);
    XWalkSpeakerAudioData audioData =
        self.operations.decodeAudio(self.decoderContext, filePath, handler);
    validateDecodedAudio(audioData);
    return audioData;
}

/**
 * @brief Opens one shared ALSA float32 playback stream.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 *
 * @param[in] sampleRateHz
 * Positive sample rate in Hertz.
 *
 * @param[in] channelCount
 * Channel count from one through eight.
 *
 * @return
 * Non-null stream handle owned by the shared audio backend.
 */
speakerstreamhandle XWalkSpeakerAlsa::openStream(contextpointer context,
    uint32 sampleRateHz, uint8 channelCount)
{
    XWalkSpeakerAlsa& self = adapter(context);
    const XWalkAudioStreamConfiguration configuration{
        sampleRateHz,
        channelCount,
        XWalkAudioSampleFormat::Float32LittleEndian,
        XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT,
        XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
    return self.audioBackend->openStream(configuration);
}

/**
 * @brief Converts and writes one bounded range of decoded frames.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 *
 * @param[in,out] stream
 * Non-null shared-ALSA stream handle.
 *
 * @param[in] audioData
 * Immutable decoded interleaved audio.
 *
 * @param[in] firstFrame
 * Zero-based first frame to convert.
 *
 * @param[in] frameCount
 * Positive frame count not exceeding 1,024.
 *
 * @throws std::invalid_argument
 * If the range, frame count, metadata, or a selected sample is invalid.
 */
void XWalkSpeakerAlsa::writeStream(contextpointer context,
    speakerstreamhandle stream, const XWalkSpeakerAudioData& audioData,
    size firstFrame, size frameCount)
{
    static_assert(sizeof(float32) == 4U, "Speaker ALSA requires four-byte float32 samples");
    if ((frameCount == 0U) || (frameCount > XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT) ||
        (audioData.channelCount == 0U) ||
        (audioData.channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT))
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA write has invalid frame metadata");
    }
    const size channelCount = static_cast<size>(audioData.channelCount);
    const hal::boolean firstFrameOffsetOverflow =
        static_cast<hal::boolean>(
            firstFrame > (std::numeric_limits<size>::max() / channelCount));
    if (firstFrameOffsetOverflow)
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA first frame is unrepresentable");
    }
    const size firstSample = firstFrame * channelCount;
    const hal::boolean sampleCountOverflow =
        static_cast<hal::boolean>(
            frameCount > (std::numeric_limits<size>::max() / channelCount));
    if (sampleCountOverflow)
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA frame range is unrepresentable");
    }
    const size sampleCount = frameCount * channelCount;
    const hal::boolean firstSampleAudioDataSamplesInvalid =
        static_cast<hal::boolean>(
            (firstSample > audioData.samples.size()) ||
        (sampleCount > (audioData.samples.size() - firstSample)));
    if (firstSampleAudioDataSamplesInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA write exceeds decoded audio data");
    }

    bytevector pcmData{};
    pcmData.reserve(sampleCount * 4U);
    for (size sampleOffset = 0U; sampleOffset < sampleCount; ++sampleOffset)
    {
        const float64 sampleValue = audioData.samples[firstSample + sampleOffset];
        const hal::boolean sampleInvalid =
            static_cast<hal::boolean>(
                !XHAL_IS_FINITE(sampleValue) || (sampleValue < -1.0) || (sampleValue > 1.0));
        if (sampleInvalid)
        {
            XHAL_THROW_INVALID_ARGUMENT("Speaker ALSA sample is outside minus one through one");
        }
        const float32 floatSample = static_cast<float32>(sampleValue);
        uint32 sampleBits{};
        std::memcpy(&sampleBits, &floatSample, sizeof(floatSample));
        pcmData.push_back(static_cast<uint8>(sampleBits & 0xFFU));
        pcmData.push_back(static_cast<uint8>((sampleBits >> 8U) & 0xFFU));
        pcmData.push_back(static_cast<uint8>((sampleBits >> 16U) & 0xFFU));
        pcmData.push_back(static_cast<uint8>((sampleBits >> 24U) & 0xFFU));
    }
    adapter(context).audioBackend->writeFrames(stream, pcmData, frameCount);
}

/**
 * @brief Closes one stream owned by the shared ALSA backend.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 *
 * @param[in,out] stream
 * Non-null stream handle that becomes invalid after this call.
 */
void XWalkSpeakerAlsa::closeStream(contextpointer context,
    speakerstreamhandle stream)
{
    adapter(context).audioBackend->closeStream(stream);
}

/**
 * @brief Creates one deterministic process-local task identifier.
 *
 * @param[in,out] context
 * Non-null Speaker ALSA adapter context.
 *
 * @return
 * Non-empty identifier unique for this adapter's lifetime.
 *
 * @throws std::out_of_range
 * If the identifier sequence is exhausted.
 */
string XWalkSpeakerAlsa::createTaskId(contextpointer context)
{
    XWalkSpeakerAlsa& self = adapter(context);
    const mutexlock lock(self.identifierMutex);
    const hal::boolean selfTaskIdentifierMatched =
        static_cast<hal::boolean>(
            self.taskIdentifierValue == std::numeric_limits<uint64>::max());
    if (selfTaskIdentifierMatched)
    {
        XHAL_THROW_OUT_OF_RANGE("Speaker ALSA task identifier sequence is exhausted");
    }
    ++self.taskIdentifierValue;
    return string("speaker-alsa-") + std::to_string(self.taskIdentifierValue);
}

} /* namespace xwalk::hal */
