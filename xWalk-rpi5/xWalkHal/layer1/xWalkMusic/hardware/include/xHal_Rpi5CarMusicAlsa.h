/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsa.h
 * @brief       Declares the shared-ALSA adapter for music callbacks.
 *
 * @details
 * Adapts file playback, transport, mixer volume, and generated PCM tone calls
 * to one caller-owned shared ALSA backend and one injected file decoder.
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

#ifndef XHAL_RPI5CAR_MUSIC_ALSA_H
#define XHAL_RPI5CAR_MUSIC_ALSA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarMusicAlsaTypes.h"

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
     * @class XWalkMusicAlsa
     * @brief Connects every music callback to shared ALSA playback and mixer state.
     *
     * @details
     * Stores a non-owning ALSA backend pointer, owns decoded worker data and two
     * bounded workers, and provides a complete callback table for `XWalkMusic`.
     */
    class XWalkMusicAlsa final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning backend pointer that remains valid for this adapter's lifetime. */
            XWalkAudioAlsa* audioBackend{nullptr};
            /** @brief Nullable non-owning context passed to the decoder callback. */
            contextpointer decoderContext{nullptr};
            /** @brief Complete file-decoder operation table copied during construction. */
            XWalkMusicAlsaOperations operations{};
            /** @brief Mutex protecting worker requests and retained decoded audio. */
            mutable mutexhandle stateMutex{};
            /** @brief Joinable worker for the most recent background sound effect. */
            threadhandle soundWorker{};
            /** @brief Joinable worker for the current streamed-music operation. */
            threadhandle musicWorker{};
            /** @brief Decoded background sound retained until its worker is joined. */
            XWalkMusicAlsaAudioData soundAudio{};
            /** @brief Decoded streamed music retained until its worker is joined. */
            XWalkMusicAlsaAudioData musicAudio{};
            /** @brief Initial streamed-music frame calculated from the requested offset. */
            size musicStartFrame{};
            /** @brief Number of additional complete streamed-music repetitions. */
            int32 musicLoopCount{};
            /** @brief Requests that the background sound worker stop at a period boundary. */
            boolean soundStopRequested{};
            /** @brief Requests that the streamed-music worker stop at a period boundary. */
            boolean musicStopRequested{};
            /** @brief Requests that streamed music wait without writing frames. */
            boolean musicPauseRequested{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Returns the default PCM WAVE decoder operation table.
             *
             * @return
             * Operation table containing the built-in decoder.
             */
            static XWalkMusicAlsaOperations systemOperations() noexcept;
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
            static XWalkMusicAlsaAudioData decodeWave(contextpointer context, stringview filename);
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
            static uint16 readUint16(stringview data, size byteOffset);
            /**
             * @brief Reads one little-endian unsigned thirty-two-bit value from binary data.
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
            static uint32 readUint32(stringview data, size byteOffset);
            /**
             * @brief Validates decoded frame metadata and byte alignment.
             *
             * @param[in] audioData
             * Decoded signed sixteen-bit PCM data to validate.
             *
             * @throws std::runtime_error
             * If metadata, channel count, payload size, or alignment is invalid.
             */
            static void validateAudioData(const XWalkMusicAlsaAudioData& audioData);
            /**
             * @brief Converts normalized music volume to mixer percent.
             *
             * @param[in] normalizedVolume
             * Finite normalized volume in the inclusive range zero through one.
             *
             * @return
             * Rounded mixer volume from zero through one hundred percent.
             *
             * @throws std::invalid_argument
             * If the value is not finite.
             *
             * @throws std::out_of_range
             * If the value is outside the normalized range.
             */
            static uint8 volumePercent(float64 normalizedVolume);
            /**
             * @brief Converts a callback context into its required adapter.
             *
             * @param[in,out] context
             * Non-null adapter pointer supplied to `XWalkMusic`.
             *
             * @return
             * Adapter referenced by the callback context.
             *
             * @throws std::invalid_argument
             * If the context is null.
             */
            static XWalkMusicAlsa& adapter(contextpointer context);
            /**
             * @brief Writes decoded PCM from one starting frame for the requested repetitions.
             *
             * @param[in] audioData
             * Valid decoded PCM data retained for the complete call.
             *
             * @param[in] startFrame
             * Initial frame below the total frame count.
             *
             * @param[in] loopCount
             * Non-negative number of additional complete repetitions.
             *
             * @param[in] observeMusicState
             * `true` to observe streamed-music pause and stop requests.
             *
             * @param[in] observeSoundState
             * `true` to observe the background-sound stop request.
             *
             * @throws std::runtime_error
             * If the shared ALSA backend cannot open, write, recover, or close playback.
             */
            void writeAudio(const XWalkMusicAlsaAudioData& audioData,
                            size startFrame,
                            int32 loopCount,
                            boolean observeMusicState,
                            boolean observeSoundState);
            /**
             * @brief Runs the single retained background sound worker.
             *
             * @post
             * Playback completes or observes a stop request.
             */
            void soundPlaybackLoop() noexcept;
            /**
             * @brief Runs the single retained streamed-music worker.
             *
             * @post
             * Playback completes its loops or observes a stop request.
             */
            void musicPlaybackLoop() noexcept;
            /**
             * @brief Starts one worker while blocking process-cancellation signals in the new thread.
             *
             * @param[in] workerOperation
             * Non-null playback member function executed by the new worker.
             *
             * @return
             * Joinable worker whose inherited signal mask blocks SIGINT and SIGTERM.
             *
             * @throws std::runtime_error
             * If the calling thread's signal mask cannot be changed or restored.
             */
            threadhandle startWorker(void (XWalkMusicAlsa::*workerOperation)() noexcept);
            /**
             * @brief Requests and joins the retained background sound worker.
             *
             * @post
             * No background sound worker remains joinable.
             */
            void stopSoundWorker();
            /**
             * @brief Requests and joins the retained streamed-music worker.
             *
             * @post
             * No streamed-music worker remains joinable.
             */
            void stopMusicWorker();
            /**
             * @brief Implements the output-enable callback after ALSA construction.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             */
            static void enableOutput(contextpointer context);
            /**
             * @brief Implements synchronous sound-effect playback.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             *
             * @param[in] filename
             * Non-empty file path decoded before playback.
             *
             * @param[in] normalizedVolume
             * Optional normalized mixer volume from zero through one.
             */
            static void playSound(contextpointer context, stringview filename, optionalfloat64 normalizedVolume);
            /**
             * @brief Implements background sound-effect playback.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             *
             * @param[in] filename
             * Non-empty file path decoded before worker creation.
             *
             * @param[in] normalizedVolume
             * Optional normalized mixer volume from zero through one.
             */
            static void
            playSoundBackground(contextpointer context, stringview filename, optionalfloat64 normalizedVolume);
            /**
             * @brief Implements streamed-music playback.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             *
             * @param[in] filename
             * Non-empty file path decoded before worker creation.
             *
             * @param[in] loops
             * Non-negative number of additional repetitions.
             *
             * @param[in] startSeconds
             * Finite non-negative initial offset in seconds.
             */
            static void playMusic(contextpointer context, stringview filename, int32 loops, float64 startSeconds);
            /**
             * @brief Implements shared ALSA mixer volume control.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             *
             * @param[in] normalizedVolume
             * Finite normalized volume from zero through one.
             */
            static void setMusicVolume(contextpointer context, float64 normalizedVolume);
            /**
             * @brief Implements streamed-music stop control.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             */
            static void stopMusic(contextpointer context);
            /**
             * @brief Implements streamed-music pause control.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             */
            static void pauseMusic(contextpointer context);
            /**
             * @brief Implements streamed-music resume control.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             */
            static void resumeMusic(contextpointer context);
            /**
             * @brief Implements decoded sound duration measurement.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             *
             * @param[in] filename
             * Non-empty file path decoded for measurement.
             *
             * @return
             * Positive decoded duration in seconds.
             */
            static float64 getSoundLength(contextpointer context, stringview filename);
            /**
             * @brief Implements generated signed sixteen-bit PCM tone playback.
             *
             * @param[in,out] context
             * Non-null music ALSA adapter context.
             *
             * @param[in] pcmData
             * Complete signed sixteen-bit PCM bytes.
             *
             * @param[in] sampleRateHz
             * Positive sample rate in Hertz.
             *
             * @param[in] channelCount
             * Interleaved channel count from one through eight.
             */
            static void
            playTone(contextpointer context, const bytevector& pcmData, uint32 sampleRateHz, uint8 channelCount);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs an adapter using the built-in PCM WAVE decoder.
             *
             * @param[in,out] sharedAudioBackend
             * Caller-owned ALSA backend that must outlive this adapter and its consumers.
             */
            explicit XWalkMusicAlsa(XWalkAudioAlsa& sharedAudioBackend);

            /**
             * @brief Constructs an adapter using an injected file decoder.
             *
             * @param[in,out] sharedAudioBackend
             * Caller-owned ALSA backend that must outlive this adapter and its consumers.
             *
             * @param[in,out] context
             * Nullable non-owning decoder context that must outlive this adapter.
             *
             * @param[in] backendOperations
             * Operation table containing one non-null decoder callback.
             *
             * @throws std::invalid_argument
             * If the decoder callback is null.
             */
            XWalkMusicAlsa(XWalkAudioAlsa& sharedAudioBackend,
                           contextpointer context,
                           const XWalkMusicAlsaOperations& backendOperations);

            /**
             * @brief Stops and joins both retained playback workers.
             *
             * @warning
             * ALSA operations invoked during worker shutdown must not throw.
             */
            ~XWalkMusicAlsa();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying because workers retain adapter identity. */
            XWalkMusicAlsa(const XWalkMusicAlsa&) = delete;
            /** @brief Disables copy assignment because dependencies are non-owning. */
            XWalkMusicAlsa& operator=(const XWalkMusicAlsa&) = delete;
            /** @brief Disables moving because callback contexts retain adapter identity. */
            XWalkMusicAlsa(XWalkMusicAlsa&&) = delete;
            /** @brief Disables move assignment because worker state cannot relocate safely. */
            XWalkMusicAlsa& operator=(XWalkMusicAlsa&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Returns the complete callback table bound through this adapter's address.
             *
             * @return
             * Ten non-null callbacks suitable for constructing `XWalkMusic` with this object as context.
             */
            XWalkMusicCallbacks callbacks() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_MUSIC_ALSA_H */
