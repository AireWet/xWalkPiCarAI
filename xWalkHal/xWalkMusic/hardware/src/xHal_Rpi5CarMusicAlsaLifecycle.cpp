/******************************************************************************
 * @file        xHal_Rpi5CarMusicAlsaLifecycle.cpp
 * @brief       Implements ALSA music-adapter lifecycle and callback binding.
 *
 * @details
 * Validates injected decoding, stores the shared audio dependency, stops both
 * workers deterministically, and publishes the complete music callback table.
 *
 * @project     xWalk Firmware
 * @module      xWalkMusic ALSA Adapter
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

#include "xHal_Rpi5CarMusicAlsa.h"

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
 * @brief Constructs an adapter using the built-in PCM WAVE decoder.
 *
 * @param[in,out] sharedAudioBackend
 * Caller-owned ALSA backend that must outlive this adapter and its consumers.
 */
XWalkMusicAlsa::XWalkMusicAlsa(XWalkAudioAlsa& sharedAudioBackend):
    XWalkMusicAlsa(sharedAudioBackend, nullptr, systemOperations())
{
}

/**
 * @brief Constructs an adapter using an injected file decoder.
 *
 * @param[in,out] sharedAudioBackend
 * Caller-owned ALSA backend that must outlive this adapter and its consumers.
 *
 * @param[in,out] context
 * Nullable non-owning decoder context that must outlive this adapter.
 *
 * @param[in] backendOperations
 * Operation table containing one non-null decoder callback.
 *
 * @throws std::invalid_argument
 * If the decoder callback is null.
 */
XWalkMusicAlsa::XWalkMusicAlsa(XWalkAudioAlsa& sharedAudioBackend,
    contextpointer context, const XWalkMusicAlsaOperations& backendOperations):
    audioBackend(&sharedAudioBackend), decoderContext(context), operations(backendOperations)
{
    if (operations.decodeAudio == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Music ALSA adapter requires a decoder callback");
    }
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Stops and joins both retained playback workers.
 *
 * @warning
 * ALSA operations invoked during worker shutdown must not throw.
 */
XWalkMusicAlsa::~XWalkMusicAlsa()
{
    stopSoundWorker();
    stopMusicWorker();
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns the complete callback table bound through this adapter's address.
 *
 * @return
 * Ten non-null callbacks suitable for constructing `XWalkMusic` with this object as context.
 */
XWalkMusicCallbacks XWalkMusicAlsa::callbacks() const noexcept
{
    return {&enableOutput, &playSound, &playSoundBackground, &playMusic,
        &setMusicVolume, &stopMusic, &pauseMusic, &resumeMusic,
        &getSoundLength, &playTone};
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Converts a callback context into its required adapter.
 *
 * @param[in,out] context
 * Non-null pointer supplied to `XWalkMusic` during construction.
 *
 * @return
 * Adapter referenced by the callback context.
 *
 * @throws std::invalid_argument
 * If the callback context is null.
 */
XWalkMusicAlsa& XWalkMusicAlsa::adapter(contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Music ALSA callback context must not be null");
    }
    return *static_cast<XWalkMusicAlsa*>(context);
}

/**
 * @brief Requests and joins the retained background sound worker.
 *
 * @post
 * No background sound worker is joinable and its retained audio is empty.
 */
void XWalkMusicAlsa::stopSoundWorker()
{
    {
        const mutexlock lock(stateMutex);
        soundStopRequested = true;
    }
    if (soundWorker.joinable())
    {
        soundWorker.join();
    }
    const mutexlock lock(stateMutex);
    soundAudio = {};
    soundStopRequested = false;
}

/**
 * @brief Requests and joins the retained streamed-music worker.
 *
 * @post
 * No streamed-music worker is joinable and its retained state is reset.
 */
void XWalkMusicAlsa::stopMusicWorker()
{
    {
        const mutexlock lock(stateMutex);
        musicStopRequested = true;
        musicPauseRequested = false;
    }
    if (musicWorker.joinable())
    {
        musicWorker.join();
    }
    const mutexlock lock(stateMutex);
    musicAudio = {};
    musicStartFrame = 0U;
    musicLoopCount = 0;
    musicStopRequested = false;
    musicPauseRequested = false;
}

} /* namespace xwalk::hal */
