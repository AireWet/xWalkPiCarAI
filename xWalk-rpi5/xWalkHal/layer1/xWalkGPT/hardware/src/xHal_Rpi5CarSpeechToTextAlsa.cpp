/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsa.cpp
 * @brief       Implements bounded capture, recognition, and callback routing.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 * @copyright Copyright (c) 2026 Joxy John. All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include "xHal_Rpi5CarSpeechCaptureGuard.h"
#include "xHal_Rpi5CarSpeechRecognitionGuard.h"

#include "xHal_Rpi5CarTrace.h"
/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /**
     * @brief Constructs a real ALSA capture adapter with an injected recognizer.
     *
     * @param[in] deviceName Non-empty deployment-selected ALSA capture name.
     * @param[in,out] recognizerContext Nullable non-owning context that must
     * outlive this adapter.
     * @param[in] recognizerOperations Operations containing four non-null
     * recognizer callbacks.
     * @throws std::invalid_argument If a recognizer callback is null or the device
     * name is empty.
     */
    XWalkSpeechToTextAlsa::XWalkSpeechToTextAlsa(stringview deviceName,
                                                 contextpointer recognizerContext,
                                                 const XWalkSpeechToTextAlsaOperations& recognizerOperations,
                                                 const XWalkSpeechEndpointConfiguration& endpointSettings)
        : XWalkSpeechToTextAlsa(
              recognizerContext, systemCaptureOperations(recognizerOperations), deviceName, endpointSettings)
    {
    }

    /**
     * @brief Constructs an adapter with fully injected operations.
     *
     * @param[in,out] context Nullable non-owning context that must outlive this
     * adapter.
     * @param[in] backendOperations Complete capture and recognition operation
     * table.
     * @param[in] deviceName Non-empty capture name forwarded to the open operation.
     * @throws std::invalid_argument If any callback is null or the device name is
     * empty.
     */
    XWalkSpeechToTextAlsa::XWalkSpeechToTextAlsa(contextpointer context,
                                                 const XWalkSpeechToTextAlsaOperations& backendOperations,
                                                 stringview deviceName,
                                                 const XWalkSpeechEndpointConfiguration& endpointSettings)
        : operationContext(context), operations(backendOperations), microphoneDeviceName(deviceName),
          endpointConfiguration(endpointSettings)
    {
        validateOperations(operations, microphoneDeviceName);
        validateEndpointConfiguration(endpointConfiguration);
    }

    /**
     * @brief Requests cancellation without releasing non-owning backend state.
     *
     * @warning The injected cancellation callback must not throw.
     */
    XWalkSpeechToTextAlsa::~XWalkSpeechToTextAlsa()
    {
        stopCallback(this);
    }

    /**
     * @brief Returns all coordinator callbacks bound through this adapter.
     * @return Complete stateless callback table requiring this adapter as context.
     */
    XWalkSpeechToTextCallbacks XWalkSpeechToTextAlsa::callbacks() const noexcept
    {
        return {&readyCallback, &listenCallback, &fileCallback, &stopCallback};
    }

    /**
     * @brief Copies recognizer operations and installs real ALSA capture
     * operations.
     *
     * @param[in] recognizerOperations Operations containing recognizer callbacks.
     * @return Combined recognition and libasound capture operation table.
     */
    XWalkSpeechToTextAlsaOperations
    XWalkSpeechToTextAlsa::systemCaptureOperations(const XWalkSpeechToTextAlsaOperations& recognizerOperations)
    {
        XWalkSpeechToTextAlsaOperations combined = recognizerOperations;
        combined.openCapture = &systemOpenCapture;
        combined.readCapture = &systemReadCapture;
        combined.recoverCapture = &systemRecoverCapture;
        combined.closeCapture = &systemCloseCapture;
        return combined;
    }

    /**
     * @brief Validates every operation and the configured microphone name.
     *
     * @param[in] backendOperations Operation table inspected for null callbacks.
     * @param[in] deviceName ALSA capture device name that must not be empty.
     * @throws std::invalid_argument If any callback is null or `deviceName` is
     * empty.
     */
    void XWalkSpeechToTextAlsa::validateOperations(const XWalkSpeechToTextAlsaOperations& backendOperations,
                                                   stringview deviceName)
    {
        const boolean captureMissing =
            (backendOperations.openCapture == nullptr) || (backendOperations.readCapture == nullptr) ||
            (backendOperations.recoverCapture == nullptr) || (backendOperations.closeCapture == nullptr);
        const boolean recognizerMissing =
            (backendOperations.recognizerReady == nullptr) || (backendOperations.recognizePcm == nullptr) ||
            (backendOperations.startRecognition == nullptr) || (backendOperations.feedRecognition == nullptr) ||
            (backendOperations.finishRecognition == nullptr) || (backendOperations.releaseRecognition == nullptr) ||
            (backendOperations.recognizeFile == nullptr) || (backendOperations.cancelRecognition == nullptr);
        const hal::boolean speechConfigurationInvalid =
            static_cast<hal::boolean>(captureMissing || recognizerMissing || deviceName.empty());
        if (speechConfigurationInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Speech ALSA backend requires complete operations and a device");
        }
    }

    /**
     * @brief Validates bounded fallback endpoint and trace settings.
     * @param[in] configuration Endpoint settings selected by deployment.
     * @throws std::out_of_range If durations are zero, unordered, or exceed the listen bound.
     */
    void XWalkSpeechToTextAlsa::validateEndpointConfiguration(const XWalkSpeechEndpointConfiguration& configuration)
    {
        const boolean durationInvalid =
            (configuration.minimumSpeechMilliseconds == 0U) || (configuration.trailingSilenceMilliseconds == 0U) ||
            (configuration.maximumUtteranceMilliseconds == 0U) ||
            (configuration.minimumSpeechMilliseconds > configuration.maximumUtteranceMilliseconds) ||
            (configuration.trailingSilenceMilliseconds > configuration.maximumUtteranceMilliseconds) ||
            (configuration.maximumUtteranceMilliseconds > XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS);
        const boolean thresholdInvalid =
            (configuration.silencePeakThreshold == 0U) || (configuration.silencePeakThreshold > 32'767U);
        const hal::boolean endpointConfigurationInvalid =
            static_cast<hal::boolean>(durationInvalid || thresholdInvalid);
        if (endpointConfigurationInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Speech fallback endpoint configuration is outside its range");
        }
    }

    /**
     * @brief Classifies one complete signed-sixteen little-endian PCM period.
     * @param[in] pcmData Non-empty complete mono PCM bytes.
     * @param[in] peakThreshold Inclusive absolute sample-magnitude threshold.
     * @return `true` when every decoded sample is at or below `peakThreshold`.
     */
    boolean XWalkSpeechToTextAlsa::periodIsLowLevel(const bytevector& pcmData, uint32 peakThreshold)
    {
        const hal::boolean pcmMalformed = static_cast<hal::boolean>(pcmData.empty() || ((pcmData.size() % 2U) != 0U));
        if (pcmMalformed)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Speech fallback endpoint requires complete signed-sixteen PCM");
        }
        for (size index{}; index < pcmData.size(); index += 2U)
        {
            const uint32 lowByte = pcmData[index];
            const uint32 highByte = pcmData[index + 1U];
            const uint32 rawSample = lowByte | (highByte << 8U);
            const int32 signedSample =
                rawSample >= 32'768U ? static_cast<int32>(rawSample) - 65'536 : static_cast<int32>(rawSample);
            const uint32 magnitude =
                signedSample < 0 ? static_cast<uint32>(-signedSample) : static_cast<uint32>(signedSample);
            const hal::boolean sampleAboveThreshold = static_cast<hal::boolean>(magnitude > peakThreshold);
            if (sampleAboveThreshold)
            {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Converts one callback context into its required adapter.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @return Referenced adapter.
     * @throws std::invalid_argument If `context` is null.
     */
    XWalkSpeechToTextAlsa& XWalkSpeechToTextAlsa::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Speech ALSA callback context must not be null");
        }
        return *static_cast<XWalkSpeechToTextAlsa*>(context);
    }

    /**
     * @brief Reports recognizer readiness.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @return Result from the selected recognizer readiness operation.
     */
    boolean XWalkSpeechToTextAlsa::readyCallback(contextpointer context)
    {
        XWalkSpeechToTextAlsa& self = adapter(context);
        return self.operations.recognizerReady(self.operationContext);
    }

    /**
     * @brief Captures bounded microphone PCM and dispatches recognition.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @param[in] timeoutMs Capture interval from 1 through 300,000 milliseconds.
     * @return Owned transcript after an endpoint or hard timeout, or an empty string after cancellation or silence.
     * @throws std::runtime_error If capture open, data validation, or recovery
     * fails.
     */
    string XWalkSpeechToTextAlsa::listenCallback(contextpointer context, uint32 timeoutMs)
    {
        XWalkSpeechToTextAlsa& self = adapter(context);
        self.cancellationRequested.store(false);
        speechcapturehandle capture = self.operations.openCapture(self.operationContext,
                                                                  self.microphoneDeviceName,
                                                                  XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ,
                                                                  XHAL_RPI5CAR_SPEECH_CAPTURE_CHANNEL_COUNT,
                                                                  XHAL_RPI5CAR_SPEECH_CAPTURE_PERIOD_FRAMES);
        if (capture == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Speech ALSA microphone could not be opened");
        }
        XWalkSpeechCaptureGuard captureGuard(self.operationContext, capture, self.operations.closeCapture);
        speechrecognitionsession session = self.operations.startRecognition(self.operationContext,
                                                                            XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ,
                                                                            XHAL_RPI5CAR_SPEECH_CAPTURE_CHANNEL_COUNT);
        if (session == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Speech streaming recognizer could not be started");
        }
        XWalkSpeechRecognitionGuard recognitionGuard(
            self.operationContext, session, self.operations.releaseRecognition);
        XWALK_HAL_TRACE_UID1(
            RPI .390, "Speech recognition session started with a %u millisecond hard timeout", timeoutMs);
        const uint64 sampleRate = XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ;
        const uint64 requestedMilliseconds = timeoutMs;
        const uint64 requestedFrameProduct = sampleRate * requestedMilliseconds;
        const uint64 maximumFrames = (requestedFrameProduct + 999U) / 1'000U;
        uint64 capturedFrames{};
        uint32 recoveryCount{};
        boolean endpointDetected{false};
        boolean fallbackEndpointDetected{false};
        boolean speechObserved{false};
        uint64 observedSpeechFrames{};
        uint64 trailingLowLevelFrames{};
        const uint64 minimumSpeechFrames =
            (sampleRate * self.endpointConfiguration.minimumSpeechMilliseconds + 999U) / 1'000U;
        const uint64 trailingSilenceFrames =
            (sampleRate * self.endpointConfiguration.trailingSilenceMilliseconds + 999U) / 1'000U;
        const uint64 maximumUtteranceFrames =
            (sampleRate * self.endpointConfiguration.maximumUtteranceMilliseconds + 999U) / 1'000U;
        const hal::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const hal::boolean captureMayContinue =
                static_cast<hal::boolean>((capturedFrames < maximumFrames) && !self.cancellationRequested.load());
            if (captureMayContinue == false)
            {
                break;
            }
            const uint64 remainingFrames = maximumFrames - capturedFrames;
            const uint64 periodFrames = XHAL_RPI5CAR_SPEECH_CAPTURE_PERIOD_FRAMES;
            const size readFrames = static_cast<size>(std::min(remainingFrames, periodFrames));
            bytevector periodData{};
            const int32 result = self.operations.readCapture(self.operationContext, capture, periodData, readFrames);
            if (result > 0)
            {
                const size completedFrames = static_cast<size>(result);
                const size expectedBytes = completedFrames * XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_BYTES;
                const hal::boolean completedFramesReadFramesPeriodDataInvalid =
                    static_cast<hal::boolean>((completedFrames > readFrames) || (periodData.size() != expectedBytes));
                if (completedFramesReadFramesPeriodDataInvalid)
                {
                    XWALK_HAL_ERROR(XWALK_RUNTIME, "Speech ALSA capture returned malformed PCM");
                }
                capturedFrames += completedFrames;
                const hal::boolean cancellationObserved = self.cancellationRequested.load();
                if (cancellationObserved)
                {
                    break;
                }
                const XWalkSpeechRecognitionFeedStatus feedStatus =
                    self.operations.feedRecognition(self.operationContext, session, periodData);
                if (feedStatus == XWalkSpeechRecognitionFeedStatus::Endpoint)
                {
                    endpointDetected = true;
                    XWALK_HAL_TRACE_UID0(RPI .391, "Speech recognition native Vosk endpoint detected");
                    break;
                }
                if (feedStatus == XWalkSpeechRecognitionFeedStatus::Cancelled)
                {
                    self.cancellationRequested.store(true);
                    break;
                }
                const hal::boolean speechReported =
                    static_cast<hal::boolean>(feedStatus == XWalkSpeechRecognitionFeedStatus::SpeechObserved);
                speechObserved = static_cast<boolean>(speechObserved || speechReported);
                if (speechObserved)
                {
                    observedSpeechFrames += completedFrames;
                    const hal::boolean lowLevel =
                        periodIsLowLevel(periodData, self.endpointConfiguration.silencePeakThreshold);
                    trailingLowLevelFrames = lowLevel ? trailingLowLevelFrames + completedFrames : 0U;
                    const boolean minimumSpeechReached = observedSpeechFrames >= minimumSpeechFrames;
                    const boolean trailingSilenceReached = trailingLowLevelFrames >= trailingSilenceFrames;
                    const boolean utteranceLimitReached = observedSpeechFrames >= maximumUtteranceFrames;
                    fallbackEndpointDetected =
                        static_cast<boolean>(utteranceLimitReached || (minimumSpeechReached && trailingSilenceReached));
                    if (fallbackEndpointDetected)
                    {
                        XWALK_HAL_TRACE_UID0(RPI .392, "Speech recognition fallback endpoint detected");
                        break;
                    }
                }
                const hal::boolean feedStatusInvalid = static_cast<hal::boolean>(
                    (feedStatus != XWalkSpeechRecognitionFeedStatus::Listening) && !speechReported);
                if (feedStatusInvalid)
                {
                    XWALK_HAL_ERROR(XWALK_RUNTIME, "Speech streaming recognizer returned an invalid status");
                }
            }
            else
            {
                ++recoveryCount;
                const hal::boolean resultInvalid = static_cast<hal::boolean>(
                    (result == 0) || (recoveryCount > XHAL_RPI5CAR_AUDIO_RECOVERY_ATTEMPT_COUNT) ||
                    !self.operations.recoverCapture(self.operationContext, capture, result));
                if (resultInvalid)
                {
                    XWALK_HAL_ERROR(XWALK_RUNTIME, "Speech ALSA capture recovery failed");
                }
            }
        }
        const hal::boolean recognitionCancelled = self.cancellationRequested.load();
        if (recognitionCancelled)
        {
            return {};
        }
        const boolean endpointAccepted = static_cast<boolean>(endpointDetected || fallbackEndpointDetected);
        if (endpointAccepted == false)
        {
            XWALK_HAL_TRACE_UID0(RPI .393, "Speech recognition hard timeout reached before an endpoint");
        }
        const string transcript = self.operations.finishRecognition(self.operationContext, session, endpointDetected);
        const hal::boolean transcriptEmpty = static_cast<hal::boolean>(transcript.empty());
        if (transcriptEmpty)
        {
            XWALK_HAL_TRACE_UID0(RPI .394, "Speech recognition completed without recognized text");
        }
        else
        {
            XWALK_HAL_TRACE_UID1(RPI .395,
                                 "Speech recognition completed with %llu transcript character(s)",
                                 static_cast<unsigned long long>(transcript.size()));
            if (self.endpointConfiguration.traceTranscript)
            {
                XWALK_HAL_TRACE_UID1(RPI .396, "Speech recognition diagnostic transcript: %s", transcript.c_str());
            }
        }
        return transcript;
    }

    /**
     * @brief Dispatches one audio-file transcription request.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @param[in] filePath Non-empty path already validated by the coordinator.
     * @return Owned recognizer transcript, which may be empty.
     */
    string XWalkSpeechToTextAlsa::fileCallback(contextpointer context, stringview filePath)
    {
        XWalkSpeechToTextAlsa& self = adapter(context);
        self.cancellationRequested.store(false);
        return self.operations.recognizeFile(self.operationContext, filePath);
    }

    /**
     * @brief Requests capture and recognizer cancellation.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @warning The recognizer cancellation operation must not throw.
     */
    void XWalkSpeechToTextAlsa::stopCallback(contextpointer context)
    {
        XWalkSpeechToTextAlsa& self = adapter(context);
        self.cancellationRequested.store(true);
        self.operations.cancelRecognition(self.operationContext);
    }

} /* namespace xwalk::hal */
