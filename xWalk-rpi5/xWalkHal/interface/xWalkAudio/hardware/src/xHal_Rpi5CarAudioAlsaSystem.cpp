/******************************************************************************
 * @file        xHal_Rpi5CarAudioAlsaSystem.cpp
 * @brief       Defines libasound operations for the shared audio backend.
 *
 * @details
 * Opens, configures, writes, recovers, drains, and closes ALSA PCM resources
 * and opens, updates, and closes the configured ALSA simple mixer element.
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

#include <alsa/asoundlib.h>

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
     * @brief Returns the real Linux ALSA operation table.
     *
     * @return
     * Complete stateless callback table backed by libasound.
     */
    XWalkAudioAlsaOperations XWalkAudioAlsa::systemOperations() noexcept
    {
        return {&systemOpenPcm,
                &systemConfigurePcm,
                &systemWritePcm,
                &systemRecoverPcm,
                &systemClosePcm,
                &systemOpenMixer,
                &systemSetMixerVolume,
                &systemCloseMixer};
    }

    /**
     * @brief Opens one real ALSA PCM playback handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in] deviceName
     * Non-empty ALSA PCM device name.
     *
     * @return
     * Non-null opened handle, or null when ALSA rejects the device.
     */
    audiopcmhandle XWalkAudioAlsa::systemOpenPcm(contextpointer context, stringview deviceName)
    {
        static_cast<void>(context);
        snd_pcm_t* pcmHandle{nullptr};
        const string ownedDeviceName{deviceName};
        const hal::boolean pcmOpenFailed = static_cast<hal::boolean>(
            ::snd_pcm_open(&pcmHandle, ownedDeviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0) < 0);
        if (pcmOpenFailed)
        {
            return nullptr;
        }
        return pcmHandle;
    }

    /**
     * @brief Configures one real ALSA PCM playback handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null opened ALSA PCM handle.
     *
     * @param[in] configuration
     * Validated format, rate, channel, period, and latency request.
     *
     * @return
     * `true` when ALSA accepts and prepares the complete configuration.
     */
    boolean XWalkAudioAlsa::systemConfigurePcm(contextpointer context,
                                               audiopcmhandle pcmHandle,
                                               const XWalkAudioStreamConfiguration& configuration)
    {
        static_cast<void>(context);
        snd_pcm_t* const nativeHandle = static_cast<snd_pcm_t*>(pcmHandle);
        snd_pcm_hw_params_t* hardwareParameters{nullptr};
        snd_pcm_hw_params_alloca(&hardwareParameters);

        snd_pcm_format_t nativeFormat = SND_PCM_FORMAT_UNKNOWN;
        switch (configuration.format)
        {
            case XWalkAudioSampleFormat::Signed16LittleEndian:
                nativeFormat = SND_PCM_FORMAT_S16_LE;
                break;
            case XWalkAudioSampleFormat::Float32LittleEndian:
                nativeFormat = SND_PCM_FORMAT_FLOAT_LE;
                break;
            default:
                return false;
        }

        unsigned int negotiatedRateHz = configuration.sampleRateHz;
        snd_pcm_uframes_t negotiatedPeriodFrames = configuration.periodFrames;
        unsigned int negotiatedLatencyUs = configuration.latencyUs;
        int direction{};
        const boolean parametersAccepted =
            (::snd_pcm_hw_params_any(nativeHandle, hardwareParameters) >= 0) &&
            (::snd_pcm_hw_params_set_access(nativeHandle, hardwareParameters, SND_PCM_ACCESS_RW_INTERLEAVED) >= 0) &&
            (::snd_pcm_hw_params_set_format(nativeHandle, hardwareParameters, nativeFormat) >= 0) &&
            (::snd_pcm_hw_params_set_channels(nativeHandle, hardwareParameters, configuration.channelCount) >= 0) &&
            (::snd_pcm_hw_params_set_rate_near(nativeHandle, hardwareParameters, &negotiatedRateHz, &direction) >= 0) &&
            (::snd_pcm_hw_params_set_period_size_near(
                 nativeHandle, hardwareParameters, &negotiatedPeriodFrames, &direction) >= 0) &&
            (::snd_pcm_hw_params_set_buffer_time_near(
                 nativeHandle, hardwareParameters, &negotiatedLatencyUs, &direction) >= 0) &&
            (::snd_pcm_hw_params(nativeHandle, hardwareParameters) >= 0) && (::snd_pcm_prepare(nativeHandle) >= 0);
        return parametersAccepted && (negotiatedRateHz == configuration.sampleRateHz);
    }

    /**
     * @brief Writes frames to one real ALSA PCM playback handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null configured ALSA PCM handle.
     *
     * @param[in] pcmData
     * Immutable complete interleaved PCM payload.
     *
     * @param[in] byteOffset
     * Byte offset of the first requested frame.
     *
     * @param[in] frameCount
     * Positive bounded number of frames requested.
     *
     * @return
     * Written frame count or a negative ALSA error value.
     */
    int32 XWalkAudioAlsa::systemWritePcm(
        contextpointer context, audiopcmhandle pcmHandle, const bytevector& pcmData, size byteOffset, size frameCount)
    {
        static_cast<void>(context);
        snd_pcm_t* const nativeHandle = static_cast<snd_pcm_t*>(pcmHandle);
        const void* const framePointer = pcmData.data() + byteOffset;
        const snd_pcm_sframes_t writeResult =
            ::snd_pcm_writei(nativeHandle, framePointer, static_cast<snd_pcm_uframes_t>(frameCount));
        return static_cast<int32>(writeResult);
    }

    /**
     * @brief Recovers one real ALSA PCM playback handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null configured ALSA PCM handle.
     *
     * @param[in] errorValue
     * Negative ALSA result returned by `snd_pcm_writei`.
     *
     * @return
     * `true` when ALSA recovers the stream; otherwise `false`.
     */
    boolean XWalkAudioAlsa::systemRecoverPcm(contextpointer context, audiopcmhandle pcmHandle, int32 errorValue)
    {
        static_cast<void>(context);
        snd_pcm_t* const nativeHandle = static_cast<snd_pcm_t*>(pcmHandle);
        return ::snd_pcm_recover(nativeHandle, errorValue, 1) >= 0;
    }

    /**
     * @brief Drains and closes one real ALSA PCM playback handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in,out] pcmHandle
     * Non-null handle that becomes invalid after this call.
     */
    void XWalkAudioAlsa::systemClosePcm(contextpointer context, audiopcmhandle pcmHandle)
    {
        static_cast<void>(context);
        snd_pcm_t* const nativeHandle = static_cast<snd_pcm_t*>(pcmHandle);
        static_cast<void>(::snd_pcm_drain(nativeHandle));
        static_cast<void>(::snd_pcm_close(nativeHandle));
    }

    /**
     * @brief Opens and initializes one real ALSA mixer handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in] deviceName
     * Non-empty ALSA mixer device name.
     *
     * @return
     * Non-null initialized mixer handle, or null when setup fails.
     */
    audiomixerhandle XWalkAudioAlsa::systemOpenMixer(contextpointer context, stringview deviceName)
    {
        static_cast<void>(context);
        snd_mixer_t* mixerHandle{nullptr};
        const string ownedDeviceName{deviceName};
        const boolean opened = ::snd_mixer_open(&mixerHandle, 0) >= 0;
        const boolean attached = opened && (::snd_mixer_attach(mixerHandle, ownedDeviceName.c_str()) >= 0);
        const boolean registered = attached && (::snd_mixer_selem_register(mixerHandle, nullptr, nullptr) >= 0);
        const boolean loaded = registered && (::snd_mixer_load(mixerHandle) >= 0);
        if (!loaded)
        {
            if (mixerHandle != nullptr)
            {
                static_cast<void>(::snd_mixer_close(mixerHandle));
            }
            return nullptr;
        }
        return mixerHandle;
    }

    /**
     * @brief Applies volume to one real ALSA mixer element.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in,out] mixerHandle
     * Non-null initialized ALSA mixer handle.
     *
     * @param[in] elementName
     * Non-empty ALSA simple-element name.
     *
     * @param[in] volumePercent
     * Volume in the inclusive range zero through one hundred percent.
     *
     * @return
     * `true` when the named playback element accepts the volume.
     */
    boolean XWalkAudioAlsa::systemSetMixerVolume(contextpointer context,
                                                 audiomixerhandle mixerHandle,
                                                 stringview elementName,
                                                 uint8 volumePercent)
    {
        static_cast<void>(context);
        snd_mixer_t* const nativeHandle = static_cast<snd_mixer_t*>(mixerHandle);
        snd_mixer_selem_id_t* elementIdentifier{nullptr};
        snd_mixer_selem_id_alloca(&elementIdentifier);
        ::snd_mixer_selem_id_set_index(elementIdentifier, 0U);
        const string ownedElementName{elementName};
        ::snd_mixer_selem_id_set_name(elementIdentifier, ownedElementName.c_str());
        snd_mixer_elem_t* const mixerElement = ::snd_mixer_find_selem(nativeHandle, elementIdentifier);
        const hal::boolean mixerElementInvalid = static_cast<hal::boolean>(
            (mixerElement == nullptr) || (!::snd_mixer_selem_has_playback_volume(mixerElement)));
        if (mixerElementInvalid)
        {
            return false;
        }

        long minimumVolume{};
        long maximumVolume{};
        ::snd_mixer_selem_get_playback_volume_range(mixerElement, &minimumVolume, &maximumVolume);
        const long volumeRange = maximumVolume - minimumVolume;
        const long requestedPercent = static_cast<long>(volumePercent);
        const long scaledVolume = volumeRange * requestedPercent;
        const long selectedVolume = minimumVolume + (scaledVolume / 100L);
        return ::snd_mixer_selem_set_playback_volume_all(mixerElement, selectedVolume) >= 0;
    }

    /**
     * @brief Closes one real ALSA mixer handle.
     *
     * @param[in] context
     * Unused stateless operation context.
     *
     * @param[in,out] mixerHandle
     * Non-null handle that becomes invalid after this call.
     */
    void XWalkAudioAlsa::systemCloseMixer(contextpointer context, audiomixerhandle mixerHandle)
    {
        static_cast<void>(context);
        snd_mixer_t* const nativeHandle = static_cast<snd_mixer_t*>(mixerHandle);
        static_cast<void>(::snd_mixer_close(nativeHandle));
    }

} /* namespace xwalk::hal */
