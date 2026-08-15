/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechAlsa.cpp
 * @brief       Implements synthesis validation and shared ALSA playback.
 *
 * @details
 * Validates provider output before resource acquisition and writes signed
 * sixteen-bit PCM using bounded period-sized shared-audio operations.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Text-to-Speech ALSA Backend
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

#include "xHal_Rpi5CarTextToSpeechAlsa.h"

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
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs an adapter for one injected synthesis provider.
     *
     * @param[in,out] sharedAudioBackend Caller-owned audio owner that must outlive
     * this adapter.
     * @param[in,out] context Nullable non-owning provider context that must outlive
     * this adapter.
     * @param[in] backendOperations Table containing one non-null synthesis
     * callback.
     * @param[in] playbackVolumePercent Mixer volume from zero through one hundred
     * percent.
     * @throws std::invalid_argument If the synthesis callback is null.
     * @throws std::out_of_range If playback volume exceeds one hundred percent.
     */
    XWalkTextToSpeechAlsa::XWalkTextToSpeechAlsa(XWalkAudioAlsa& sharedAudioBackend,
                                                 contextpointer context,
                                                 const XWalkTextToSpeechAlsaOperations& backendOperations,
                                                 uint8 playbackVolumePercent)
        : audioBackend(&sharedAudioBackend), providerContext(context), operations(backendOperations),
          playbackVolumePercentValue(playbackVolumePercent)
    {
        if (operations.synthesize == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Text-to-speech ALSA adapter requires synthesis");
        }
        if (playbackVolumePercentValue > 100U)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Text-to-speech ALSA volume exceeds one hundred percent");
        }
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the adapter without releasing its caller-owned audio owner.
     */
    XWalkTextToSpeechAlsa::~XWalkTextToSpeechAlsa() = default;

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Returns the synthesis-and-playback callback for `XWalkTextToSpeech`.
     * @return Non-null callback requiring this adapter as its context.
     */
    texttospeechspeakcallback XWalkTextToSpeechAlsa::callback() const noexcept
    {
        return &speak;
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Validates one provider PCM result before opening ALSA.
     *
     * @param[in] audioData Provider-owned result returned by value.
     * @throws std::invalid_argument If non-empty PCM metadata or alignment is
     * invalid.
     * @throws std::out_of_range If the bounded PCM byte limit is exceeded.
     */
    void XWalkTextToSpeechAlsa::validateAudioData(const XWalkTextToSpeechPcmData& audioData)
    {
        const hal::boolean pcmDataTooLarge =
            static_cast<hal::boolean>(audioData.pcmData.size() > XHAL_RPI5CAR_TEXT_TO_SPEECH_MAXIMUM_PCM_BYTES);
        if (pcmDataTooLarge)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Text-to-speech PCM exceeds bounded byte count");
        }
        const hal::boolean pcmDataEmpty = static_cast<hal::boolean>(audioData.pcmData.empty());
        if (pcmDataEmpty)
        {
            return;
        }
        if ((audioData.sampleRateHz == 0U) || (audioData.channelCount == 0U) ||
            (audioData.channelCount > XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Text-to-speech PCM metadata is invalid");
        }
        const size channelCount = static_cast<size>(audioData.channelCount);
        const size bytesPerFrame = channelCount * XHAL_RPI5CAR_TEXT_TO_SPEECH_SAMPLE_BYTES;
        const hal::boolean audioDataPcmDataBytesPerFrameDifferent =
            static_cast<hal::boolean>((audioData.pcmData.size() % bytesPerFrame) != 0U);
        if (audioDataPcmDataBytesPerFrameDifferent)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Text-to-speech PCM has an incomplete frame");
        }
    }

    /**
     * @brief Converts a callback context into its required adapter.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @return Referenced adapter.
     * @throws std::invalid_argument If `context` is null.
     */
    XWalkTextToSpeechAlsa& XWalkTextToSpeechAlsa::adapter(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Text-to-speech ALSA context must not be null");
        }
        return *static_cast<XWalkTextToSpeechAlsa*>(context);
    }

    /**
     * @brief Synthesizes text and writes bounded PCM periods through ALSA.
     *
     * @param[in,out] context Non-null pointer to a live adapter.
     * @param[in] text Text view retained only for this synchronous call.
     * @throws std::invalid_argument If synthesized PCM metadata or alignment is
     * invalid.
     * @throws std::out_of_range If synthesized PCM exceeds its bounded byte count.
     * @throws std::runtime_error If synthesis or shared ALSA playback fails.
     */
    void XWalkTextToSpeechAlsa::speak(contextpointer context, stringview text)
    {
        XWalkTextToSpeechAlsa& self = adapter(context);
        const XWalkTextToSpeechPcmData audioData = self.operations.synthesize(self.providerContext, text);
        validateAudioData(audioData);
        const hal::boolean audioDataEmpty = static_cast<hal::boolean>(audioData.pcmData.empty());
        if (audioDataEmpty)
        {
            return;
        }

        self.audioBackend->setVolume(self.playbackVolumePercentValue);
        const XWalkAudioStreamConfiguration configuration{audioData.sampleRateHz,
                                                          audioData.channelCount,
                                                          XWalkAudioSampleFormat::Signed16LittleEndian,
                                                          XHAL_RPI5CAR_TEXT_TO_SPEECH_PERIOD_FRAMES,
                                                          XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US};
        audiopcmhandle streamHandle = self.audioBackend->openStream(configuration);
        const size channelCount = static_cast<size>(audioData.channelCount);
        const size bytesPerFrame = channelCount * XHAL_RPI5CAR_TEXT_TO_SPEECH_SAMPLE_BYTES;
        const size totalFrames = audioData.pcmData.size() / bytesPerFrame;
        size framePosition{};
        while (framePosition < totalFrames)
        {
            const size remainingFrames = totalFrames - framePosition;
            const size periodFrames = static_cast<size>(XHAL_RPI5CAR_TEXT_TO_SPEECH_PERIOD_FRAMES);
            const size frameCount = std::min(remainingFrames, periodFrames);
            const size firstByte = framePosition * bytesPerFrame;
            const size byteCount = frameCount * bytesPerFrame;
            const auto firstIterator = audioData.pcmData.begin() + static_cast<bytevector::difference_type>(firstByte);
            const auto finalIterator = firstIterator + static_cast<bytevector::difference_type>(byteCount);
            const bytevector periodData(firstIterator, finalIterator);
            self.audioBackend->writeFrames(streamHandle, periodData, frameCount);
            framePosition += frameCount;
        }
        self.audioBackend->closeStream(streamHandle);
    }

} /* namespace xwalk::hal */
