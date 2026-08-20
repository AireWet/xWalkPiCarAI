/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsaTest.cpp
 * @brief       Verifies bounded streaming recognition without a microphone.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Host Test
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarGptTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"

namespace
{
    using namespace xwalk::hal;
    using namespace xwalk::hal::test::gpt;
    using xwalk::hal::test::expectFailure;
    using xwalk::hal::test::requireTestCondition;

    /** @brief Verifies that periods are fed incrementally and endpoint detection stops capture. */
    void testIncrementalEndpoint()
    {
        TestStreamingBackend backend;
        backend.endpointAfterFeed = 2U;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic");
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.listen(1'000U) == "recognized microphone");
        requireTestCondition(backend.readCount == 2U);
        requireTestCondition(backend.feedCount == 2U);
        requireTestCondition(backend.capturedFrames == 2'048U);
        requireTestCondition(backend.finishCount == 1U);
        requireTestCondition(backend.endpointFinalized);
        requireTestCondition(backend.closeCount == 1U);
        requireTestCondition(backend.releaseCount == 1U);
    }

    /** @brief Verifies that the hard timeout finalizes a partial utterance. */
    void testTimeoutFinalizesPartial()
    {
        TestStreamingBackend backend;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic");
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.listen(100U) == "recognized microphone");
        requireTestCondition(backend.readCount == 2U);
        requireTestCondition(backend.feedCount == 2U);
        requireTestCondition(backend.capturedFrames == 1'600U);
        requireTestCondition(!backend.endpointFinalized);
    }

    /** @brief Verifies observed Vosk speech plus trailing low-level PCM triggers bounded fallback completion. */
    void testTrailingSilenceFallback()
    {
        TestStreamingBackend backend;
        backend.speechAfterFeed = 1U;
        backend.lowLevelAfterRead = 2U;
        XWalkSpeechEndpointConfiguration endpointConfiguration;
        endpointConfiguration.minimumSpeechMilliseconds = 64U;
        endpointConfiguration.trailingSilenceMilliseconds = 128U;
        endpointConfiguration.maximumUtteranceMilliseconds = 1'000U;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic", endpointConfiguration);
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.listen(1'000U) == "recognized microphone");
        requireTestCondition(backend.readCount == 3U);
        requireTestCondition(backend.feedCount == 3U);
        requireTestCondition(!backend.endpointFinalized);
    }

    /** @brief Verifies quiet initial PCM never arms fallback endpointing without recognizer-observed speech. */
    void testQuietInitialInputDoesNotEndpoint()
    {
        TestStreamingBackend backend;
        backend.lowLevelAfterRead = 1U;
        XWalkSpeechEndpointConfiguration endpointConfiguration;
        endpointConfiguration.minimumSpeechMilliseconds = 64U;
        endpointConfiguration.trailingSilenceMilliseconds = 64U;
        endpointConfiguration.maximumUtteranceMilliseconds = 1'000U;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic", endpointConfiguration);
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.listen(128U) == "recognized microphone");
        requireTestCondition(backend.readCount == 2U);
        requireTestCondition(backend.feedCount == 2U);
        requireTestCondition(!backend.endpointFinalized);
    }

    /** @brief Verifies that a finalized silent session preserves an empty transcript. */
    void testSilenceReturnsEmpty()
    {
        TestStreamingBackend backend;
        backend.silentResult = true;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic");
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.listen(64U).empty());
        requireTestCondition(backend.feedCount == 1U);
        requireTestCondition(backend.finishCount == 1U);
        requireTestCondition(backend.releaseCount == 1U);
    }

    /** @brief Verifies one ALSA underrun recovery remains bounded and recognition continues. */
    void testCaptureRecovery()
    {
        TestStreamingBackend backend;
        backend.failFirstRead = true;
        backend.endpointAfterFeed = 1U;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic");
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.listen(1'000U) == "recognized microphone");
        requireTestCondition(backend.readCount == 2U);
        requireTestCondition(backend.recoveryCount == 1U);
        requireTestCondition(backend.feedCount == 1U);
    }

    /** @brief Verifies repeated ALSA failures terminate after the configured recovery bound. */
    void testRecoveryFailureIsBounded()
    {
        TestStreamingBackend backend;
        backend.alwaysFailRead = true;
        expectFailure(
            [&]()
            {
                XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic");
                XWalkSpeechToText speech(&adapter, adapter.callbacks());
                static_cast<void>(speech.listen(1'000U));
            });
    }

    /** @brief Verifies cancellation closes capture and releases recognition without finalizing. */
    void testCancellationCleanup()
    {
        TestStreamingBackend backend;
        backend.delayRead = true;
        XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic");
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        string transcript{"not cancelled"};
        threadhandle listener(
            [&]()
            {
                transcript = speech.listen(1'000U);
            });
        for (uint32 waitCount{}; !backend.readStarted.load() && (waitCount < 1'000U); ++waitCount)
        {
            std::this_thread::sleep_for(millisecondduration(1));
        }
        requireTestCondition(backend.readStarted.load());
        speech.stop();
        listener.join();
        requireTestCondition(transcript.empty());
        requireTestCondition(backend.feedCount == 0U);
        requireTestCondition(backend.finishCount == 0U);
        requireTestCondition(backend.closeCount == 1U);
        requireTestCondition(backend.releaseCount == 1U);
        requireTestCondition(backend.cancelCount == 1U);
    }

    /** @brief Verifies whole-buffer and file recognition callbacks remain compatible. */
    void testCompatibilityPaths()
    {
        TestStreamingBackend backend;
        const XWalkSpeechToTextAlsaOperations operations = streamingOperations();
        const bytevector pcm(320U, 0x11U);
        requireTestCondition(operations.recognizePcm(&backend, pcm, 16'000U, 1U) == "recognized whole buffer");
        requireTestCondition(backend.recognizedBytes == pcm.size());
        XWalkSpeechToTextAlsa adapter(&backend, operations, "test-mic");
        XWalkSpeechToText speech(&adapter, adapter.callbacks());
        requireTestCondition(speech.transcribeFile("sample.wav") == "recognized file");
    }

    /** @brief Verifies incomplete streaming callback tables are rejected. */
    void testValidation()
    {
        TestStreamingBackend backend;
        XWalkSpeechToTextAlsaOperations incomplete = streamingOperations();
        incomplete.feedRecognition = nullptr;
        expectFailure(
            [&]()
            {
                XWalkSpeechToTextAlsa adapter(&backend, incomplete, "test-mic");
            });
        XWalkSpeechEndpointConfiguration invalidEndpoint;
        invalidEndpoint.silencePeakThreshold = 0U;
        expectFailure(
            [&]()
            {
                XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic", invalidEndpoint);
            });
        invalidEndpoint.silencePeakThreshold = 500U;
        invalidEndpoint.trailingSilenceMilliseconds = 16'000U;
        expectFailure(
            [&]()
            {
                XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "test-mic", invalidEndpoint);
            });
        expectFailure(
            [&]()
            {
                XWalkSpeechToTextAlsa adapter(&backend, streamingOperations(), "");
            });
    }
} /* namespace */

/** @brief Runs all streaming speech capture adapter host tests. */
XWalkHal::int32 main()
{
    testIncrementalEndpoint();
    testTimeoutFinalizesPartial();
    testTrailingSilenceFallback();
    testQuietInitialInputDoesNotEndpoint();
    testSilenceReturnsEmpty();
    testCaptureRecovery();
    testRecoveryFailureIsBounded();
    testCancellationCleanup();
    testCompatibilityPaths();
    testValidation();
    return 0;
}
