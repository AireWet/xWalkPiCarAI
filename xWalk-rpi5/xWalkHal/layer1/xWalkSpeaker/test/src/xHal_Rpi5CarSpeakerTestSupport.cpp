/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerTestSupport.cpp
 * @brief       Implements reusable xWalkSpeaker host-test support.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Test
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarSpeakerTestSupport.h"
#include "xHal_Rpi5CarTrace.h"
namespace xwalk::hal::test::speaker
{
    void enableOutput(contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->enableCount;
    }
    void disableOutput(contextpointer context)
    {
        ++static_cast<TestBackend*>(context)->disableCount;
    }
    XWalkSpeakerAudioData decodeAudio(contextpointer context, stringview filePath, XWalkSpeakerAudioHandler handler)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        static_cast<void>(filePath);
        ++backend.decodeCount;
        backend.handler = handler;
        if (backend.invalidAudio)
        {
            return {{0.0}, 0U, 1U};
        }
        return {float64vector(backend.decodedFrameCount, 0.25), 44'100U, 1U};
    }
    speakerstreamhandle openStream(contextpointer context, uint32 sampleRateHz, uint8 channelCount)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(sampleRateHz == 44'100U);
        assert(channelCount == 1U);
        const mutexlock lock(backend.callbackMutex);
        ++backend.openCount;
        return context;
    }
    void writeStream(contextpointer context,
                     speakerstreamhandle stream,
                     const XWalkSpeakerAudioData& audioData,
                     size firstFrame,
                     size frameCount)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(stream == context);
        assert((firstFrame + frameCount) <= audioData.samples.size());
        assert(frameCount <= XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT);
        if (backend.failWrite)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Simulated speaker stream failure");
        }
        {
            const mutexlock lock(backend.callbackMutex);
            ++backend.writeCount;
        }
        backend.callbackCondition.notify_all();
        if (backend.writeDelayEnabled)
        {
            common::sleepMilliseconds(2U);
        }
    }
    void closeStream(contextpointer context, speakerstreamhandle stream)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        assert(stream == context);
        const mutexlock lock(backend.callbackMutex);
        ++backend.closeCount;
    }
    string createTaskId(contextpointer context)
    {
        TestBackend& backend = *static_cast<TestBackend*>(context);
        ++backend.taskIdCount;
        return string("task-") + std::to_string(backend.taskIdCount);
    }
    XWalkSpeakerCallbacks speakerCallbacks()
    {
        return {&enableOutput, &disableOutput, &decodeAudio, &openStream, &writeStream, &closeStream, &createTaskId};
    }
    void createTestFile(const filesystempath& path)
    {
        outputfilestream file(path, FILE_OPEN_WRITE_TRUNCATE);
        assert(file.is_open());
    }
    boolean waitForWriteCount(TestBackend& backend, uint32 expectedCount)
    {
        uniquemutexlock lock(backend.callbackMutex);
        return backend.callbackCondition.wait_for(lock,
                                                  millisecondduration(1'000),
                                                  [&backend, expectedCount]()
                                                  {
                                                      return backend.writeCount >= expectedCount;
                                                  });
    }
} /* namespace xwalk::hal::test::speaker */
