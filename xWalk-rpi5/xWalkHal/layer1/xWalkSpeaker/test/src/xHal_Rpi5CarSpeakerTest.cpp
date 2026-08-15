/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerTest.cpp
 * @brief       Verifies Speaker behavior using named in-memory support.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Test
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarSpeaker.h"
#include "xHal_Rpi5CarSpeakerSimulationArguments.h"
#include "xHal_Rpi5CarSpeakerSimulationConfig.h"
#include "xHal_Rpi5CarSpeakerTestSupport.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarTrace.h"
namespace
{
    using namespace xwalk::hal::test::speaker;
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
            const XWalkHal::XWalkSpeakerProgress progress = speaker.getProgress(taskId);
            assert(progress.isPlaying == false);
            assert(progress.totalFrames == backend.decodedFrameCount);
            assert(progress.progressRatio >= 0.0);
            assert(progress.progressRatio <= 1.0);
            speaker.resume(taskId);
            assert(speaker.stop(taskId));
            assert(speaker.stop(taskId) == false);
            assert(speaker.listTasks().empty());
            assert(backend.openCount == 1U);
            assert(backend.closeCount == 1U);
        }
        assert(backend.disableCount == 1U);
    }
    /** @brief Verifies compressed routing and automatic completion cleanup. */
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
    /** @brief Verifies all eight bounded task slots can be occupied. */
    void testTaskCapacity(const XWalkHal::filesystempath& wavePath)
    {
        TestBackend backend;
        const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
        XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
        for (XWalkHal::uint32 count = 0U; count < XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT; ++count)
        {
            static_cast<void>(speaker.play(wavePath.string()));
        }
        assert(speaker.listTasks().size() == XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT);
    }
    /** @brief Verifies rejection of a ninth playback task. */
    void testTaskCapacityFailure(const XWalkHal::filesystempath& wavePath)
    {
        xwalk::hal::test::expectFailure(
            [&]()
            {
                TestBackend backend;
                const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
                XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
                for (XWalkHal::uint32 count = 0U; count < XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT; ++count)
                {
                    static_cast<void>(speaker.play(wavePath.string()));
                }
                static_cast<void>(speaker.play(wavePath.string()));
            });
    }
    /** @brief Verifies unsupported paths, invalid audio, and incomplete callbacks.
     */
    void testValidation(const XWalkHal::filesystempath& unsupportedPath, const XWalkHal::filesystempath& missingPath)
    {
        TestBackend backend;
        XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
        callbacks.writeStream = nullptr;
        xwalk::hal::test::expectFailure(
            [&]()
            {
                XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
            });
        callbacks = speakerCallbacks();
        XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speaker.play(unsupportedPath.string()));
            });
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speaker.play(missingPath.string()));
            });
        backend.invalidAudio = true;
        XWalkHal::filesystempath validPath = unsupportedPath;
        validPath += ".wav";
        createTestFile(validPath);
        xwalk::hal::test::expectFailure(
            [&]()
            {
                static_cast<void>(speaker.play(validPath.string()));
            });
        static_cast<void>(XWalkHal::removeFilesystemEntry(validPath));
    }
    /** @brief Verifies that a worker backend failure terminates the child process.
     */
    void testWorkerFailure(const XWalkHal::filesystempath& wavePath)
    {
        xwalk::hal::test::expectFailure(
            [&]()
            {
                TestBackend backend;
                backend.failWrite = true;
                const XWalkHal::XWalkSpeakerCallbacks callbacks = speakerCallbacks();
                XWalkHal::XWalkSpeaker speaker(&backend, callbacks);
                static_cast<void>(speaker.play(wavePath.string()));
                XWalkHal::common::sleepMilliseconds(20U);
            });
    }
    /** @brief Verifies persistent Speaker trace-selector behavior. */
    void testTraceSelection()
    {
        char executable[] = "xWalkSpeakerTest";
        char option[] = "--trace";
        char enableSelector[] = "RPI.316.enable";
        char disableSelector[] = "RPI.316.disable";
        char malformedSelector[] = "RPI.invalid.enable";
        XWalkHal::charpointer enableArguments[]{executable, option, enableSelector};
        xwalk::hal::sim::XWalkSpeakerSimulationArguments enable(3, enableArguments);
        assert(enable.valid());
        assert(enable.applyTraceUpdate());
        XWalkHal::charpointer disableArguments[]{executable, option, disableSelector};
        xwalk::hal::sim::XWalkSpeakerSimulationArguments disable(3, disableArguments);
        assert(disable.valid());
        assert(disable.applyTraceUpdate());
        XWalkHal::charpointer malformedArguments[]{executable, option, malformedSelector};
        const xwalk::hal::sim::XWalkSpeakerSimulationArguments malformed(3, malformedArguments);
        assert(malformed.valid() == false);
    }
} /* namespace */
/** @brief Runs host-side Speaker tests using build-local fixture files. */
XWalkHal::int32 main(XWalkHal::int32 argumentCount, XWalkHal::charpointer arguments[])
{
    assert(argumentCount == 5);
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_SPEAKER_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_SPEAKER_SIMULATION_TRACE_LOG_PATH);
    const XWalkHal::filesystempath wavePath(arguments[1]);
    const XWalkHal::filesystempath compressedPath(arguments[2]);
    const XWalkHal::filesystempath unsupportedPath(arguments[3]);
    const XWalkHal::stringview suiteMode(arguments[4]);
    XWALK_HAL_TRACE_UID2(RPI .319,
                         "xWalkSpeaker host tests started for %.*s",
                         static_cast<XWalkHal::int32>(suiteMode.size()),
                         suiteMode.data());
    XWalkHal::filesystempath missingPath = wavePath;
    missingPath.replace_extension(".missing.wav");
    createTestFile(wavePath);
    createTestFile(compressedPath);
    createTestFile(unsupportedPath);
    if (suiteMode == "concurrency")
    {
        testPlaybackControl(wavePath);
        testFormatAndCompletion(compressedPath);
        testTaskCapacity(wavePath);
        testTraceSelection();
    }
    else
    {
        assert(suiteMode == "failure");
        testTaskCapacityFailure(wavePath);
        testValidation(unsupportedPath, missingPath);
        testWorkerFailure(wavePath);
    }
    static_cast<void>(XWalkHal::removeFilesystemEntry(wavePath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(compressedPath));
    static_cast<void>(XWalkHal::removeFilesystemEntry(unsupportedPath));
    XWALK_HAL_TRACE_UID2(RPI .320,
                         "xWalkSpeaker host tests completed for %.*s",
                         static_cast<XWalkHal::int32>(suiteMode.size()),
                         suiteMode.data());
    return 0;
}
