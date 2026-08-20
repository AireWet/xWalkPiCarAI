/******************************************************************************
 * @file        xHal_Rpi5CarGptTestSupport.cpp
 * @brief       Implements reusable xWalkGPT core host-test support.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarGptTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::test::gpt
{
    boolean ready(contextpointer context)
    {
        TestRecognitionBackend& backend = *static_cast<TestRecognitionBackend*>(context);
        ++backend.readyCount;
        if (backend.failReady)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test speech readiness failed");
        }
        return backend.readyValue;
    }
    string listen(contextpointer context, uint32 timeoutMs)
    {
        TestRecognitionBackend& backend = *static_cast<TestRecognitionBackend*>(context);
        ++backend.listenCount;
        backend.timeoutMs = timeoutMs;
        if (backend.failListen)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test microphone recognition failed");
        }
        return backend.listenResult;
    }
    string transcribeFile(contextpointer context, stringview filePath)
    {
        TestRecognitionBackend& backend = *static_cast<TestRecognitionBackend*>(context);
        ++backend.fileCount;
        backend.filePath = string(filePath);
        if (backend.failFile)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test file transcription failed");
        }
        return backend.fileResult;
    }
    void stop(contextpointer context)
    {
        TestRecognitionBackend& backend = *static_cast<TestRecognitionBackend*>(context);
        ++backend.stopCount;
        if (backend.failStop)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test speech cancellation failed");
        }
    }
    XWalkSpeechToTextCallbacks recognitionCallbacks()
    {
        return {&ready, &listen, &transcribeFile, &stop};
    }
    speechcapturehandle
    openStreamingCapture(contextpointer context, stringview device, uint32 rate, uint8 channels, uint32 period)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(device == "test-mic");
        requireTestCondition((rate == 16'000U) && (channels == 1U) && (period == 1'024U));
        return &backend.captureToken;
    }
    int32 readStreamingCapture(contextpointer context, speechcapturehandle handle, bytevector& data, size frames)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(handle == &backend.captureToken);
        if (backend.delayRead)
        {
            backend.readStarted.store(true);
            std::this_thread::sleep_for(millisecondduration(20));
        }
        ++backend.readCount;
        const boolean readFails =
            static_cast<boolean>(backend.alwaysFailRead || (backend.failFirstRead && (backend.readCount == 1U)));
        if (readFails)
        {
            data.clear();
            return -32;
        }
        data.assign(frames * 2U, 0x22U);
        backend.capturedFrames += frames;
        return static_cast<int32>(frames);
    }
    boolean recoverStreamingCapture(contextpointer context, speechcapturehandle handle, int32 error)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(handle == &backend.captureToken);
        ++backend.recoveryCount;
        return error == -32;
    }
    void closeStreamingCapture(contextpointer context, speechcapturehandle handle)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(handle == &backend.captureToken);
        ++backend.closeCount;
    }
    boolean streamingRecognizerReady(contextpointer context)
    {
        static_cast<void>(context);
        return true;
    }
    string recognizeWholePcm(contextpointer context, const bytevector& pcm, uint32 rate, uint8 channels)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition((rate == 16'000U) && (channels == 1U));
        backend.recognizedBytes = pcm.size();
        return "recognized whole buffer";
    }
    speechrecognitionsession startStreamingRecognition(contextpointer context, uint32 rate, uint8 channels)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition((rate == 16'000U) && (channels == 1U));
        ++backend.startCount;
        return &backend.sessionToken;
    }
    XWalkSpeechRecognitionFeedStatus
    feedStreamingRecognition(contextpointer context, speechrecognitionsession session, const bytevector& pcm)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(session == &backend.sessionToken);
        requireTestCondition(!pcm.empty());
        ++backend.feedCount;
        const boolean endpointReached =
            static_cast<boolean>((backend.endpointAfterFeed > 0U) && (backend.feedCount >= backend.endpointAfterFeed));
        return endpointReached ? XWalkSpeechRecognitionFeedStatus::Endpoint
                               : XWalkSpeechRecognitionFeedStatus::Listening;
    }
    string
    finishStreamingRecognition(contextpointer context, speechrecognitionsession session, boolean endpointDetected)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(session == &backend.sessionToken);
        ++backend.finishCount;
        backend.endpointFinalized = endpointDetected;
        return backend.silentResult ? string{} : string{"recognized microphone"};
    }
    void releaseStreamingRecognition(contextpointer context, speechrecognitionsession session)
    {
        TestStreamingBackend& backend = *static_cast<TestStreamingBackend*>(context);
        requireTestCondition(session == &backend.sessionToken);
        ++backend.releaseCount;
    }
    string recognizeStreamingFile(contextpointer context, stringview path)
    {
        static_cast<void>(context);
        requireTestCondition(path == "sample.wav");
        return "recognized file";
    }
    void cancelStreamingRecognition(contextpointer context)
    {
        ++static_cast<TestStreamingBackend*>(context)->cancelCount;
    }
    XWalkSpeechToTextAlsaOperations streamingOperations()
    {
        XWalkSpeechToTextAlsaOperations result{};
        result.openCapture = &openStreamingCapture;
        result.readCapture = &readStreamingCapture;
        result.recoverCapture = &recoverStreamingCapture;
        result.closeCapture = &closeStreamingCapture;
        result.recognizerReady = &streamingRecognizerReady;
        result.recognizePcm = &recognizeWholePcm;
        result.startRecognition = &startStreamingRecognition;
        result.feedRecognition = &feedStreamingRecognition;
        result.finishRecognition = &finishStreamingRecognition;
        result.releaseRecognition = &releaseStreamingRecognition;
        result.recognizeFile = &recognizeStreamingFile;
        result.cancelRecognition = &cancelStreamingRecognition;
        return result;
    }
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<TestGpioBackend*>(context)->physicalValue = initialValue;
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
    }
    boolean readGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<TestGpioBackend*>(context)->physicalValue;
    }
    void writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        TestGpioBackend& backend = *static_cast<TestGpioBackend*>(context);
        backend.physicalValue = value;
        ++backend.writeCount;
        static_cast<void>(pin);
    }
    void registerInterrupt(contextpointer context,
                           uint8 pin,
                           XWalkGpioEdge edge,
                           uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }
    void cancelInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    XWalkGpioCallbacks gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
    }
    boolean probeI2c(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }
    void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }
    bytevector readI2c(contextpointer context, uint8 address, size length)
    {
        ++static_cast<TestI2cBackend*>(context)->readCount;
        static_cast<void>(address);
        static_cast<void>(length);
        return {};
    }
    void primeSpeaker(contextpointer context, uint32 durationMs)
    {
        TestSpeakerPrime& prime = *static_cast<TestSpeakerPrime*>(context);
        ++prime.callCount;
        prime.durationMs = durationMs;
    }
    void speakText(contextpointer context, stringview text)
    {
        TestSpeechBackend& backend = *static_cast<TestSpeechBackend*>(context);
        ++backend.callCount;
        backend.text = string(text);
        if (backend.fail)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Test text-to-speech backend failed");
        }
    }
} /* namespace xwalk::hal::test::gpt */
