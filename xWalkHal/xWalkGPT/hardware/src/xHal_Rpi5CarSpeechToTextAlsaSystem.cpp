/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsaSystem.cpp
 * @brief       Implements real libasound microphone capture operations.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 * @copyright Copyright (c) 2026 Joxy John. All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToTextAlsa.h"

#include <alsa/asoundlib.h>

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/**
 * @brief Opens and configures one real ALSA capture stream.
 *
 * @param[in,out] context Unused nullable recognition context.
 * @param[in] deviceName Non-empty ALSA capture device name.
 * @param[in] sampleRateHz Required sample rate in Hertz.
 * @param[in] channelCount Required interleaved channel count.
 * @param[in] periodFrames Requested maximum frames per read.
 * @return Owned opaque capture handle, or null when configuration fails.
 */
speechcapturehandle XWalkSpeechToTextAlsa::systemOpenCapture(contextpointer context,
    stringview deviceName, uint32 sampleRateHz, uint8 channelCount, uint32 periodFrames)
{
    static_cast<void>(context);
    snd_pcm_t* capture{nullptr};
    const string ownedName{deviceName};
    if (snd_pcm_open(&capture, ownedName.c_str(), SND_PCM_STREAM_CAPTURE, 0) < 0)
    {
        return nullptr;
    }
    snd_pcm_hw_params_t* parameters{nullptr};
    snd_pcm_hw_params_alloca(&parameters);
    unsigned int negotiatedRate = sampleRateHz;
    snd_pcm_uframes_t negotiatedPeriod = periodFrames;
    const boolean configured =
        (snd_pcm_hw_params_any(capture, parameters) >= 0) &&
        (snd_pcm_hw_params_set_access(capture, parameters, SND_PCM_ACCESS_RW_INTERLEAVED) >= 0) &&
        (snd_pcm_hw_params_set_format(capture, parameters, SND_PCM_FORMAT_S16_LE) >= 0) &&
        (snd_pcm_hw_params_set_channels(capture, parameters, channelCount) >= 0) &&
        (snd_pcm_hw_params_set_rate_near(capture, parameters, &negotiatedRate, nullptr) >= 0) &&
        (snd_pcm_hw_params_set_period_size_near(capture, parameters,
            &negotiatedPeriod, nullptr) >= 0) &&
        (snd_pcm_hw_params(capture, parameters) >= 0) &&
        (negotiatedRate == sampleRateHz) && (snd_pcm_prepare(capture) >= 0);
    if (!configured)
    {
        static_cast<void>(snd_pcm_close(capture));
        return nullptr;
    }
    return capture;
}

/**
 * @brief Reads up to one requested frame count from real ALSA capture.
 *
 * @param[in,out] context Unused nullable recognition context.
 * @param[in,out] captureHandle Non-null handle returned by `systemOpenCapture`.
 * @param[out] pcmData Captured interleaved signed-16 PCM bytes.
 * @param[in] frameCount Requested frames from 1 through 1,024.
 * @return Positive frame count, zero, or a negative ALSA error value.
 */
int32 XWalkSpeechToTextAlsa::systemReadCapture(contextpointer context,
    speechcapturehandle captureHandle, bytevector& pcmData, size frameCount)
{
    static_cast<void>(context);
    snd_pcm_t* capture = static_cast<snd_pcm_t*>(captureHandle);
    pcmData.assign(frameCount * XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_BYTES, 0U);
    const snd_pcm_sframes_t result = snd_pcm_readi(capture, pcmData.data(), frameCount);
    if (result <= 0)
    {
        pcmData.clear();
        return static_cast<int32>(result);
    }
    const size completedFrames = static_cast<size>(result);
    pcmData.resize(completedFrames * XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_BYTES);
    return static_cast<int32>(result);
}

/**
 * @brief Attempts ALSA recovery for one negative capture result.
 *
 * @param[in,out] context Unused nullable recognition context.
 * @param[in,out] captureHandle Non-null open ALSA capture handle.
 * @param[in] errorValue Negative ALSA result returned by capture.
 * @return `true` after successful recovery; otherwise `false`.
 */
boolean XWalkSpeechToTextAlsa::systemRecoverCapture(contextpointer context,
    speechcapturehandle captureHandle, int32 errorValue)
{
    static_cast<void>(context);
    return snd_pcm_recover(static_cast<snd_pcm_t*>(captureHandle), errorValue, 1) >= 0;
}

/**
 * @brief Drops pending capture data and closes one real ALSA handle.
 *
 * @param[in,out] context Unused nullable recognition context.
 * @param[in,out] captureHandle Non-null owned ALSA capture handle.
 */
void XWalkSpeechToTextAlsa::systemCloseCapture(contextpointer context,
    speechcapturehandle captureHandle)
{
    static_cast<void>(context);
    static_cast<void>(snd_pcm_drop(static_cast<snd_pcm_t*>(captureHandle)));
    static_cast<void>(snd_pcm_close(static_cast<snd_pcm_t*>(captureHandle)));
}

} /* namespace xwalk::hal */
