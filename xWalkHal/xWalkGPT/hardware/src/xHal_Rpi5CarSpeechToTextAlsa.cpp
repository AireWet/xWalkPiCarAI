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
 * @param[in,out] recognizerContext Nullable non-owning context that must outlive this adapter.
 * @param[in] recognizerOperations Operations containing four non-null recognizer callbacks.
 * @throws std::invalid_argument If a recognizer callback is null or the device name is empty.
 */
XWalkSpeechToTextAlsa::XWalkSpeechToTextAlsa(stringview deviceName,
    contextpointer recognizerContext,
    const XWalkSpeechToTextAlsaOperations& recognizerOperations):
    XWalkSpeechToTextAlsa(recognizerContext,
        systemCaptureOperations(recognizerOperations), deviceName)
{
}

/**
 * @brief Constructs an adapter with fully injected operations.
 *
 * @param[in,out] context Nullable non-owning context that must outlive this adapter.
 * @param[in] backendOperations Complete capture and recognition operation table.
 * @param[in] deviceName Non-empty capture name forwarded to the open operation.
 * @throws std::invalid_argument If any callback is null or the device name is empty.
 */
XWalkSpeechToTextAlsa::XWalkSpeechToTextAlsa(contextpointer context,
    const XWalkSpeechToTextAlsaOperations& backendOperations, stringview deviceName):
    operationContext(context), operations(backendOperations), microphoneDeviceName(deviceName)
{
    validateOperations(operations, microphoneDeviceName);
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
 * @brief Copies recognizer operations and installs real ALSA capture operations.
 *
 * @param[in] recognizerOperations Operations containing recognizer callbacks.
 * @return Combined recognition and libasound capture operation table.
 */
XWalkSpeechToTextAlsaOperations XWalkSpeechToTextAlsa::systemCaptureOperations(
    const XWalkSpeechToTextAlsaOperations& recognizerOperations)
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
 * @throws std::invalid_argument If any callback is null or `deviceName` is empty.
 */
void XWalkSpeechToTextAlsa::validateOperations(
    const XWalkSpeechToTextAlsaOperations& backendOperations, stringview deviceName)
{
    const boolean captureMissing = (backendOperations.openCapture == nullptr) ||
        (backendOperations.readCapture == nullptr) ||
        (backendOperations.recoverCapture == nullptr) ||
        (backendOperations.closeCapture == nullptr);
    const boolean recognizerMissing = (backendOperations.recognizerReady == nullptr) ||
        (backendOperations.recognizePcm == nullptr) ||
        (backendOperations.recognizeFile == nullptr) ||
        (backendOperations.cancelRecognition == nullptr);
    const hal::boolean speechConfigurationInvalid =
        static_cast<hal::boolean>(
            captureMissing || recognizerMissing || deviceName.empty());
    if (speechConfigurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Speech ALSA backend requires complete operations and a device");
    }
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
        XHAL_THROW_INVALID_ARGUMENT("Speech ALSA callback context must not be null");
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
 * @return Owned transcript, or an empty string after cancellation or empty capture.
 * @throws std::runtime_error If capture open, data validation, or recovery fails.
 */
string XWalkSpeechToTextAlsa::listenCallback(contextpointer context, uint32 timeoutMs)
{
    XWalkSpeechToTextAlsa& self = adapter(context);
    self.cancellationRequested.store(false);
    speechcapturehandle capture = self.operations.openCapture(self.operationContext,
        self.microphoneDeviceName, XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ,
        XHAL_RPI5CAR_SPEECH_CAPTURE_CHANNEL_COUNT,
        XHAL_RPI5CAR_SPEECH_CAPTURE_PERIOD_FRAMES);
    if (capture == nullptr)
    {
        XHAL_THROW_RUNTIME_ERROR("Speech ALSA microphone could not be opened");
    }
    const uint64 sampleRate = XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ;
    const uint64 requestedMilliseconds = timeoutMs;
    const uint64 requestedFrameProduct = sampleRate * requestedMilliseconds;
    const uint64 maximumFrames = (requestedFrameProduct + 999U) / 1'000U;
    bytevector capturedPcm{};
    const uint64 maximumBytes = maximumFrames * XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_BYTES;
    capturedPcm.reserve(static_cast<size>(maximumBytes));
    uint64 capturedFrames{};
    uint32 recoveryCount{};
    const hal::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const hal::boolean captureMayContinue =
            static_cast<hal::boolean>(
                (capturedFrames < maximumFrames) && !self.cancellationRequested.load());
        if (captureMayContinue == false)
        {
            break;
        }
        const uint64 remainingFrames = maximumFrames - capturedFrames;
        const uint64 periodFrames = XHAL_RPI5CAR_SPEECH_CAPTURE_PERIOD_FRAMES;
        const size readFrames = static_cast<size>(std::min(remainingFrames, periodFrames));
        bytevector periodData{};
        const int32 result = self.operations.readCapture(
            self.operationContext, capture, periodData, readFrames);
        if (result > 0)
        {
            const size completedFrames = static_cast<size>(result);
            const size expectedBytes = completedFrames * XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_BYTES;
            const hal::boolean completedFramesReadFramesPeriodDataInvalid =
                static_cast<hal::boolean>(
                    (completedFrames > readFrames) || (periodData.size() != expectedBytes));
            if (completedFramesReadFramesPeriodDataInvalid)
            {
                self.operations.closeCapture(self.operationContext, capture);
                XHAL_THROW_RUNTIME_ERROR("Speech ALSA capture returned malformed PCM");
            }
            capturedPcm.insert(capturedPcm.end(), periodData.begin(), periodData.end());
            capturedFrames += completedFrames;
        }
        else
        {
            ++recoveryCount;
            const hal::boolean resultInvalid =
                static_cast<hal::boolean>(
                    (result == 0) ||
                (recoveryCount > XHAL_RPI5CAR_AUDIO_RECOVERY_ATTEMPT_COUNT) ||
                !self.operations.recoverCapture(self.operationContext, capture, result));
            if (resultInvalid)
            {
                self.operations.closeCapture(self.operationContext, capture);
                XHAL_THROW_RUNTIME_ERROR("Speech ALSA capture recovery failed");
            }
        }
    }
    self.operations.closeCapture(self.operationContext, capture);
    const hal::boolean selfCancellationRequestedCapturedPcmInvalid =
        static_cast<hal::boolean>(
            self.cancellationRequested.load() || capturedPcm.empty());
    if (selfCancellationRequestedCapturedPcmInvalid)
    {
        return {};
    }
    return self.operations.recognizePcm(self.operationContext, capturedPcm,
        XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ,
        XHAL_RPI5CAR_SPEECH_CAPTURE_CHANNEL_COUNT);
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
