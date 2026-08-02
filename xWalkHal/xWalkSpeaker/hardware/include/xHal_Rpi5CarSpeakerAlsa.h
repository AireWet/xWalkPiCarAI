/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsa.h
 * @brief       Declares the shared-ALSA adapter for Speaker callbacks.
 *
 * @details
 * Connects bounded decoding and floating-point Speaker streams to one caller-
 * owned shared ALSA backend while retaining only task-identifier state.
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

#ifndef XHAL_RPI5CAR_SPEAKER_ALSA_H
#define XHAL_RPI5CAR_SPEAKER_ALSA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarSpeakerAlsaTypes.h"

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
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkSpeakerAlsa
 * @brief Connects every Speaker backend callback to decoding and shared ALSA.
 *
 * @details
 * Observes one caller-owned `XWalkAudioAlsa`, validates bounded decoded data,
 * converts requested frame ranges to float32 little-endian PCM, and creates
 * deterministic identifiers. `XWalkSpeaker` continues to own all task workers.
 */
class XWalkSpeakerAlsa final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning audio owner that must outlive this adapter and every consumer. */
        XWalkAudioAlsa* audioBackend{nullptr};
        /** @brief Nullable non-owning context supplied to the decoder callback. */
        contextpointer decoderContext{nullptr};
        /** @brief Complete decoder operation table copied during construction. */
        XWalkSpeakerAlsaOperations operations{};
        /** @brief Mixer volume applied whenever Speaker output becomes enabled. */
        uint8 playbackVolumePercentValue{};
        /** @brief Mutex serializing task-identifier generation. */
        mutexhandle identifierMutex{};
        /** @brief Monotonic task sequence used to create non-empty identifiers. */
        uint64 taskIdentifierValue{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Returns the built-in bounded PCM WAVE decoder operation table.
         *
         * @return
         * Operation table containing the built-in decoder.
         */
        static XWalkSpeakerAlsaOperations systemOperations() noexcept;

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
        static XWalkSpeakerAudioData decodeWave(contextpointer context,
            stringview filePath, XWalkSpeakerAudioHandler handler);

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
        static uint16 readUint16(stringview data, size byteOffset);

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
        static uint32 readUint32(stringview data, size byteOffset);

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
        static void validateDecodedAudio(const XWalkSpeakerAudioData& audioData);

        /**
         * @brief Converts a callback context to its required adapter.
         *
         * @param[in,out] context
         * Non-null adapter context supplied to `XWalkSpeaker`.
         *
         * @return
         * Adapter referenced by the callback context.
         *
         * @throws std::invalid_argument
         * If the context is null.
         */
        static XWalkSpeakerAlsa& adapter(contextpointer context);

        /**
         * @brief Enables output by applying the configured conservative volume.
         *
         * @param[in,out] context
         * Non-null Speaker ALSA adapter context.
         */
        static void enableOutput(contextpointer context);

        /**
         * @brief Completes logical output disable without releasing shared ALSA.
         *
         * @param[in,out] context
         * Non-null Speaker ALSA adapter context.
         */
        static void disableOutput(contextpointer context);

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
        static XWalkSpeakerAudioData decodeAudio(contextpointer context,
            stringview filePath, XWalkSpeakerAudioHandler handler);

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
        static speakerstreamhandle openStream(contextpointer context,
            uint32 sampleRateHz, uint8 channelCount);

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
         */
        static void writeStream(contextpointer context, speakerstreamhandle stream,
            const XWalkSpeakerAudioData& audioData, size firstFrame, size frameCount);

        /**
         * @brief Closes one stream owned by the shared ALSA backend.
         *
         * @param[in,out] context
         * Non-null Speaker ALSA adapter context.
         *
         * @param[in,out] stream
         * Non-null stream handle that becomes invalid after this call.
         */
        static void closeStream(contextpointer context, speakerstreamhandle stream);

        /**
         * @brief Creates one deterministic process-local task identifier.
         *
         * @param[in,out] context
         * Non-null Speaker ALSA adapter context.
         *
         * @return
         * Non-empty identifier unique for this adapter's lifetime.
         */
        static string createTaskId(contextpointer context);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs an adapter with the built-in bounded PCM WAVE decoder.
         *
         * @param[in,out] sharedAudioBackend
         * Caller-owned shared ALSA backend that must outlive this adapter.
         *
         * @param[in] playbackVolumePercent
         * Mixer volume from zero through one hundred percent applied on enable.
         *
         * @throws std::out_of_range
         * If the volume exceeds one hundred percent.
         */
        explicit XWalkSpeakerAlsa(XWalkAudioAlsa& sharedAudioBackend,
            uint8 playbackVolumePercent = 50U);

        /**
         * @brief Constructs an adapter with an injected optional decoder.
         *
         * @param[in,out] sharedAudioBackend
         * Caller-owned shared ALSA backend that must outlive this adapter.
         *
         * @param[in,out] context
         * Nullable non-owning decoder context that must outlive this adapter.
         *
         * @param[in] backendOperations
         * Operation table containing one non-null bounded decoder callback.
         *
         * @param[in] playbackVolumePercent
         * Mixer volume from zero through one hundred percent applied on enable.
         *
         * @throws std::invalid_argument
         * If the decoder callback is null.
         *
         * @throws std::out_of_range
         * If the volume exceeds one hundred percent.
         */
        XWalkSpeakerAlsa(XWalkAudioAlsa& sharedAudioBackend, contextpointer context,
            const XWalkSpeakerAlsaOperations& backendOperations,
            uint8 playbackVolumePercent = 50U);

        /** @brief Destroys the adapter without releasing its caller-owned audio backend. */
        ~XWalkSpeakerAlsa();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables copying of the non-owning callback binding. */
        XWalkSpeakerAlsa(const XWalkSpeakerAlsa&) = delete;
        /** @brief Disables copy assignment of shared dependency state. */
        XWalkSpeakerAlsa& operator=(const XWalkSpeakerAlsa&) = delete;
        /** @brief Disables moving because callbacks retain adapter identity. */
        XWalkSpeakerAlsa(XWalkSpeakerAlsa&&) = delete;
        /** @brief Disables move assignment because callback identity must remain stable. */
        XWalkSpeakerAlsa& operator=(XWalkSpeakerAlsa&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Returns the complete callback table for this adapter context.
         *
         * @return
         * Seven non-null callbacks suitable for `XWalkSpeaker` construction.
         */
        XWalkSpeakerCallbacks callbacks() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEAKER_ALSA_H */
