/******************************************************************************
 * @file        xHal_Rpi5CarMusicLifecycle.cpp
 * @brief       Implements music-controller lifecycle and backend validation.
 *
 * @details
 * Validates and stores the caller's callback backend, then performs the
 * Python-compatible output-enable operation during construction.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic
 *
 * @author      Joxy John
 * @date        2026-07-29
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

#include "xHal_Rpi5CarMusic.h"

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
 * @brief Constructs a music controller and enables its audio output.
 *
 * @param[in,out] context
 * Non-owning backend context; nullability is backend-specific.
 *
 * @param[in] backendCallbacks
 * Complete callback table copied into the controller.
 *
 * @pre
 * Any non-null context object outlives this controller.
 *
 * @post
 * The backend enable callback has completed, and default state is 4/4, 120
 * quarter-note beats per minute, with no key displacement.
 *
 * @throws std::invalid_argument
 * If any required callback is null.
 */
XWalkMusic::XWalkMusic(contextpointer context, const XWalkMusicCallbacks& backendCallbacks):
    backendContext(context), callbacks(backendCallbacks)
{
    validateCallbacks(callbacks);
    callbacks.enableOutput(backendContext);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the controller without releasing its caller-owned backend.
 *
 * @note
 * The Python implementation does not disable the speaker at destruction, so
 * this port intentionally leaves platform output state unchanged.
 */
XWalkMusic::~XWalkMusic() = default;

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Validates the complete injected callback table.
 *
 * @param[in] backendCallbacks
 * Callback table whose entries must all be non-null.
 *
 * @throws std::invalid_argument
 * If any required callback is null.
 */
void XWalkMusic::validateCallbacks(const XWalkMusicCallbacks& backendCallbacks)
{
    const boolean missingPlaybackCallback =
        (backendCallbacks.enableOutput == nullptr) ||
        (backendCallbacks.playSound == nullptr) ||
        (backendCallbacks.playSoundBackground == nullptr) ||
        (backendCallbacks.playMusic == nullptr) ||
        (backendCallbacks.setMusicVolume == nullptr);
    const boolean missingControlCallback =
        (backendCallbacks.stopMusic == nullptr) ||
        (backendCallbacks.pauseMusic == nullptr) ||
        (backendCallbacks.resumeMusic == nullptr);
    const boolean missingDataCallback =
        (backendCallbacks.getSoundLength == nullptr) ||
        (backendCallbacks.playTone == nullptr);
    if (missingPlaybackCallback || missingControlCallback || missingDataCallback)
    {
        XHAL_THROW_INVALID_ARGUMENT("Music backend requires a complete callback table");
    }
}

} /* namespace xwalk::hal */
