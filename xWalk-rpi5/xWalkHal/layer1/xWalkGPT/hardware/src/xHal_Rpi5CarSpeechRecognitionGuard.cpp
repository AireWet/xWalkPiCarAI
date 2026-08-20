/******************************************************************************
 * @file        xHal_Rpi5CarSpeechRecognitionGuard.cpp
 * @brief       Implements scope-bound streaming recognition cleanup.
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#include "xHal_Rpi5CarSpeechRecognitionGuard.h"

namespace xwalk::hal
{
    /**
     * @brief Binds one recognition session to its release callback.
     * @param[in,out] context Nullable non-owning callback context.
     * @param[in,out] session Non-null owned recognition session.
     * @param[in] release Non-null non-throwing release callback.
     */
    XWalkSpeechRecognitionGuard::XWalkSpeechRecognitionGuard(contextpointer context,
                                                             speechrecognitionsession session,
                                                             speechrecognizerreleasecallback release) noexcept
        : callbackContext(context), sessionHandle(session), releaseCallback(release)
    {
    }

    /** @brief Releases the bound recognition session exactly once. */
    XWalkSpeechRecognitionGuard::~XWalkSpeechRecognitionGuard() noexcept
    {
        releaseCallback(callbackContext, sessionHandle);
    }
} /* namespace xwalk::hal */
