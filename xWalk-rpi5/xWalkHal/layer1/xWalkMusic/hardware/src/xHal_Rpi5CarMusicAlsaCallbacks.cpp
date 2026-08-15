/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaCallbacks.cpp
 * @brief       Implements the concrete ALSA music callback table.
 *
 * @details
 * Routes output enable, sound effects, streamed transport, mixer volume, sound
 * duration, and generated tones through the shared ALSA adapter state.
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

#include "xHal_Rpi5CarTrace.h"
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
     * @brief Converts normalized music volume to mixer percent.
     *
     * @param[in] normalizedVolume
     * Finite normalized volume in the inclusive range zero through one.
     *
     * @return
     * Rounded mixer volume in the inclusive range zero through one hundred percent.
     *
     * @throws std::invalid_argument
     * If the value is not finite.
     *
     * @throws std::out_of_range
     * If the value is outside the normalized range.
     */
    uint8 XWalkMusicAlsa::volumePercent(float64 normalizedVolume)
    {
        const hal::boolean normalizedVolumeNotFinite = static_cast<hal::boolean>(!XHAL_IS_FINITE(normalizedVolume));
        if (normalizedVolumeNotFinite)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Music ALSA volume must be finite");
        }
        if ((normalizedVolume < 0.0) || (normalizedVolume > 1.0))
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Music ALSA normalized volume must be between zero and one");
        }
        const float64 scaledVolume = normalizedVolume * XHAL_RPI5CAR_MUSIC_VOLUME_PERCENT_DIVISOR;
        const uint32 roundedVolume = common::roundedValue(scaledVolume, "Music ALSA volume", 0U, 100U);
        return static_cast<uint8>(roundedVolume);
    }

    /**
     * @brief Implements the output-enable callback after ALSA construction.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @note
     * Shared ALSA mixer construction establishes output availability, so no extra
     * hardware operation is required here.
     */
    void XWalkMusicAlsa::enableOutput(contextpointer context)
    {
        static_cast<void>(adapter(context));
    }

    /**
     * @brief Implements synchronous sound-effect playback.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @param[in] filename
     * Non-empty file path decoded before playback.
     *
     * @param[in] normalizedVolume
     * Optional normalized mixer volume from zero through one.
     */
    void XWalkMusicAlsa::playSound(contextpointer context, stringview filename, optionalfloat64 normalizedVolume)
    {
        XWalkMusicAlsa& self = adapter(context);
        XWalkMusicAlsaAudioData audioData = self.operations.decodeAudio(self.decoderContext, filename);
        validateAudioData(audioData);
        const hal::boolean normalizedVolumeProvided = static_cast<hal::boolean>(normalizedVolume.has_value());
        if (normalizedVolumeProvided)
        {
            self.audioBackend->setVolume(volumePercent(*normalizedVolume));
        }
        self.writeAudio(audioData, 0U, 0, false, false);
    }

    /**
     * @brief Implements background sound-effect playback.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @param[in] filename
     * Non-empty file path decoded before worker creation.
     *
     * @param[in] normalizedVolume
     * Optional normalized mixer volume from zero through one.
     */
    void
    XWalkMusicAlsa::playSoundBackground(contextpointer context, stringview filename, optionalfloat64 normalizedVolume)
    {
        XWalkMusicAlsa& self = adapter(context);
        self.stopSoundWorker();
        XWalkMusicAlsaAudioData audioData = self.operations.decodeAudio(self.decoderContext, filename);
        validateAudioData(audioData);
        const hal::boolean normalizedVolumeProvided = static_cast<hal::boolean>(normalizedVolume.has_value());
        if (normalizedVolumeProvided)
        {
            self.audioBackend->setVolume(volumePercent(*normalizedVolume));
        }
        {
            const mutexlock lock(self.stateMutex);
            self.soundAudio = std::move(audioData);
            self.soundStopRequested = false;
        }
        self.soundWorker = threadhandle(&XWalkMusicAlsa::soundPlaybackLoop, &self);
    }

    /**
     * @brief Implements streamed-music playback.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @param[in] filename
     * Non-empty file path decoded before worker creation.
     *
     * @param[in] loops
     * Non-negative number of additional complete repetitions.
     *
     * @param[in] startSeconds
     * Finite non-negative initial playback offset in seconds.
     */
    void XWalkMusicAlsa::playMusic(contextpointer context, stringview filename, int32 loops, float64 startSeconds)
    {
        XWalkMusicAlsa& self = adapter(context);
        self.stopMusicWorker();
        XWalkMusicAlsaAudioData audioData = self.operations.decodeAudio(self.decoderContext, filename);
        validateAudioData(audioData);
        const float64 sampleRateHz = static_cast<float64>(audioData.sampleRateHz);
        const float64 startFrameValue = startSeconds * sampleRateHz;
        const float64 maximumStartFrame = static_cast<float64>(std::numeric_limits<size>::max());
        const hal::boolean startFrameMaximumStartFrameInvalid =
            static_cast<hal::boolean>(!XHAL_IS_FINITE(startFrameValue) || (startFrameValue > maximumStartFrame));
        if (startFrameMaximumStartFrameInvalid)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Music ALSA playback offset is unrepresentable");
        }
        const size startFrame = static_cast<size>(startFrameValue);
        const size channelCount = static_cast<size>(audioData.channelCount);
        const size bytesPerFrame = channelCount * XHAL_RPI5CAR_MUSIC_SAMPLE_BYTES;
        const size totalFrames = audioData.pcmData.size() / bytesPerFrame;
        if (startFrame >= totalFrames)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Music ALSA playback offset exceeds the decoded audio");
        }
        {
            const mutexlock lock(self.stateMutex);
            self.musicAudio = std::move(audioData);
            self.musicStartFrame = startFrame;
            self.musicLoopCount = loops;
            self.musicStopRequested = false;
            self.musicPauseRequested = false;
        }
        self.musicWorker = threadhandle(&XWalkMusicAlsa::musicPlaybackLoop, &self);
    }

    /**
     * @brief Implements shared ALSA mixer volume control.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @param[in] normalizedVolume
     * Finite normalized volume in the inclusive range zero through one.
     */
    void XWalkMusicAlsa::setMusicVolume(contextpointer context, float64 normalizedVolume)
    {
        XWalkMusicAlsa& self = adapter(context);
        self.audioBackend->setVolume(volumePercent(normalizedVolume));
    }

    /**
     * @brief Implements streamed-music stop control.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     */
    void XWalkMusicAlsa::stopMusic(contextpointer context)
    {
        adapter(context).stopMusicWorker();
    }

    /**
     * @brief Implements streamed-music pause control.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     */
    void XWalkMusicAlsa::pauseMusic(contextpointer context)
    {
        XWalkMusicAlsa& self = adapter(context);
        const mutexlock lock(self.stateMutex);
        self.musicPauseRequested = true;
    }

    /**
     * @brief Implements streamed-music resume control.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     */
    void XWalkMusicAlsa::resumeMusic(contextpointer context)
    {
        XWalkMusicAlsa& self = adapter(context);
        const mutexlock lock(self.stateMutex);
        self.musicPauseRequested = false;
    }

    /**
     * @brief Implements decoded sound duration measurement.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @param[in] filename
     * Non-empty file path decoded for measurement.
     *
     * @return
     * Positive decoded duration in seconds.
     */
    float64 XWalkMusicAlsa::getSoundLength(contextpointer context, stringview filename)
    {
        XWalkMusicAlsa& self = adapter(context);
        const XWalkMusicAlsaAudioData audioData = self.operations.decodeAudio(self.decoderContext, filename);
        validateAudioData(audioData);
        const size channelCount = static_cast<size>(audioData.channelCount);
        const size bytesPerFrame = channelCount * XHAL_RPI5CAR_MUSIC_SAMPLE_BYTES;
        const size totalFrames = audioData.pcmData.size() / bytesPerFrame;
        const float64 frameCount = static_cast<float64>(totalFrames);
        const float64 sampleRateHz = static_cast<float64>(audioData.sampleRateHz);
        return frameCount / sampleRateHz;
    }

    /**
     * @brief Implements generated signed sixteen-bit PCM tone playback.
     *
     * @param[in,out] context
     * Non-null music ALSA adapter context.
     *
     * @param[in] pcmData
     * Complete signed sixteen-bit little-endian interleaved PCM bytes.
     *
     * @param[in] sampleRateHz
     * Positive PCM sample rate in Hertz.
     *
     * @param[in] channelCount
     * Interleaved channel count from one through eight.
     */
    void
    XWalkMusicAlsa::playTone(contextpointer context, const bytevector& pcmData, uint32 sampleRateHz, uint8 channelCount)
    {
        const hal::boolean pcmDataEmpty = static_cast<hal::boolean>(pcmData.empty());
        if (pcmDataEmpty)
        {
            return;
        }
        XWalkMusicAlsaAudioData audioData{pcmData, sampleRateHz, channelCount};
        validateAudioData(audioData);
        adapter(context).writeAudio(audioData, 0U, 0, false, false);
    }

} /* namespace xwalk::hal */
