/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerTest.cpp
 * @brief       Verifies speaker behavior using an in-memory audio backend.
 *
 * @details
 * Checks output lifecycle, format routing, asynchronous task control, progress,
 * completion cleanup, validation, and fatal worker failures.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Test
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarSpeaker.h"

#include "xHal_Rpi5CarTestFunctions.h"

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/**
 * @brief Contains test state and callbacks private to this translation unit.
 */
namespace
{

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records simulated decoding, stream, and speaker-output operations. */
struct TestBackend
{
    /** @brief Number of speaker-output enable operations. */
    XWalkHal::uint32 enableCount{};
    /** @brief Number of speaker-output disable operations. */
    XWalkHal::uint32 disableCount{};
    /** @brief Number of audio decode operations. */
    XWalkHal::uint32 decodeCount{};
    /** @brief Number of stream-open operations. */
    XWalkHal::uint32 openCount{};
    /** @brief Number of frame-write operations. */
    XWalkHal::uint32 writeCount{};
    /** @brief Number of stream-close operations. */
    XWalkHal::uint32 closeCount{};
    /** @brief Counter used to produce deterministic unique task identifiers. */
    XWalkHal::uint32 taskIdCount{};
    /** @brief Frame count returned by the simulated decoder. */
    XWalkHal::size decodedFrameCount{65'536U};
    /** @brief Most recently selected Python-compatible decoder family. */
    XWalkHal::XWalkSpeakerAudioHandler handler{
        XWalkHal::XWalkSpeakerAudioHandler::SoundFile};
    /** @brief `true` when decoded metadata must be invalid. */
    XWalkHal::boolean invalidAudio{};
    /** @brief `true` when every frame write must fail. */
    XWalkHal::boolean failWrite{};
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Records one simulated speaker-output enable operation. */
void enableOutput(XWalkHal::contextpointer context)
{
    ++static_cast<TestBackend*>(context)->enableCount;
}

/** @brief Records one simulated speaker-output disable operation. */
void disableOutput(XWalkHal::contextpointer context)
{
    ++static_cast<TestBackend*>(context)->disableCount;
}

/** @brief Returns deterministic mono audio for one simulated decode operation. */
XWalkHal::XWalkSpeakerAudioData decodeAudio(XWalkHal::contextpointer context,
    XWalkHal::stringview filePath, XWalkHal::XWalkSpeakerAudioHandler handler)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    static_cast<void>(filePath);
    ++backend.decodeCount;
    backend.handler = handler;
    if (backend.invalidAudio)
    {
        return {{0.0}, 0U, 1U};
    }
    return {XWalkHal::float64vector(backend.decodedFrameCount, 0.25), 44'100U, 1U};
}

/** @brief Returns the backend address as a non-null simulated stream handle. */
XWalkHal::speakerstreamhandle openStream(XWalkHal::contextpointer context,
    XWalkHal::uint32 sampleRateHz, XWalkHal::uint8 channelCount)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(sampleRateHz == 44'100U);
    assert(channelCount == 1U);
    ++backend.openCount;
    return context;
}

/** @brief Records one bounded simulated stream write or reports a configured failure. */
void writeStream(XWalkHal::contextpointer context, XWalkHal::speakerstreamhandle stream,
    const XWalkHal::XWalkSpeakerAudioData& audioData, XWalkHal::size firstFrame,
    XWalkHal::size frameCount)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(stream == context);
    assert((firstFrame + frameCount) <= audioData.samples.size());
    assert(frameCount <= XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT);
    if (backend.failWrite)
    {
        XHAL_THROW_RUNTIME_ERROR("Simulated speaker stream failure");
    }
    ++backend.writeCount;
    XWalkHal::common::sleepMilliseconds(2U);
}

/** @brief Records one simulated stream-close operation. */
void closeStream(XWalkHal::contextpointer context, XWalkHal::speakerstreamhandle stream)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    assert(stream == context);
    ++backend.closeCount;
}

/** @brief Creates one deterministic unique playback-task identifier. */
XWalkHal::string createTaskId(XWalkHal::contextpointer context)
{
    TestBackend& backend = *static_cast<TestBackend*>(context);
    ++backend.taskIdCount;
    return XWalkHal::string("task-") + std::to_string(backend.taskIdCount);
}

/** @brief Returns the complete callback table used by valid test scenarios. */
XWalkHal::XWalkSpeakerCallbacks speakerCallbacks()
{
    return {&enableOutput, &disableOutput, &decodeAudio, &openStream,
        &writeStream, &closeStream, &createTaskId};
}

/** @brief Verifies output lifecycle, task progress, pause, resume, and stop. */
void testPlaybackControl(const XWalkHal::filesystempath& wavePath)
{
    TestBackend backend;
    const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
    {
        XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
        assert(speaker.isSpeakerEnabled());
        assert(backend.enableCount == 1U);
        const XWalkHal::string taskId = speaker.play(wavePath.string());
        XWalkHal::common::sleepMilliseconds(10U);
        speaker.pause(taskId);
        const XWalkHal::XWalkSpeakerProgress pausedProgress = speaker.getProgress(taskId);
        assert(!pausedProgress.isPlaying);
        assert(pausedProgress.totalFrames == backend.decodedFrameCount);
        assert(pausedProgress.progressRatio >= 0.0);
        assert(pausedProgress.progressRatio <= 1.0);
        speaker.resume(taskId);
        assert(speaker.stop(taskId));
        assert(!speaker.stop(taskId));
        assert(speaker.listTasks().empty());
        assert(backend.openCount == 1U);
        assert(backend.closeCount == 1U);
    }
    assert(backend.disableCount == 1U);
}

/** @brief Verifies Librosa routing and automatic cleanup after normal completion. */
void testFormatAndCompletion(const XWalkHal::filesystempath& compressedPath)
{
    TestBackend backend;
    backend.decodedFrameCount = 1'024U;
    const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
    XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
    static_cast<void>(speaker.play(compressedPath.string()));
    XWalkHal::common::sleepMilliseconds(20U);
    assert(speaker.listTasks().empty());
    assert(backend.handler == XWalkHal::XWalkSpeakerAudioHandler::Librosa);
    assert(backend.closeCount == 1U);
}

/** @brief Verifies rejection when all eight bounded playback slots are occupied. */
void testTaskCapacity(const XWalkHal::filesystempath& wavePath)
{
    TestBackend backend;
    const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
    XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
    for (XWalkHal::uint32 taskCount = 0U;
        taskCount < XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT; ++taskCount)
    {
        static_cast<void>(speaker.play(wavePath.string()));
    }
    assert(speaker.listTasks().size() == XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT);

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speaker.play(wavePath.string()));
    });
}

