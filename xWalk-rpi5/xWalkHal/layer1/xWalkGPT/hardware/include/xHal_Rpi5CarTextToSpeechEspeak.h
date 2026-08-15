/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechEspeak.h
 * @brief       Declares the Espeak PCM synthesis provider.
 *
 * @details
 * Runs a deployment-selected Espeak executable without a shell, captures its
 * WAV output, and returns validated PCM to the shared ALSA playback adapter.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Espeak Provider
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_ESPEAK_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_ESPEAK_H

/******************************************************************************
 * Includes
 ******************************************************************************/

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
     * @class XWalkTextToSpeechEspeak
     * @brief Provides offline Espeak synthesis to `XWalkTextToSpeechAlsa`.
     *
     * @details
     * Owns executable and voice names only. Each request owns one bounded child
     * process and captures its output synchronously without shell interpretation.
     */
    class XWalkTextToSpeechEspeak final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Owned executable name or absolute path. */
            string executableName{};
            /** @brief Owned non-empty Espeak voice identifier. */
            string voiceName{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Converts a callback context into its required provider object. */
            static XWalkTextToSpeechEspeak& provider(contextpointer context);
            /** @brief Synthesizes one bounded text value into signed sixteen-bit PCM. */
            static XWalkTextToSpeechPcmData synthesize(contextpointer context, stringview text);
            /** @brief Runs Espeak and captures its bounded WAV standard output. */
            bytevector execute(stringview text) const;
            /** @brief Parses one PCM WAV result returned by Espeak. */
            static XWalkTextToSpeechPcmData parseWave(const bytevector& waveData);
            /** @brief Reads one little-endian unsigned sixteen-bit WAV field. */
            static uint16 readUint16(const bytevector& bytes, size offset);
            /** @brief Reads one little-endian unsigned thirty-two-bit WAV field. */
            static uint32 readUint32(const bytevector& bytes, size offset);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Stores one deployment-selected Espeak executable and voice.
             * @param[in] executable Non-empty executable name or path.
             * @param[in] voice Non-empty Espeak voice identifier such as `en`.
             * @throws std::invalid_argument If either value is empty.
             */
            XWalkTextToSpeechEspeak(stringview executable, stringview voice);
            /** @brief Destroys the provider after every synchronous request completes. */
            ~XWalkTextToSpeechEspeak() = default;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            XWalkTextToSpeechEspeak(const XWalkTextToSpeechEspeak&) = delete;
            XWalkTextToSpeechEspeak& operator=(const XWalkTextToSpeechEspeak&) = delete;
            XWalkTextToSpeechEspeak(XWalkTextToSpeechEspeak&&) = delete;
            XWalkTextToSpeechEspeak& operator=(XWalkTextToSpeechEspeak&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /** @brief Returns the synthesis operation requiring this object as context. */
            XWalkTextToSpeechAlsaOperations operations() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_ESPEAK_H */
