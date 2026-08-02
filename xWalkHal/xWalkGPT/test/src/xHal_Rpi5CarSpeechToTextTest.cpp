/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextTest.cpp
 * @brief       Verifies speech-recognition dispatch and validation behavior.
 *
 * @details
 * Exercises readiness, default and explicit timeouts, file transcription,
 * cancellation, backend failures, destructor safety, and incomplete callbacks.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Host Test
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarSpeechToText.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains host-test state and callbacks private to this translation unit.
 */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Supplies deterministic speech-recognition callback behavior. */
struct TestSpeechBackend
{
    boolean readyValue{true}; /**< Readiness returned by the backend. */
    string listenResult{"microphone result"}; /**< Next microphone recognition result. */
    string fileResult{"file result"}; /**< Next audio-file transcription result. */
    string filePath{}; /**< Owned copy of the most recent audio-file path. */
    uint32 timeoutMs{}; /**< Most recent microphone timeout in milliseconds. */
    uint32 readyCount{}; /**< Number of readiness callback entries. */
    uint32 listenCount{}; /**< Number of microphone callback entries. */
    uint32 fileCount{}; /**< Number of audio-file callback entries. */
    uint32 stopCount{}; /**< Number of cancellation callback entries. */
    boolean failReady{}; /**< `true` to fail the next readiness request. */
    boolean failListen{}; /**< `true` to fail the next microphone request. */
    boolean failFile{}; /**< `true` to fail the next file request. */
    boolean failStop{}; /**< `true` to fail a cancellation request. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/**
 * @brief Reports configured backend readiness or raises a test failure.
 *
 * @param[in,out] context
 * Non-null test speech backend.
 *
 * @return
 * Configured readiness value.
 *
 * @throws std::runtime_error
 * If readiness failure is enabled.
 */
boolean ready(contextpointer context)
{
    TestSpeechBackend& backend = *static_cast<TestSpeechBackend*>(context);
    ++backend.readyCount;
    if (backend.failReady)
    {
        XHAL_THROW_RUNTIME_ERROR("Test speech readiness failed");
    }
    return backend.readyValue;
}

/**
 * @brief Records a microphone request and returns configured recognized text.
 *
 * @param[in,out] context
 * Non-null test speech backend.
 *
 * @param[in] timeoutMs
 * Validated microphone interval in milliseconds.
 *
 * @return
 * Configured microphone recognition result.
 *
 * @throws std::runtime_error
 * If microphone failure is enabled.
 */
string listen(contextpointer context, uint32 timeoutMs)
{
    TestSpeechBackend& backend = *static_cast<TestSpeechBackend*>(context);
    ++backend.listenCount;
    backend.timeoutMs = timeoutMs;
    if (backend.failListen)
    {
        XHAL_THROW_RUNTIME_ERROR("Test microphone recognition failed");
    }
    return backend.listenResult;
}

/**
 * @brief Records an audio-file request and returns configured recognized text.
 *
 * @param[in,out] context
 * Non-null test speech backend.
 *
 * @param[in] filePath
 * Validated non-empty audio-file path.
 *
 * @return
 * Configured audio-file transcription result.
 *
 * @throws std::runtime_error
 * If audio-file failure is enabled.
 */
string transcribeFile(contextpointer context, stringview filePath)
{
    TestSpeechBackend& backend = *static_cast<TestSpeechBackend*>(context);
    ++backend.fileCount;
    backend.filePath = string(filePath);
    if (backend.failFile)
    {
        XHAL_THROW_RUNTIME_ERROR("Test audio-file transcription failed");
    }
    return backend.fileResult;
}

/**
 * @brief Records a cancellation request or raises a configured failure.
 *
 * @param[in,out] context
 * Non-null test speech backend.
 *
 * @throws std::runtime_error
 * If cancellation failure is enabled.
 */
void stop(contextpointer context)
{
    TestSpeechBackend& backend = *static_cast<TestSpeechBackend*>(context);
    ++backend.stopCount;
    if (backend.failStop)
    {
        XHAL_THROW_RUNTIME_ERROR("Test speech cancellation failed");
    }
}

/**
 * @brief Returns the complete in-memory speech backend callback table.
 *
 * @return
 * Callback table containing only non-null functions.
 */
XWalkSpeechToTextCallbacks backendCallbacks()
{
    return {&ready, &listen, &transcribeFile, &stop};
}

/** @brief Verifies readiness, recognition, transcription, and cancellation. */
void testOperations()
{
    TestSpeechBackend backend;
    {
        XWalkSpeechToText speechToText(&backend, backendCallbacks());
        assert(speechToText.isReady());
        assert(backend.readyCount == 1U);

        assert(speechToText.listen() == "microphone result");
        assert(backend.timeoutMs == XHAL_RPI5CAR_SPEECH_TO_TEXT_DEFAULT_TIMEOUT_MS);
        assert(speechToText.listen(1'500U) == "microphone result");
        assert(backend.timeoutMs == 1'500U);
        assert(backend.listenCount == 2U);

        assert(speechToText.transcribeFile("sample.wav") == "file result");
        assert(backend.filePath == "sample.wav");
        assert(backend.fileCount == 1U);

        speechToText.stop();
        assert(backend.stopCount == 1U);
    }
    assert(backend.stopCount == 2U);
}

/** @brief Verifies invalid timeouts and empty file paths are rejected. */
void testInputValidation()
{
    TestSpeechBackend backend;
    XWalkSpeechToText speechToText(&backend, backendCallbacks());

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speechToText.listen(0U));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speechToText.listen(
            XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS + 1U));
    });
    assert(backend.listenCount == 0U);

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speechToText.transcribeFile(""));
    });
    assert(backend.fileCount == 0U);
}

/** @brief Verifies backend operation failures are propagated to the caller. */
void testBackendFailures()
{
    TestSpeechBackend backend;
    XWalkSpeechToText speechToText(&backend, backendCallbacks());

    backend.failReady = true;
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speechToText.isReady());
    });

    backend.failListen = true;
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speechToText.listen());
    });

    backend.failFile = true;
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speechToText.transcribeFile("failure.wav"));
    });

    backend.failStop = true;
    xwalk::hal::test::expectFailure([&]()
    {
        speechToText.stop();
    });
    backend.failStop = false;
}

/** @brief Verifies every callback is mandatory during construction. */
void testCallbackValidation()
{
    TestSpeechBackend backend;
    fixedarray<XWalkSpeechToTextCallbacks, 4U> incompleteCallbacks{};
    incompleteCallbacks[0U] = {nullptr, &listen, &transcribeFile, &stop};
    incompleteCallbacks[1U] = {&ready, nullptr, &transcribeFile, &stop};
    incompleteCallbacks[2U] = {&ready, &listen, nullptr, &stop};
    incompleteCallbacks[3U] = {&ready, &listen, &transcribeFile, nullptr};

    for (const XWalkSpeechToTextCallbacks& callbacks : incompleteCallbacks)
    {
        xwalk::hal::test::expectFailure([&]()
        {
            XWalkSpeechToText speechToText(&backend, callbacks);
            static_cast<void>(speechToText);
        });
    }
    assert(backend.stopCount == 0U);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side speech-to-text test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int32 main()
{
    testOperations();
    testInputValidation();
    testBackendFailures();
    testCallbackValidation();
    return 0;
}
