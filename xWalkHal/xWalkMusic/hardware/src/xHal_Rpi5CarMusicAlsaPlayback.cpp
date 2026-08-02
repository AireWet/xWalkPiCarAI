/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaPlayback.cpp
 * @brief       Implements bounded shared-ALSA playback and workers.
 *
 * @details
 * Opens configured PCM streams, writes period-sized chunks, observes worker
 * pause and stop requests, repeats streamed music, and closes every stream.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic ALSA Adapter
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarMusicAlsa.h"

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
 * @brief Writes decoded PCM from one starting frame for the requested repetitions.
 *
 * @param[in] audioData
 * Valid decoded PCM data retained for this complete call.
 *
 * @param[in] startFrame
 * Initial frame below the total frame count; repetitions restart at frame zero.
 *
 * @param[in] loopCount
 * Non-negative number of additional complete repetitions.
 *
 * @param[in] observeMusicState
 * `true` to observe streamed-music pause and stop requests.
 *
 * @param[in] observeSoundState
 * `true` to observe the background-sound stop request.
 *
 * @post
 * The temporary PCM stream is closed after completion or a stop request.
 */
void XWalkMusicAlsa::writeAudio(const XWalkMusicAlsaAudioData& audioData,
    size startFrame, int32 loopCount, boolean observeMusicState,
    boolean observeSoundState)
{
    const XWalkAudioStreamConfiguration configuration{
        audioData.sampleRateHz,
        audioData.channelCount,
        XWalkAudioSampleFormat::Signed16LittleEndian,
        XHAL_RPI5CAR_MUSIC_ALSA_PERIOD_FRAMES,
        XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
    audiopcmhandle streamHandle = audioBackend->openStream(configuration);
    const size channelCount = static_cast<size>(audioData.channelCount);
    const size bytesPerFrame = channelCount * XHAL_RPI5CAR_MUSIC_SAMPLE_BYTES;
    const size totalFrames = audioData.pcmData.size() / bytesPerFrame;
    int32 completedLoopCount{};
    boolean playbackComplete = false;
    while (!playbackComplete)
    {
        size framePosition = (completedLoopCount == 0) ? startFrame : 0U;
        while (framePosition < totalFrames)
        {
            boolean stopRequested = false;
            boolean pauseRequested = false;
            if (observeMusicState || observeSoundState)
            {
                const mutexlock lock(stateMutex);
                stopRequested = (observeMusicState && musicStopRequested) ||
                    (observeSoundState && soundStopRequested);
                pauseRequested = observeMusicState && musicPauseRequested;
            }
            if (stopRequested)
            {
                audioBackend->closeStream(streamHandle);
                return;
            }
            if (pauseRequested)
            {
                common::sleepMilliseconds(XHAL_RPI5CAR_MUSIC_ALSA_PAUSE_POLL_INTERVAL_MS);
                continue;
            }

            const size remainingFrames = totalFrames - framePosition;
            const size periodFrames = static_cast<size>(XHAL_RPI5CAR_MUSIC_ALSA_PERIOD_FRAMES);
            const size frameCount = std::min(remainingFrames, periodFrames);
            const size firstByte = framePosition * bytesPerFrame;
            const size byteCount = frameCount * bytesPerFrame;
            const auto firstIterator = audioData.pcmData.begin() +
                static_cast<bytevector::difference_type>(firstByte);
            const auto finalIterator = firstIterator +
                static_cast<bytevector::difference_type>(byteCount);
            const bytevector periodData(firstIterator, finalIterator);
            audioBackend->writeFrames(streamHandle, periodData, frameCount);
            framePosition += frameCount;
        }
        if (completedLoopCount >= loopCount)
        {
            playbackComplete = true;
        }
        else
        {
            ++completedLoopCount;
        }
    }
    audioBackend->closeStream(streamHandle);
}

/**
 * @brief Runs the single retained background sound worker.
 *
 * @post
 * Background PCM playback has completed or observed a stop request.
 */
void XWalkMusicAlsa::soundPlaybackLoop() noexcept
{
    writeAudio(soundAudio, 0U, 0, false, true);
}

/**
 * @brief Runs the single retained streamed-music worker.
 *
 * @post
 * Streamed playback has completed all loops or observed a stop request.
 */
void XWalkMusicAlsa::musicPlaybackLoop() noexcept
{
    writeAudio(musicAudio, musicStartFrame, musicLoopCount, true, false);
}

} /* namespace xwalk::hal */
