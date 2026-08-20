/******************************************************************************
 * @file        xHal_Rpi5CarGptTestSupport.h
 * @brief       Declares reusable xWalkGPT core host-test support.
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_GPT_TEST_SUPPORT_H
#define XHAL_RPI5CAR_GPT_TEST_SUPPORT_H
#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSpeechToTextAlsa.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTextToSpeech.h"
namespace xwalk::hal::test::gpt
{
    /** @brief Supplies deterministic speech-recognition callback behavior. */
    struct TestRecognitionBackend
    {
            boolean readyValue{true};
            string listenResult{"microphone result"};
            string fileResult{"file result"};
            string filePath{};
            uint32 timeoutMs{};
            uint32 readyCount{};
            uint32 listenCount{};
            uint32 fileCount{};
            uint32 stopCount{};
            boolean failReady{};
            boolean failListen{};
            boolean failFile{};
            boolean failStop{};
    };
    /** @brief Records incremental capture and streaming-recognition behavior. */
    struct TestStreamingBackend
    {
            uint8 captureToken{1U};
            uint8 sessionToken{2U};
            size capturedFrames{};
            size recognizedBytes{};
            uint32 readCount{};
            uint32 closeCount{};
            uint32 recoveryCount{};
            uint32 startCount{};
            uint32 feedCount{};
            uint32 finishCount{};
            uint32 releaseCount{};
            uint32 cancelCount{};
            uint32 endpointAfterFeed{};
            boolean endpointFinalized{};
            boolean failFirstRead{};
            boolean alwaysFailRead{};
            boolean delayRead{};
            boolean silentResult{};
            atomicboolean readStarted{false};
    };
    /** @brief Retains the current simulated GPIO state. */
    struct TestGpioBackend
    {
            boolean physicalValue{};
            uint32 writeCount{};
    };
    /** @brief Provides context for the injected I2C interface. */
    struct TestI2cBackend
    {
            uint32 readCount{};
    };
    /** @brief Records speaker priming. */
    struct TestSpeakerPrime
    {
            uint32 callCount{};
            uint32 durationMs{};
    };
    /** @brief Records speech requests and optional failure. */
    struct TestSpeechBackend
    {
            string text{};
            uint32 callCount{};
            boolean fail{};
    };
    boolean ready(contextpointer context);
    string listen(contextpointer context, uint32 timeoutMs);
    string transcribeFile(contextpointer context, stringview filePath);
    void stop(contextpointer context);
    XWalkSpeechToTextCallbacks recognitionCallbacks();
    speechcapturehandle
    openStreamingCapture(contextpointer context, stringview device, uint32 rate, uint8 channels, uint32 period);
    int32 readStreamingCapture(contextpointer context, speechcapturehandle handle, bytevector& data, size frames);
    boolean recoverStreamingCapture(contextpointer context, speechcapturehandle handle, int32 error);
    void closeStreamingCapture(contextpointer context, speechcapturehandle handle);
    boolean streamingRecognizerReady(contextpointer context);
    string recognizeWholePcm(contextpointer context, const bytevector& pcm, uint32 rate, uint8 channels);
    speechrecognitionsession startStreamingRecognition(contextpointer context, uint32 rate, uint8 channels);
    XWalkSpeechRecognitionFeedStatus
    feedStreamingRecognition(contextpointer context, speechrecognitionsession session, const bytevector& pcm);
    string
    finishStreamingRecognition(contextpointer context, speechrecognitionsession session, boolean endpointDetected);
    void releaseStreamingRecognition(contextpointer context, speechrecognitionsession session);
    string recognizeStreamingFile(contextpointer context, stringview path);
    void cancelStreamingRecognition(contextpointer context);
    XWalkSpeechToTextAlsaOperations streamingOperations();
    void configureGpio(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
    boolean readGpio(contextpointer context, uint8 pin);
    void writeGpio(contextpointer context, uint8 pin, boolean value);
    void registerInterrupt(contextpointer context,
                           uint8 pin,
                           XWalkGpioEdge edge,
                           uint32 debounceMs,
                           contextpointer handlerContext,
                           gpiointerrupthandler handler);
    void cancelInterrupt(contextpointer context, uint8 pin);
    XWalkGpioCallbacks gpioCallbacks();
    boolean probeI2c(contextpointer context, uint8 address);
    void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
    bytevector readI2c(contextpointer context, uint8 address, size length);
    void primeSpeaker(contextpointer context, uint32 durationMs);
    void speakText(contextpointer context, stringview text);
} /* namespace xwalk::hal::test::gpt */
#endif /* XHAL_RPI5CAR_GPT_TEST_SUPPORT_H */
