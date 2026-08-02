/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerWorker.cpp
 * @brief       Implements bounded background audio-stream playback.
 *
 * @details
 * Opens one backend stream, writes decoded audio in bounded frame chunks,
 * observes pause and stop requests, and closes the stream.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker
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

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Executes one playback task.
 *
 * @param[in] taskIndex
 * Valid occupied task-slot index below the maximum task count.
 *
 * @post
 * The stream is closed and the slot is marked finished.
 */
void XWalkSpeaker::playbackLoop(size taskIndex) noexcept
{
    XWalkSpeakerTaskSlot& task = taskSlots[taskIndex];
    speakerstreamhandle openedStream = callbacks.openStream(backendContext,
        task.audioData.sampleRateHz, task.audioData.channelCount);
    if (openedStream == nullptr)
    {
        std::terminate();
    }
    {
        const mutexlock lock(stateMutex);
        task.stream = openedStream;
        task.playing = !task.pauseRequested;
    }

    const size channelCount = static_cast<size>(task.audioData.channelCount);
    const size totalFrames = task.audioData.samples.size() / channelCount;
    boolean playbackComplete = false;
    while (!playbackComplete)
    {
        boolean paused = false;
        size firstFrame = 0U;
        size frameCount = 0U;
        {
            const mutexlock lock(stateMutex);
            if (task.stopRequested || (task.positionFrames >= totalFrames))
            {
                playbackComplete = true;
                task.playing = false;
            }
            else if (task.pauseRequested)
            {
                paused = true;
                task.playing = false;
            }
            else
            {
                firstFrame = task.positionFrames;
                const size remainingFrames = totalFrames - firstFrame;
                const size maximumChunkFrames =
                    static_cast<size>(XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT);
                frameCount = std::min(remainingFrames, maximumChunkFrames);
                task.playing = true;
            }
        }

        if (paused)
        {
            common::sleepMilliseconds(XHAL_RPI5CAR_SPEAKER_PAUSE_POLL_INTERVAL_MS);
        }
        else if (!playbackComplete)
        {
            callbacks.writeStream(backendContext, openedStream, task.audioData,
                firstFrame, frameCount);
            const mutexlock lock(stateMutex);
            task.positionFrames = firstFrame + frameCount;
        }
    }

    callbacks.closeStream(backendContext, openedStream);
    const mutexlock lock(stateMutex);
    task.stream = nullptr;
    task.playing = false;
    task.finished = true;
}

} /* namespace xwalk::hal */
