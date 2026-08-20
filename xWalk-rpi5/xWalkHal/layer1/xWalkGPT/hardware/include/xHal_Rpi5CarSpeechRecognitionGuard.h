/******************************************************************************
 * @file        xHal_Rpi5CarSpeechRecognitionGuard.h
 * @brief       Declares scope-bound streaming recognition cleanup.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SPEECH_RECOGNITION_GUARD_H
#define XHAL_RPI5CAR_SPEECH_RECOGNITION_GUARD_H

#include "xHal_Rpi5CarSpeechToTextAlsaTypes.h"

namespace xwalk::hal
{
    /** @brief Releases one owned streaming recognition session at scope exit. */
    class XWalkSpeechRecognitionGuard final
    {
        private:
            contextpointer callbackContext{nullptr};
            speechrecognitionsession sessionHandle{nullptr};
            speechrecognizerreleasecallback releaseCallback{nullptr};

        public:
            /** @brief Binds one session to its non-throwing release callback. */
            XWalkSpeechRecognitionGuard(contextpointer context,
                                        speechrecognitionsession session,
                                        speechrecognizerreleasecallback release) noexcept;
            /** @brief Releases the bound recognition session exactly once. */
            ~XWalkSpeechRecognitionGuard() noexcept;
            XWalkSpeechRecognitionGuard(const XWalkSpeechRecognitionGuard&) = delete;
            XWalkSpeechRecognitionGuard& operator=(const XWalkSpeechRecognitionGuard&) = delete;
            XWalkSpeechRecognitionGuard(XWalkSpeechRecognitionGuard&&) = delete;
            XWalkSpeechRecognitionGuard& operator=(XWalkSpeechRecognitionGuard&&) = delete;
    };
} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_RECOGNITION_GUARD_H */
