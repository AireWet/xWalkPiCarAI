/******************************************************************************
 * @file        xHal_Rpi5CarSpeechCaptureGuard.cpp
 * @brief       Implements scope-bound speech capture cleanup.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarSpeechCaptureGuard.h"

namespace xwalk::hal
{
    /**
     * @brief Binds one open capture handle to its close callback.
     * @param[in,out] context Nullable non-owning callback context.
     * @param[in,out] capture Non-null owned capture handle.
     * @param[in] close Non-null non-throwing close callback.
     */
    XWalkSpeechCaptureGuard::XWalkSpeechCaptureGuard(contextpointer context,
                                                     speechcapturehandle capture,
                                                     speechcaptureclosecallback close) noexcept
        : callbackContext(context), captureHandle(capture), closeCallback(close)
    {
    }

    /** @brief Closes the bound capture handle exactly once. */
    XWalkSpeechCaptureGuard::~XWalkSpeechCaptureGuard() noexcept
    {
        closeCallback(callbackContext, captureHandle);
    }
} /* namespace xwalk::hal */
