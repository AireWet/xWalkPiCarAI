/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechPico2Wave.h
 * @brief       Declares the shell-free Pico2Wave speech provider.
 *
 * @details
 * Generates one private WAV file with a deployment-selected Pico2Wave process
 * and plays it synchronously through a deployment-selected executable.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Pico2Wave Provider
 *
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TEXT_TO_SPEECH_PICO2WAVE_H
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_PICO2WAVE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTextToSpeech.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/** @namespace xwalk::hal @brief Contains xWalk hardware abstraction components. */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Owns Pico2Wave process settings and provides synchronous speech output. */
    class XWalkTextToSpeechPico2Wave final
    {
        private:
            /** @brief Owned Pico2Wave executable name or path. */
            string executableName{};
            /** @brief Owned WAV playback executable name or path. */
            string playbackExecutableName{};
            /** @brief Owned Pico2Wave language identifier. */
            string languageName{};

        protected:
            /** @brief Converts callback context into its required live provider. */
            static XWalkTextToSpeechPico2Wave& provider(contextpointer context);
            /** @brief Generates and plays one non-empty bounded text value. */
            static void speak(contextpointer context, stringview text);
            /** @brief Executes synthesis and playback without shell interpretation. */
            void execute(stringview text) const;

        public:
            /**
             * @brief Stores deployment-selected synthesis, playback, and language settings.
             * @param[in] executable Non-empty Pico2Wave executable name or path.
             * @param[in] playbackExecutable Non-empty WAV playback executable name or path.
             * @param[in] language Non-empty language identifier such as `en-US`.
             */
            XWalkTextToSpeechPico2Wave(stringview executable, stringview playbackExecutable, stringview language);
            /** @brief Destroys owned settings after synchronous requests complete. */
            ~XWalkTextToSpeechPico2Wave() = default;

            XWalkTextToSpeechPico2Wave(const XWalkTextToSpeechPico2Wave&) = delete;
            XWalkTextToSpeechPico2Wave(XWalkTextToSpeechPico2Wave&&) = delete;
            XWalkTextToSpeechPico2Wave& operator=(const XWalkTextToSpeechPico2Wave&) = delete;
            XWalkTextToSpeechPico2Wave& operator=(XWalkTextToSpeechPico2Wave&&) = delete;

            /** @brief Returns the synchronous speech callback requiring this object as context. */
            texttospeechspeakcallback callback() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_TEXT_TO_SPEECH_PICO2WAVE_H */
