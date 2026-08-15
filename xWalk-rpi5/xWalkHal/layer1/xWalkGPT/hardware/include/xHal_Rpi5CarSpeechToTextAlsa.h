/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsa.h
 * @brief       Declares bounded ALSA capture and recognition adaptation.
 *
 * @details
 * Owns each ALSA capture handle for one synchronous listen operation and routes
 * captured PCM or audio files to a caller-owned recognition backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
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

#ifndef XHAL_RPI5CAR_SPEECH_TO_TEXT_ALSA_H
#define XHAL_RPI5CAR_SPEECH_TO_TEXT_ALSA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToTextAlsaTypes.h"

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
     * @class XWalkSpeechToTextAlsa
     * @brief Captures bounded ALSA microphone PCM and dispatches recognition.
     *
     * @details
     * Owns the configured microphone name, copies a complete operation table, and
     * uses an atomic cancellation request. Operation contexts remain non-owning.
     */
    class XWalkSpeechToTextAlsa final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Nullable non-owning context supplied to backend operations.
             *
             * @note A non-null object must outlive this adapter.
             */
            contextpointer operationContext{nullptr};

            /** @brief Complete capture and recognition operation table copied at construction. */
            XWalkSpeechToTextAlsaOperations operations{};

            /** @brief Owned non-empty ALSA capture device name. */
            string microphoneDeviceName{};

            /** @brief Atomic stop request observed between bounded capture reads. */
            atomicboolean cancellationRequested{false};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Combines real ALSA capture with one injected recognizer.
             *
             * @param[in] recognizerOperations
             * Operations whose four recognition callbacks must be non-null.
             *
             * @return Complete operation table using libasound for capture.
             */
            static XWalkSpeechToTextAlsaOperations
            systemCaptureOperations(const XWalkSpeechToTextAlsaOperations& recognizerOperations);

            /**
             * @brief Validates the complete backend operation table and device name.
             *
             * @param[in] backendOperations Operation table inspected for null callbacks.
             * @param[in] deviceName Non-empty ALSA capture device name.
             *
             * @throws std::invalid_argument If any callback is null or the device name is empty.
             */
            static void validateOperations(const XWalkSpeechToTextAlsaOperations& backendOperations,
                                           stringview deviceName);

            /**
             * @brief Converts one callback context into its required adapter.
             *
             * @param[in,out] context Non-null pointer to a live adapter.
             * @return Referenced adapter.
             * @throws std::invalid_argument If `context` is null.
             */
            static XWalkSpeechToTextAlsa& adapter(contextpointer context);

            /**
             * @brief Reports whether the selected recognizer can accept requests.
             *
             * @param[in,out] context Non-null pointer to a live adapter.
             * @return Recognizer readiness result.
             */
            static boolean readyCallback(contextpointer context);

            /**
             * @brief Captures bounded microphone PCM and dispatches recognition.
             *
             * @param[in,out] context Non-null pointer to a live adapter.
             * @param[in] timeoutMs Capture interval from 1 through 300,000 milliseconds.
             * @return Owned transcript, or an empty string after cancellation or silence.
             * @throws std::runtime_error If capture open, data validation, or recovery fails.
             */
            static string listenCallback(contextpointer context, uint32 timeoutMs);

            /**
             * @brief Dispatches one audio-file transcription request.
             *
             * @param[in,out] context Non-null pointer to a live adapter.
             * @param[in] filePath Non-empty path already validated by the coordinator.
             * @return Owned recognizer transcript, which may be empty.
             */
            static string fileCallback(contextpointer context, stringview filePath);

            /**
             * @brief Requests capture and recognition cancellation.
             *
             * @param[in,out] context Non-null pointer to a live adapter.
             *
             * @warning The recognizer cancellation operation must not throw.
             */
            static void stopCallback(contextpointer context);

            /**
             * @brief Opens and configures one real ALSA capture stream.
             *
             * @param[in,out] context Unused nullable recognition context.
             * @param[in] deviceName Non-empty ALSA capture device name.
             * @param[in] sampleRateHz Required sample rate in Hertz.
             * @param[in] channelCount Required interleaved channel count.
             * @param[in] periodFrames Requested maximum frames per read.
             * @return Owned opaque capture handle, or null when configuration fails.
             */
            static speechcapturehandle systemOpenCapture(contextpointer context,
                                                         stringview deviceName,
                                                         uint32 sampleRateHz,
                                                         uint8 channelCount,
                                                         uint32 periodFrames);

            /**
             * @brief Reads up to one requested frame count from real ALSA capture.
             *
             * @param[in,out] context Unused nullable recognition context.
             * @param[in,out] captureHandle Non-null handle returned by `systemOpenCapture`.
             * @param[out] pcmData Captured interleaved signed-16 PCM bytes.
             * @param[in] frameCount Requested frames from 1 through 1,024.
             * @return Positive frame count, zero, or a negative ALSA error value.
             */
            static int32 systemReadCapture(contextpointer context,
                                           speechcapturehandle captureHandle,
                                           bytevector& pcmData,
                                           size frameCount);

            /**
             * @brief Attempts ALSA recovery for one negative capture result.
             *
             * @param[in,out] context Unused nullable recognition context.
             * @param[in,out] captureHandle Non-null open ALSA capture handle.
             * @param[in] errorValue Negative ALSA result returned by capture.
             * @return `true` after successful recovery; otherwise `false`.
             */
            static boolean
            systemRecoverCapture(contextpointer context, speechcapturehandle captureHandle, int32 errorValue);

            /**
             * @brief Drops pending capture data and closes one real ALSA handle.
             *
             * @param[in,out] context Unused nullable recognition context.
             * @param[in,out] captureHandle Non-null owned ALSA capture handle.
             */
            static void systemCloseCapture(contextpointer context, speechcapturehandle captureHandle);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a real ALSA capture adapter with an injected recognizer.
             *
             * @param[in] deviceName Non-empty deployment-selected ALSA capture name.
             * @param[in,out] recognizerContext Non-owning recognizer context that outlives this adapter.
             * @param[in] recognizerOperations Operations containing all recognizer callbacks.
             * @throws std::invalid_argument If a callback is null or `deviceName` is empty.
             */
            XWalkSpeechToTextAlsa(stringview deviceName,
                                  contextpointer recognizerContext,
                                  const XWalkSpeechToTextAlsaOperations& recognizerOperations);

            /**
             * @brief Constructs an adapter with injected capture and recognition operations.
             *
             * @param[in,out] context Nullable non-owning context that must outlive this adapter.
             * @param[in] backendOperations Complete operation table copied by the adapter.
             * @param[in] deviceName Non-empty capture name forwarded to the open operation.
             * @throws std::invalid_argument If any callback is null or the device name is empty.
             */
            XWalkSpeechToTextAlsa(contextpointer context,
                                  const XWalkSpeechToTextAlsaOperations& backendOperations,
                                  stringview deviceName);

            /**
             * @brief Requests cancellation without releasing non-owning backend state.
             *
             * @warning The injected cancellation callback must not throw.
             */
            ~XWalkSpeechToTextAlsa();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkSpeechToTextAlsa(const XWalkSpeechToTextAlsa&) = delete;
            XWalkSpeechToTextAlsa& operator=(const XWalkSpeechToTextAlsa&) = delete;
            XWalkSpeechToTextAlsa(XWalkSpeechToTextAlsa&&) = delete;
            XWalkSpeechToTextAlsa& operator=(XWalkSpeechToTextAlsa&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Returns the complete callback table for `XWalkSpeechToText`.
             * @return Stateless callback table requiring this adapter as its context.
             */
            XWalkSpeechToTextCallbacks callbacks() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_TO_TEXT_ALSA_H */
