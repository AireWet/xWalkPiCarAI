/******************************************************************************
 * @file        xHal_Rpi5CarSpeechCaptureGuard.h
 * @brief       Declares scope-bound speech capture cleanup.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SPEECH_CAPTURE_GUARD_H
#define XHAL_RPI5CAR_SPEECH_CAPTURE_GUARD_H

#include "xHal_Rpi5CarSpeechToTextAlsaTypes.h"

namespace xwalk::hal
{
    /** @brief Closes one owned capture handle on every scope-exit path. */
    class XWalkSpeechCaptureGuard final
    {
        private:
            contextpointer callbackContext{nullptr};
            speechcapturehandle captureHandle{nullptr};
            speechcaptureclosecallback closeCallback{nullptr};

        public:
            /** @brief Binds one open capture handle to its non-throwing close callback. */
            XWalkSpeechCaptureGuard(contextpointer context,
                                    speechcapturehandle capture,
                                    speechcaptureclosecallback close) noexcept;
            /** @brief Closes the bound capture handle exactly once. */
            ~XWalkSpeechCaptureGuard() noexcept;
            XWalkSpeechCaptureGuard(const XWalkSpeechCaptureGuard&) = delete;
            XWalkSpeechCaptureGuard& operator=(const XWalkSpeechCaptureGuard&) = delete;
            XWalkSpeechCaptureGuard(XWalkSpeechCaptureGuard&&) = delete;
            XWalkSpeechCaptureGuard& operator=(XWalkSpeechCaptureGuard&&) = delete;
    };
} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_CAPTURE_GUARD_H */