/** @brief Verifies unsupported paths, invalid audio, and incomplete callbacks. */
void testValidation(const XWalkHal::filesystempath& unsupportedPath,
    const XWalkHal::filesystempath& missingPath)
{
    TestBackend backend;
    XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
        callbacks.writeStream = nullptr;
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
    });

    callbacks = speakerCallbacks();
    XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speaker.play(unsupportedPath.string()));
    });

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speaker.play(missingPath.string()));
    });

    backend.invalidAudio = true;
    XWalkHal::filesystempath validPath = unsupportedPath;
    validPath += ".wav";
    XWalkHal::outputfilestream validFile(validPath, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
    validFile.close();
    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(speaker.play(validPath.string()));
    });
    static_cast<void>(XWalkHal::removeFilesystemEntry(validPath));
}

/** @brief Verifies that a worker backend failure terminates the process. */
void testWorkerFailure(const XWalkHal::filesystempath& wavePath)
{
    xwalk::hal::test::expectFailure([&]()
    {
        TestBackend backend;
        backend.failWrite = true;
        const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
        XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
        static_cast<void>(speaker.play(wavePath.string()));
        XWalkHal::common::sleepMilliseconds(20U);
    });
}

/** @brief Creates an empty test file at one module-local build path. */
void createTestFile(const XWalkHal::filesystempath& path)
{
    XWalkHal::outputfilestream file(path, XWalkHal::FILE_OPEN_WRITE_TRUNCATE);
    assert(file.is_open());
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs all host-side speaker tests using module-local files.
 *
 * @param[in] argumentCount
 * Number of command-line arguments; exactly four are required.
 *
 * @param[in] arguments
 * Executable, WAV, compressed, and unsupported test paths.
 *
 * @return
 * Zero after every assertion passes.
 */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    assert(argumentCount == 4);
    const XWalkHal::filesystempath wavePath(arguments[1]);
    const XWalkHal::filesystempath compressedPath(arguments[2]);
    const XWalkHal::filesystempath unsupportedPath(arguments[3]);
    XWalkHal::filesystempath missingPath = wavePath;
    missingPath.replace_extension(".missing.wav");
    createTestFile(wavePath);
    createTestFile(compressedPath);
    createTestFile(unsupportedPath);

    testPlaybackControl(wavePath);
    testFormatAndCompletion(compressedPath);
    testTaskCapacity(wavePath);
    testValidation(unsupportedPath, missingPath);
    testWorkerFailure(wavePath);

    static_cast<void>(XWalkHal::removeFilesystemEntry(wavePath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(compressedPath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(unsupportedPath));
    return 0;
}
