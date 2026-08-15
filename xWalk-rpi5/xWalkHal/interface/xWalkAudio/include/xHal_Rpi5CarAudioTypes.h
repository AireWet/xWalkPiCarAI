/******************************************************************************
 * @file        xHal_Rpi5CarAudioTypes.h
 * @brief       Declares shared ALSA audio configuration and operation types.
 *
 * @details
 * Defines backend-neutral PCM format, stream configuration, opaque handles,
 * and injected ALSA-operation callbacks used by the Linux audio owner.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio
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

#ifndef XHAL_RPI5CAR_AUDIO_TYPES_H
#define XHAL_RPI5CAR_AUDIO_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Enumeration declarations
     ******************************************************************************/

    /**
     * @enum XWalkAudioSampleFormat
     * @brief Selects the interleaved PCM representation negotiated with ALSA.
     */
    enum class XWalkAudioSampleFormat : uint8
    {
        /**
         * @brief Signed sixteen-bit little-endian PCM samples.
         */
        Signed16LittleEndian = 0U,

        /**
         * @brief IEEE single-precision little-endian PCM samples.
         */
        Float32LittleEndian = 1U
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkAudioStreamConfiguration
     * @brief Describes one interleaved ALSA playback stream.
     */
    struct XWalkAudioStreamConfiguration
    {
            /**
             * @brief Positive PCM sample rate in Hertz.
             */
            uint32 sampleRateHz{};

            /**
             * @brief Interleaved channel count from one through eight.
             */
            uint8 channelCount{};

            /**
             * @brief PCM representation used by every supplied sample.
             */
            XWalkAudioSampleFormat format{XWalkAudioSampleFormat::Signed16LittleEndian};

            /**
             * @brief Requested ALSA period size from one through 4,096 frames.
             */
            uint32 periodFrames{};

            /**
             * @brief Requested ALSA buffering latency in positive microseconds.
             */
            uint32 latencyUs{XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Opaque non-null PCM handle owned by the shared audio backend. */
    using audiopcmhandle = contextpointer;

    /** @brief Opaque non-null mixer handle owned by the shared audio backend. */
    using audiomixerhandle = contextpointer;

    /**
     * @brief Opens one ALSA PCM playback handle.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in] deviceName
     * Non-empty ALSA PCM device name valid for this synchronous call.
     *
     * @return
     * Non-null backend-owned PCM handle, or null when opening fails.
     */
    using audioopenpcmcallback = audiopcmhandle (*)(contextpointer context, stringview deviceName);

    /**
     * @brief Negotiates one opened PCM stream configuration.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null opened PCM handle.
     *
     * @param[in] configuration
     * Validated format, rate, channel, period, and latency request.
     *
     * @return
     * `true` when ALSA accepts the complete configuration; otherwise `false`.
     */
    using audioconfigurepcmcallback = boolean (*)(contextpointer context,
                                                  audiopcmhandle pcmHandle,
                                                  const XWalkAudioStreamConfiguration& configuration);

    /**
     * @brief Writes consecutive complete PCM frames.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null configured PCM handle.
     *
     * @param[in] pcmData
     * Immutable complete interleaved PCM payload.
     *
     * @param[in] byteOffset
     * Offset of the first frame supplied to this attempt, in bytes.
     *
     * @param[in] frameCount
     * Positive number of complete frames requested in this attempt.
     *
     * @return
     * Positive written frame count, zero for no progress, or a negative ALSA error.
     */
    using audiowritepcmcallback = int32 (*)(
        contextpointer context, audiopcmhandle pcmHandle, const bytevector& pcmData, size byteOffset, size frameCount);

    /**
     * @brief Attempts recovery from one negative ALSA PCM result.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null configured PCM handle.
     *
     * @param[in] errorValue
     * Negative value returned by the write operation.
     *
     * @return
     * `true` when the stream is ready for another write attempt; otherwise `false`.
     */
    using audiorecoverpcmcallback = boolean (*)(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue);

    /**
     * @brief Drains queued frames and closes one PCM handle without throwing.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null backend-owned handle that becomes invalid after this call.
     */
    using audioclosepcmcallback = void (*)(contextpointer context, audiopcmhandle pcmHandle);

    /**
     * @brief Opens and initializes one ALSA mixer handle.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in] deviceName
     * Non-empty configured ALSA mixer device name.
     *
     * @return
     * Non-null backend-owned mixer handle, or null when initialization fails.
     */
    using audioopenmixercallback = audiomixerhandle (*)(contextpointer context, stringview deviceName);

    /**
     * @brief Applies one percentage to a mixer playback element.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in,out] mixerHandle
     * Non-null initialized mixer handle.
     *
     * @param[in] elementName
     * Non-empty ALSA simple-element name.
     *
     * @param[in] volumePercent
     * Volume in the inclusive range zero through one hundred percent.
     *
     * @return
     * `true` when the element exists and accepts the volume; otherwise `false`.
     */
    using audiosetmixervolumecallback = boolean (*)(contextpointer context,
                                                    audiomixerhandle mixerHandle,
                                                    stringview elementName,
                                                    uint8 volumePercent);

    /**
     * @brief Closes one mixer handle without throwing.
     *
     * @param[in,out] context
     * Nullable non-owning operation context.
     *
     * @param[in,out] mixerHandle
     * Non-null backend-owned handle that becomes invalid after this call.
     */
    using audioclosemixercallback = void (*)(contextpointer context, audiomixerhandle mixerHandle);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkAudioAlsaOperations
     * @brief Contains the complete injected ALSA operation seam.
     */
    struct XWalkAudioAlsaOperations
    {
            /** @brief Opens one PCM playback handle. */
            audioopenpcmcallback openPcm{nullptr};
            /** @brief Negotiates one PCM stream. */
            audioconfigurepcmcallback configurePcm{nullptr};
            /** @brief Writes one bounded PCM frame range. */
            audiowritepcmcallback writePcm{nullptr};
            /** @brief Recovers one failed PCM write. */
            audiorecoverpcmcallback recoverPcm{nullptr};
            /** @brief Drains and closes one PCM handle. */
            audioclosepcmcallback closePcm{nullptr};
            /** @brief Opens and initializes one mixer handle. */
            audioopenmixercallback openMixer{nullptr};
            /** @brief Applies one mixer-element volume. */
            audiosetmixervolumecallback setMixerVolume{nullptr};
            /** @brief Closes one mixer handle. */
            audioclosemixercallback closeMixer{nullptr};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_AUDIO_TYPES_H */
