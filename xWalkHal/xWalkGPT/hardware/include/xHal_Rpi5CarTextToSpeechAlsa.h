/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechAlsa.h
 * @brief       Declares synthesis adaptation to shared ALSA playback.
 *
 * @details
 * Observes one caller-owned shared ALSA backend, validates provider PCM, and
 * writes complete interleaved frames through bounded playback operations.
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

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_ALSA_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_ALSA_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarTextToSpeechAlsaTypes.h"

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
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkTextToSpeechAlsa
 * @brief Connects one synthesis provider to shared bounded ALSA playback.
 *
 * @details The adapter observes its audio owner and provider context. It owns
 * no model, credential, process, network client, ALSA handle, or worker thread.
 * Calls are synchronous and require external serialization when shared.
 */
class XWalkTextToSpeechAlsa final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning audio owner that must outlive this adapter and its consumer. */
        XWalkAudioAlsa* audioBackend{nullptr};

        /** @brief Nullable non-owning provider context that must outlive this adapter. */
        contextpointer providerContext{nullptr};

        /** @brief Complete synthesis operation table copied during construction. */
        XWalkTextToSpeechAlsaOperations operations{};

        /** @brief Conservative mixer volume applied before non-empty PCM playback. */
        uint8 playbackVolumePercentValue{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates one provider PCM result before opening ALSA.
         *
         * @param[in] audioData Provider-owned result returned by value.
         * @throws std::invalid_argument If non-empty PCM metadata or alignment is invalid.
         * @throws std::out_of_range If the bounded PCM byte limit is exceeded.
         */
        static void validateAudioData(const XWalkTextToSpeechPcmData& audioData);

        /**
         * @brief Converts a callback context into its required adapter.
         *
         * @param[in,out] context Non-null pointer to a live adapter.
         * @return Referenced adapter.
         * @throws std::invalid_argument If `context` is null.
         */
        static XWalkTextToSpeechAlsa& adapter(contextpointer context);

        /**
         * @brief Synthesizes text and writes bounded PCM periods through ALSA.
         *
         * @param[in,out] context Non-null pointer to a live adapter.
         * @param[in] text Text view retained only for this synchronous call.
         * @throws std::invalid_argument If synthesized PCM metadata or alignment is invalid.
         * @throws std::out_of_range If synthesized PCM exceeds its bounded byte count.
         * @throws std::runtime_error If synthesis or shared ALSA playback fails.
         */
        static void speak(contextpointer context, stringview text);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs an adapter for one injected synthesis provider.
         *
         * @param[in,out] sharedAudioBackend Caller-owned audio owner that must outlive this adapter.
         * @param[in,out] context Nullable non-owning provider context that must outlive this adapter.
         * @param[in] backendOperations Table containing one non-null synthesis callback.
         * @param[in] playbackVolumePercent Mixer volume from zero through one hundred percent.
         * @throws std::invalid_argument If the synthesis callback is null.
         * @throws std::out_of_range If playback volume exceeds one hundred percent.
         */
        XWalkTextToSpeechAlsa(XWalkAudioAlsa& sharedAudioBackend, contextpointer context,
            const XWalkTextToSpeechAlsaOperations& backendOperations,
            uint8 playbackVolumePercent = 50U);

        /** @brief Destroys the adapter without releasing its caller-owned audio owner. */
        ~XWalkTextToSpeechAlsa();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables copying of non-owning provider and audio bindings. */
        XWalkTextToSpeechAlsa(const XWalkTextToSpeechAlsa&) = delete;
        /** @brief Disables copy assignment of non-owning bindings. */
        XWalkTextToSpeechAlsa& operator=(const XWalkTextToSpeechAlsa&) = delete;
        /** @brief Disables moving because callback context identity is retained. */
        XWalkTextToSpeechAlsa(XWalkTextToSpeechAlsa&&) = delete;
        /** @brief Disables move assignment because callback context identity is retained. */
        XWalkTextToSpeechAlsa& operator=(XWalkTextToSpeechAlsa&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Returns the synthesis-and-playback callback for `XWalkTextToSpeech`.
         * @return Non-null callback requiring this adapter as its context.
         */
        texttospeechspeakcallback callback() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_ALSA_H */
