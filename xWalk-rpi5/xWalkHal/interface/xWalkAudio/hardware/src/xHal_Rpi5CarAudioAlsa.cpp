/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsa.cpp
 * @brief       Defines shared ALSA stream, write, recovery, and mixer behavior.
 *
 * @details
 * Retains configured PCM handles, completes short writes, bounds underrun
 * recovery attempts, applies mixer volume, and releases streams explicitly.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio ALSA Backend
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

#include "xHal_Rpi5CarAudioAlsa.h"

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
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Opens, negotiates, and retains one PCM playback stream.
     *
     * @param[in] configuration
     * Valid sample rate, channel count, format, period size, and latency.
     *
     * @return
     * Non-null opaque PCM handle owned until `closeStream()` or destruction.
     *
     * @throws std::invalid_argument
     * If the configuration contains a zero required value.
     *
     * @throws std::out_of_range
     * If a limit is exceeded or all stream slots are occupied.
     *
     * @throws std::runtime_error
     * If ALSA cannot open or configure the selected PCM device.
     */
    audiopcmhandle XWalkAudioAlsa::openStream(const XWalkAudioStreamConfiguration& configuration)
    {
        const XWalkAudioStreamConfiguration validatedConfiguration = validateConfiguration(configuration);
        const mutexlock lock(mutex);
        const size slotIndex = availableStreamIndex();
        audiopcmhandle pcmHandle = operations.openPcm(operationContextPointer, pcmDeviceNameValue);
        if (pcmHandle == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_EXCEPTION, "ALSA audio PCM device could not be opened");
        }
        const hal::boolean pcmConfigured =
            operations.configurePcm(operationContextPointer, pcmHandle, validatedConfiguration);
        if (pcmConfigured == false)
        {
            operations.closePcm(operationContextPointer, pcmHandle);
            XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio PCM configuration failed");
        }
        pcmHandles[slotIndex] = pcmHandle;
        streamConfigurations[slotIndex] = validatedConfiguration;
        XWALK_HAL_TRACE_UID2(RPI .086,
                             "ALSA audio stream opened at %u Hertz with %u channel(s)",
                             validatedConfiguration.sampleRateHz,
                             static_cast<uint32>(validatedConfiguration.channelCount));
        return pcmHandle;
    }

    /**
     * @brief Writes one complete interleaved PCM payload with bounded recovery.
     *
     * @param[in,out] streamHandle
     * Non-null handle currently owned by this backend.
     *
     * @param[in] pcmData
     * Payload containing exactly `frameCount` complete frames.
     *
     * @param[in] frameCount
     * Positive number of interleaved frames represented by `pcmData`.
     *
     * @throws std::invalid_argument
     * If the handle, frame count, or payload is invalid.
     *
     * @throws std::runtime_error
     * If writes stop progressing or recovery exceeds its bounded attempts.
     */
    void XWalkAudioAlsa::writeFrames(audiopcmhandle streamHandle, const bytevector& pcmData, size frameCount)
    {
        if (frameCount == 0U)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "ALSA audio frame count must not be zero");
        }

        const mutexlock lock(mutex);
        const size slotIndex = streamIndex(streamHandle);
        const XWalkAudioStreamConfiguration& configuration = streamConfigurations[slotIndex];
        const size periodFrames = static_cast<size>(configuration.periodFrames);
        if (frameCount > periodFrames)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "ALSA audio write exceeds the configured period size");
        }
        const size sampleBytes = bytesPerSample(configuration.format);
        const size channelCount = static_cast<size>(configuration.channelCount);
        const size bytesPerFrame = sampleBytes * channelCount;
        const hal::boolean frameCountBytesPerFramePcmDataInvalid =
            static_cast<hal::boolean>((frameCount > (std::numeric_limits<size>::max() / bytesPerFrame)) ||
                                      (pcmData.size() != (frameCount * bytesPerFrame)));
        if (frameCountBytesPerFramePcmDataInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "ALSA audio payload must contain complete configured frames");
        }

        size writtenFrames{};
        uint32 recoveryAttemptCount{};
        while (writtenFrames < frameCount)
        {
            const size remainingFrames = frameCount - writtenFrames;
            const size maximumAttemptFrames = static_cast<size>(std::numeric_limits<int32>::max());
            const size attemptFrames = std::min(remainingFrames, maximumAttemptFrames);
            const size byteOffset = writtenFrames * bytesPerFrame;
            const int32 writeResult =
                operations.writePcm(operationContextPointer, streamHandle, pcmData, byteOffset, attemptFrames);
            if (writeResult > 0)
            {
                const size completedFrames = static_cast<size>(writeResult);
                if (completedFrames > attemptFrames)
                {
                    XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio write returned too many frames");
                }
                writtenFrames += completedFrames;
            }
            else if (writeResult == 0)
            {
                XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio write made no progress");
            }
            else
            {
                ++recoveryAttemptCount;
                if (recoveryAttemptCount > XHAL_RPI5CAR_AUDIO_RECOVERY_ATTEMPT_COUNT)
                {
                    XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio write recovery failed");
                }
                const hal::boolean recovered =
                    operations.recoverPcm(operationContextPointer, streamHandle, writeResult);
                if (recovered == false)
                {
                    XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio write recovery failed");
                }
            }
        }
        XWALK_HAL_TRACE_UID1(RPI .087, "ALSA audio wrote %zu frame(s)", frameCount);
    }

    /**
     * @brief Drains, closes, and releases one owned PCM stream.
     *
     * @param[in,out] streamHandle
     * Non-null handle currently owned by this backend.
     *
     * @throws std::invalid_argument
     * If the handle is null or is not owned by this backend.
     *
     * @warning
     * The injected close callback must not throw.
     */
    void XWalkAudioAlsa::closeStream(audiopcmhandle streamHandle)
    {
        const mutexlock lock(mutex);
        const size slotIndex = streamIndex(streamHandle);
        operations.closePcm(operationContextPointer, streamHandle);
        pcmHandles[slotIndex] = nullptr;
        streamConfigurations[slotIndex] = {};
        XWALK_HAL_TRACE_UID0(RPI .088, "ALSA audio stream closed");
    }

    /**
     * @brief Applies one bounded percentage to the configured mixer element.
     *
     * @param[in] volumePercent
     * Volume in the inclusive range zero through one hundred percent.
     *
     * @throws std::out_of_range
     * If the volume exceeds one hundred percent.
     *
     * @throws std::runtime_error
     * If the mixer element cannot accept the volume.
     */
    void XWalkAudioAlsa::setVolume(uint8 volumePercent)
    {
        if (volumePercent > 100U)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "ALSA audio volume exceeds one hundred percent");
        }
        const mutexlock lock(mutex);
        const hal::boolean volumeApplied =
            operations.setMixerVolume(operationContextPointer, mixerHandle, mixerElementNameValue, volumePercent);
        if (volumeApplied == false)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "ALSA audio mixer volume update failed");
        }
        XWALK_HAL_TRACE_UID1(RPI .089, "ALSA audio volume set to %u percent", static_cast<uint32>(volumePercent));
    }

    /**
     * @brief Reports how many PCM handles are currently owned.
     *
     * @return
     * Stream count from zero through eight.
     */
    size XWalkAudioAlsa::openStreamCount() const noexcept
    {
        const mutexlock lock(mutex);
        size streamCount{};
        for (const audiopcmhandle pcmHandle : pcmHandles)
        {
            if (pcmHandle != nullptr)
            {
                ++streamCount;
            }
        }
        return streamCount;
    }

} /* namespace xwalk::hal */
